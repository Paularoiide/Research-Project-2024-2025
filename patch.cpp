#include "patch.h"
#include <cmath>
#include <limits>
#include <algorithm>

using namespace Imagine;

Patch::Patch(const Point& center, CriminisiImage& image)
    : center(center), image(image) {
    half_window = image.getPatchSize() / 2;
}

Image<Color> Patch::extractRegion() const {
    int size = 2 * half_window + 1;
    Image<Color> region(size, size);
    int w = image.width();
    int h = image.height();
    const auto& img = image.getImage();

    for (int dy = -half_window; dy <= half_window; dy++) {
        for (int dx = -half_window; dx <= half_window; dx++) {
            int px = center.x + dx;
            int py = center.y + dy;

            // Boundary checks
            if (px >= 0 && px < w && py >= 0 && py < h)
                region(dx + half_window, dy + half_window) = img(px, py);
            else
                region(dx + half_window, dy + half_window) = Color(0,0,0);
        }
    }
    return region;
}

float Patch::ssd(const Patch& other) {
    // Optimized SSD calculation:
    // We compute the Euclidean distance in RGB space.
    // IMPORANT: We skip pixels that are masked (black/invalid) in the target patch,
    // because we don't know what color they should be yet.

    float distance = 0.0f;
    int valid_pixels = 0;
    int w = image.width();
    int h = image.height();
    const auto& img = image.getImage();

    for (int dy = -half_window; dy <= half_window; dy++) {
        for (int dx = -half_window; dx <= half_window; dx++) {
            int x1 = center.x + dx; int y1 = center.y + dy;
            int x2 = other.center.x + dx; int y2 = other.center.y + dy;

            // Check image bounds
            if (x1<0 || x1>=w || y1<0 || y1>=h || x2<0 || x2>=w || y2<0 || y2>=h) continue;

            Color c1 = img(x1, y1);
            Color c2 = img(x2, y2);

            // Check for validity (not black 0,0,0)
            bool v1 = !(c1.r() == 0 && c1.g() == 0 && c1.b() == 0);
            bool v2 = !(c2.r() == 0 && c2.g() == 0 && c2.b() == 0);

            // Only compare if both pixels contribute valid information
            if (v1 && v2) {
                float dr = float(c1.r()) - float(c2.r());
                float dg = float(c1.g()) - float(c2.g());
                float db = float(c1.b()) - float(c2.b());
                distance += dr*dr + dg*dg + db*db;
                valid_pixels++;
            }
        }
    }

    // Normalize by the number of valid pixels to prevent bias towards small overlaps
    return valid_pixels > 0 ? distance / valid_pixels : std::numeric_limits<float>::max();
}

void Patch::updatePatch(const Patch& other) {
    Image<Color>& img = image.getImage();
    Image<Color> source = other.extractRegion();

    // The confidence of the newly filled pixels becomes the confidence of the patch center
    float conf = computeConfidence();
    int w = image.width();
    int h = image.height();

    for (int dy = -half_window; dy <= half_window; dy++) {
        for (int dx = -half_window; dx <= half_window; dx++) {
            int tx = center.x + dx;
            int ty = center.y + dy;

            if (tx >= 0 && tx < w && ty >= 0 && ty < h) {
                // We only overwrite pixels that are currently in the target region (mask=1)
                if (image.getMask()(tx, ty) == 1) {
                    img(tx, ty) = source(dx + half_window, dy + half_window);
                    image.getConfidence()(tx, ty) = conf;
                }
            }
        }
    }
}

float Patch::computeConfidence() {
    // C(p) term: Sum of confidence of known pixels / Area of patch
    float sum = 0.0f;
    int count = 0;
    int w = image.width();
    int h = image.height();

    for (int dy = -half_window; dy <= half_window; dy++) {
        for (int dx = -half_window; dx <= half_window; dx++) {
            int px = center.x + dx;
            int py = center.y + dy;
            if(px>=0 && px<w && py>=0 && py<h) {
                sum += image.getConfidence()(px, py);
                count++;
            }
        }
    }
    return count > 0 ? sum / count : 0.0f;
}

float Patch::computePriority() {
    float C = computeConfidence();
    std::vector<float> n = image.computeNormalAtP(center);

    // Compute Isophote vector (Perpendicular to Gradient)
    // Isophote direction flows along lines of constant intensity (structures).
    // We rotate the gradient (-grad_y, grad_x) to get the isophote.
    float gx = image.getGradX()(center.x, center.y);
    float gy = image.getGradY()(center.x, center.y);

    // Handle potential NaNs
    if(std::isnan(gx)) gx=0;
    if(std::isnan(gy)) gy=0;

    // Isophote vector
    float iso_x = -gy;
    float iso_y = gx;

    // Data Term D(p) = | Isophote * Normal |
    // This prioritizes patches where structures hit the hole boundary perpendicularly.
    float D = std::abs(iso_x * n[0] + iso_y * n[1]);

    // Normalization factor alpha is implicit here (usually 255)
    return C * D;
}
