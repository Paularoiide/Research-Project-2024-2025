#include "polygons.h"

using namespace Imagine;
using namespace std;

void FillPolygon(std::vector<int>& sommets_x, std::vector<int>& sommets_y, Color c) {
    // Clear previous data to ensure we start fresh
    sommets_x.clear();
    sommets_y.clear();

    Point p0, p_prev, p_curr;
    int click = 0;

    // 1. Initialization: Get the first point (Start of the polygon)
    getMouse(p0.x, p0.y);
    fillCircle(p0.x, p0.y, 4, c); // Visual feedback
    sommets_x.push_back(p0.x);
    sommets_y.push_back(p0.y);
    p_prev = p0;

    // 2. Interactive Loop
    while (true) {
        click = getMouse(p_curr.x, p_curr.y);

        // Case 1: Left Click (Button 1) -> Add a new segment
        if (click == 1) {
            drawLine(p_prev.x, p_prev.y, p_curr.x, p_curr.y, c, 2);
            fillCircle(p_curr.x, p_curr.y, 4, c);

            sommets_x.push_back(p_curr.x);
            sommets_y.push_back(p_curr.y);

            p_prev = p_curr; // Update previous point for the next segment
        }
        // Case 3: Right Click (Button 3) -> Close the polygon
        else if (click == 3) {
            if (sommets_x.size() >= 3) {
                // Draw line from last point back to the first point
                drawLine(p_prev.x, p_prev.y, p0.x, p0.y, c, 2);
                break; // Exit the loop
            }
        }
    }
}

// Technical Note: This implements the Ray-Casting Algorithm (Crossing Number).
// We simulate a horizontal ray starting from (x,y) going to the right (positive infinity).
// We count how many times this ray intersects with the polygon edges.
// Odd number of intersections = Inside. Even number = Outside.
bool PointinPoly(int x, int y, const std::vector<int>& poly_x, const std::vector<int>& poly_y) {
    size_t n = poly_x.size();
    bool inside = false;

    // Loop through every edge of the polygon.
    // 'i' is the current vertex, 'j' is the previous vertex (closing the loop).
    for (size_t i = 0, j = n - 1; i < n; j = i++) {
        double xi = poly_x[i], yi = poly_y[i];
        double xj = poly_x[j], yj = poly_y[j];

        // 1. Check if the point's Y coordinate is within the Y-range of the edge (yi, yj).
        //    (yi > y) != (yj > y) is a clever boolean XOR to check if y is between yi and yj.
        bool is_between_y = (yi > y) != (yj > y);

        if (is_between_y) {
            // 2. Calculate the intersection X-coordinate of the edge with the horizontal line at 'y'.
            //    Formula: x_intersect = (slope_inverse * delta_y) + x_start
            //    We add 1e-12 to the denominator to prevent division by zero (vertical lines).
            double intersection_x = (xj - xi) * (double)(y - yi) / (yj - yi + 1e-12) + xi;

            // 3. Check if our point is to the left of the intersection (Ray Casting).
            if (x < intersection_x) {
                inside = !inside; // Toggle the state
            }
        }
    }
    return inside;
}

Image<octet> MaskfromPoly(const std::vector<int>& poly_x, const std::vector<int>& poly_y, int width, int height) {
    Image<octet> mask(width, height);

    // Iterate over every pixel of the image bounding box
    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
            // Determine if the pixel is inside or outside
            // 0 = Inside (Valid source / Erased zone)
            // 1 = Outside
            mask(x, y) = PointinPoly(x, y, poly_x, poly_y) ? 0 : 1;
        }
    }
    return mask;
}

Image<octet> MaskfromPoly(const std::vector<int>& poly_x, const std::vector<int>& poly_y, const CriminisiImage& im) {
    // Helper overload to easily use image dimensions
    return MaskfromPoly(poly_x, poly_y, im.width(), im.height());
}
