#include "image.h"
#include "patch.h"
#include "polygons.h"

#include <iostream>
#include <cmath>
#include <algorithm>
#include <limits>
#include <stdexcept>

using namespace Imagine;
using namespace std;

// --- Point Implementation ---
Point::Point(int x, int y) : x(x), y(y) {}
bool Point::operator==(const Point& other) const { return x == other.x && y == other.y; }

// --- CriminisiImage Implementation ---

CriminisiImage::CriminisiImage(const std::string& image_path, int size) {
    int w, h;
    Color* pixels = nullptr;

    if (!loadColorImage(image_path, pixels, w, h)) {
        throw std::runtime_error("Error: Could not load image " + image_path);
    }

    // Initialize the Imagine++ Image object
    image = Image<RGB<octet>>(w, h);

    // Deep copy of pixels
    for (int y = 0; y < h; y++) {
        for (int x = 0; x < w; x++) {
            image(x, y) = pixels[y * w + x];
        }
    }
    delete[] pixels; // Memory safety: free temporary buffer immediately

    // Initialize algorithm buffers
    mask = Image<octet>(w, h);
    confidence = Image<float>(w, h);
    fillFront = Image<octet>(w, h);
    image_gray = Image<float>(w, h);
    grad_x = Image<float>(w, h);
    grad_y = Image<float>(w, h);

    mask.fill(0);
    confidence.fill(1.0f); // Initially, all known pixels have 100% confidence
    fillFront.fill(0);

    // Allocate display buffers
    r = new octet[w * h];
    g = new octet[w * h];
    b = new octet[w * h];

    std::cout << "Image loaded: " << w << "x" << h << std::endl;

    // Initial pre-processing
    computeMask();
    findContours();
    colorToGray();
    computeGradientWithNan();
    setPatchSize(size);
}

CriminisiImage::~CriminisiImage() {
    // Prevent memory leaks
    if (r) delete[] r;
    if (g) delete[] g;
    if (b) delete[] b;
}

void CriminisiImage::show() {
    int w = image.width();
    int h = image.height();

    // Update display buffers from current image state
    for (int y = 0; y < h; y++) {
        for (int x = 0; x < w; x++) {
            Color color = image(x, y);
            r[y * w + x] = color.r();
            g[y * w + x] = color.g();
            b[y * w + x] = color.b();
        }
    }
    putColorImage(0, 0, r, g, b, w, h);
    milliSleep(5); // Small delay for visual feedback
}

void CriminisiImage::computeMask() {
    // Identify the target region (hole).
    // Convention: Pure black (0,0,0) is treated as the missing region.
    for (int y = 0; y < image.height(); y++) {
        for (int x = 0; x < image.width(); x++) {
            Color c = image(x, y);
            if (c.r() == 0 && c.g() == 0 && c.b() == 0) {
                mask(x, y) = 1; // 1 = To be filled
            } else {
                mask(x, y) = 0; // 0 = Valid source
            }
        }
    }
}

void CriminisiImage::eraseZone(const Point& topLeft, const Point& bottomRight) {
    for (int y = topLeft.y; y < bottomRight.y; y++) {
        for (int x = topLeft.x; x < bottomRight.x; x++) {
            if(x >= 0 && x < width() && y >= 0 && y < height()) {
                image(x, y) = Color(0, 0, 0); // Paint black
                confidence(x, y) = 0.0f;      // Reset confidence in erased zone
            }
        }
    }
    computeMask();
    findContours();
}

void CriminisiImage::erasePoly(const std::vector<int>& list_x, const std::vector<int>& list_y){
    for (int y = 0; y < image.height(); y++) {
        for (int x = 0; x < image.width(); x++){
            if (PointinPoly(x, y, list_x, list_y)){
                image(x, y) = Color(0, 0, 0);
                confidence(x, y) = 0.0f;
            }
        }
    }
    computeMask();
    findContours();
}

void CriminisiImage::setPatchSize(int size) {
    if (size % 2 == 0) throw std::invalid_argument("Patch size must be odd (e.g., 9, 11).");
    patch_size = size;
}

void CriminisiImage::nanToZeros() {
    // Utility to ensure masked regions are strictly black
    for (int y = 0; y < image.height(); y++) {
        for (int x = 0; x < image.width(); x++) {
            if (mask(x, y) == 1) {
                image(x, y) = Color(0, 0, 0);
            }
        }
    }
}

void CriminisiImage::findContours() {
    contour.clear();
    int w = width();
    int h = height();

    // Iterate through image (excluding 1-pixel border for safety)
    for (int y = 1; y < h - 1; y++) {
        for (int x = 1; x < w - 1; x++) {
            // A pixel is on the contour if it is VALID (mask=0)
            // but has at least one INVALID neighbor (mask=1).
            if (mask(x, y) == 0) {
                if (mask(x, y-1) == 1 || mask(x, y+1) == 1 ||
                    mask(x-1, y) == 1 || mask(x+1, y) == 1) {
                    contour.push_back(Point(x, y));
                }
            }
        }
    }
}

void CriminisiImage::colorToGray() {
    // Standard luminosity conversion: Y = 0.299R + 0.587G + 0.114B
    for (int y = 0; y < image.height(); y++) {
        for (int x = 0; x < image.width(); x++) {
            Color c = image(x, y);
            float gray = 0.299f * c.r() + 0.587f * c.g() + 0.114f * c.b();
            image_gray(x, y) = gray / 255.0f;
        }
    }
}

void CriminisiImage::computeGradientWithNan() {
    grad_x.fill(0.0f);
    grad_y.fill(0.0f);
    Point direction[8] = {{1,0}, {1,1}, {1,-1}, {-1,0}, {-1,1}, {-1,-1}, {0,1}, {0,-1}};

    // Robust gradient calculation:
    // We cannot use standard Sobel because some neighbors might be missing (masked).
    // We compute a weighted average of directional derivatives only for valid neighbors.
    for (int y = 1; y < image.height() - 1; y++) {
        for (int x = 1; x < image.width() - 1; x++) {
            if (mask(x, y) == 0) {
                float sum_gx = 0.0f, sum_gy = 0.0f;
                float w_x = 0.0f, w_y = 0.0f;

                for (int i = 0; i < 8; i++) {
                    int nx = x + direction[i].x;
                    int ny = y + direction[i].y;

                    if (mask(nx, ny) == 0) { // Only use valid pixels
                        float norm = std::sqrt(float(direction[i].x * direction[i].x + direction[i].y * direction[i].y));
                        float weight = 1.0f / norm; // Weight by inverse distance
                        float diff = image_gray(nx, ny) - image_gray(x, y);

                        if (direction[i].x != 0) { sum_gx += diff * direction[i].x * weight; w_x += weight; }
                        if (direction[i].y != 0) { sum_gy += diff * direction[i].y * weight; w_y += weight; }
                    }
                }
                // Normalize sum
                grad_x(x, y) = (w_x > 0) ? (sum_gx / w_x) : 0.0f;
                grad_y(x, y) = (w_y > 0) ? (sum_gy / w_y) : 0.0f;
            } else {
                grad_x(x, y) = std::numeric_limits<float>::quiet_NaN();
                grad_y(x, y) = std::numeric_limits<float>::quiet_NaN();
            }
        }
    }
}

std::vector<float> CriminisiImage::computeNormalAtP(const Point& p) {
    auto it = std::find(contour.begin(), contour.end(), p);
    if (it == contour.end()) return {0.0f, 0.0f};

    // Estimate normal using immediate neighbors on the contour list
    size_t idx = std::distance(contour.begin(), it);
    Point prev = (idx > 0) ? contour[idx - 1] : contour.back();
    Point next = (idx < contour.size() - 1) ? contour[idx + 1] : contour.front();

    float tx = float(next.x - prev.x);
    float ty = float(next.y - prev.y);

    // Normal is perpendicular to tangent (-dy, dx)
    float nx = -ty;
    float ny = tx;

    float n = std::sqrt(nx * nx + ny * ny);
    if (n > 1e-6) { nx /= n; ny /= n; }
    return {nx, ny};
}

void CriminisiImage::computePriorityList() {
    priority_list.clear();
    // Priority P(p) is computed for every point on the fill front
    for (const auto& coord : contour) {
        Patch patch(coord, *this);
        priority_list.push_back({coord, patch.computePriority()});
    }
}

void CriminisiImage::findPatchList() {
    patch_list.clear();
    int hw = patch_size / 2;
    int w = width();
    int h = height();

    // Naive search: Scan every pixel in the image to find valid source patches.
    // A patch is valid if it contains NO masked pixels.
    for (int y = hw; y < h - hw; y++) {
        for (int x = hw; x < w - hw; x++) {
            if (mask(x, y) == 0) {
                bool valid = true;
                // Check full patch validity
                for (int dy = -hw; dy <= hw; dy++) {
                    for (int dx = -hw; dx <= hw; dx++) {
                        if (mask(x + dx, y + dy) != 0) {
                            valid = false;
                            break;
                        }
                    }
                    if (!valid) break;
                }
                if (valid) patch_list.push_back(Patch(Point(x, y), *this));
            }
        }
    }
}

void CriminisiImage::findPatchList_focus(Image<octet> search_zone) {
    patch_list.clear();
    int hw = patch_size / 2;
    int w = width();
    int h = height();

    // Optimized search: Only look in the user-defined 'search_zone' (blue polygon).
    for (int y = hw; y < h - hw; y++) {
        for (int x = hw; x < w - hw; x++) {
            if (mask(x, y) == 0 && search_zone(x, y) == 0) { // Valid AND in search zone
                bool valid = true;
                for (int dy = -hw; dy <= hw; dy++) {
                    for (int dx = -hw; dx <= hw; dx++) {
                        if (mask(x + dx, y + dy) != 0) {
                            valid = false;
                            break;
                        }
                    }
                    if (!valid) break;
                }
                if (valid) patch_list.push_back(Patch(Point(x, y), *this));
            }
        }
    }
}

void CriminisiImage::criminisi(int maxIter) {
    std::cout << "Starting Criminisi (Global Search)..." << std::endl;
    for (int iter = 0; iter < maxIter; iter++) {
        if (contour.empty()) break;

        // 1. Where to fill? (Compute Priorities)
        computePriorityList();
        if (priority_list.empty()) break;

        auto max_elem = std::max_element(priority_list.begin(), priority_list.end(),
                                         [](const auto& a, const auto& b) { return a.second < b.second; });

        Point target = max_elem->first;
        Patch targetP(target, *this);

        // 2. What to fill with? (Find Best Exemplar)
        findPatchList(); // Re-computing this every frame is slow (O(N^2) complexity roughly)
        if (patch_list.empty()) break;

        Patch* best = nullptr;
        float min_dist = std::numeric_limits<float>::max();

        // 3. Search for lowest SSD (Sum of Squared Differences)
        for (auto& p : patch_list) {
            float d = targetP.ssd(p);
            if (d < min_dist) { min_dist = d; best = &p; }
        }

        // 4. Update image
        if (best) targetP.updatePatch(*best);

        computeMask();
        findContours();
        show();
    }
}

void CriminisiImage::criminisi_focus(int maxIter, Image<octet> search_zone) {
    // Pre-calculation of potential sources significantly improves performance
    findPatchList_focus(search_zone);
    std::cout << "Starting Criminisi (Focused Search)..." << std::endl;

    for (int iter = 0; iter < maxIter; iter++) {
        if (contour.empty()) break;

        // 1. Calculate priorities for all pixels on the fill front
        computePriorityList();
        if (priority_list.empty()) break;

        // Select the pixel with highest priority
        auto max_elem = std::max_element(priority_list.begin(), priority_list.end(),
                                         [](const auto& a, const auto& b) { return a.second < b.second; });

        Point target = max_elem->first;
        Patch targetP(target, *this);

        if (patch_list.empty()) break;

        // 2. Linear search for the best matching patch (Template Matching)
        Patch* best = nullptr;
        float min_dist = std::numeric_limits<float>::max();

        for (auto& p : patch_list) {
            float d = targetP.ssd(p);
            if (d < min_dist) { min_dist = d; best = &p; }
        }

        // 3. Copy texture and update confidence
        if (best) {
            targetP.updatePatch(*best);

            // Optimization: The newly filled patch is now a valid source for future iterations
            patch_list.push_back(targetP);

            // Optimization: Locally update mask instead of full scan
            int hw = targetP.half_window;
            for(int dy=-hw; dy<=hw; dy++) {
                for(int dx=-hw; dx<=hw; dx++) {
                    int px = target.x + dx, py = target.y + dy;
                    if(px>=0 && px<width() && py>=0 && py<height())
                        mask(px, py) = 0; // Mark as filled
                }
            }
        }
        findContours();
        show();
    }
    std::cout << "Inpainting completed." << std::endl;
}

// Getters
int CriminisiImage::width() const { return image.width(); }
int CriminisiImage::height() const { return image.height(); }
int CriminisiImage::getPatchSize() const { return patch_size; }
Image<Color>& CriminisiImage::getImage() { return image; }
Image<octet>& CriminisiImage::getMask() { return mask; }
Image<float>& CriminisiImage::getConfidence() { return confidence; }
Image<octet>& CriminisiImage::getFillFront() { return fillFront; }
Image<float>& CriminisiImage::getImageGray() { return image_gray; }
Image<float>& CriminisiImage::getGradX() { return grad_x; }
Image<float>& CriminisiImage::getGradY() { return grad_y; }
const std::vector<Point>& CriminisiImage::getContour() const { return contour; }
