#pragma once

#include <Imagine/Graphics.h>
#include <Imagine/Images.h>
#include <vector>
#include <string>
#include <cmath>

// Define 'octet' as an alias for unsigned char (0-255)
using octet = unsigned char;

// Forward declaration to avoid circular dependencies
class Patch;

/**
 * @brief Represents a 2D integer coordinate in the image.
 */
struct Point {
    int x; /**< The X coordinate (column) */
    int y; /**< The Y coordinate (row) */

    Point(int x = 0, int y = 0);
    bool operator==(const Point& other) const;
};

/**
 * @brief Core class implementing the Criminisi Exemplar-Based Inpainting algorithm.
 * * This class manages the image data, the target region mask, and the priority
 * calculations required to determine the order of filling.
 */
class CriminisiImage {
private:
    Imagine::Image<Imagine::Color> image; /**< The main image being modified */

    // --- Algorithmic Buffers ---
    Imagine::Image<octet> mask;         /**< Binary mask: 1 = Target region (to fill), 0 = Source region (valid) */
    Imagine::Image<float> confidence;   /**< Confidence map C(p): Measures the reliability of pixel information */
    Imagine::Image<octet> fillFront;    /**< Boundary between the source and target regions */

    // --- Data Structures ---
    std::vector<Point> contour;         /**< List of points currently on the fill front (boundary) */
    std::vector<std::pair<Point, float>> priority_list; /**< Stores filling priority P(p) for contour points */
    std::vector<Patch> patch_list;      /**< List of candidate source patches used for searching */

    int patch_size; /**< Size of the square patch (e.g., 9x9). Must be odd. */

    // --- Intermediate Gradients ---
    Imagine::Image<float> image_gray;   /**< Grayscale version of the image for gradient calculation */
    Imagine::Image<float> grad_x;       /**< Gradient along X axis */
    Imagine::Image<float> grad_y;       /**< Gradient along Y axis */

    // --- Display Buffers ---
    octet* r; /**< Red channel buffer for Imagine++ display */
    octet* g; /**< Green channel buffer for Imagine++ display */
    octet* b; /**< Blue channel buffer for Imagine++ display */

public:
    /**
     * @brief Constructor. Loads the image and initializes data structures.
     * @param image_path Path to the source image file.
     * @param size Size of the patch (must be odd, e.g., 9).
     */
    CriminisiImage(const std::string& image_path, int size);

    /**
     * @brief Destructor. Frees manually allocated display buffers.
     */
    ~CriminisiImage();

    // --- Visualization & Setup ---
    void show();
    void setPatchSize(int size);

    // --- Mask Management ---
    /**
     * @brief Scans the image to identify black pixels (0,0,0) as the initial hole.
     */
    void computeMask();
    void eraseZone(const Point& topLeft, const Point& bottomRight);
    void erasePoly(const std::vector<int>& list_x, const std::vector<int>& list_y);
    void nanToZeros(); // Helper to clean up bad pixels

    // --- Algorithm Steps ---

    /**
     * @brief Identifies the boundary (contour) between valid and invalid regions.
     * Populates the 'contour' vector.
     */
    void findContours();
    void colorToGray();

    /**
     * @brief Computes image gradients, handling missing data (NaN/Masked regions).
     * Uses a robust 8-neighbor approach to estimate gradients near holes.
     */
    void computeGradientWithNan();

    /**
     * @brief Computes the unit normal vector to the fill front at point p.
     * @param p The point on the contour.
     * @return std::vector<float> {nx, ny}
     */
    std::vector<float> computeNormalAtP(const Point& p);

    /**
     * @brief Calculates priority P(p) for all points on the contour.
     * P(p) = Confidence(p) * Data(p)
     */
    void computePriorityList();

    /**
     * @brief Pre-calculates all valid source patches in the image.
     * Warning: Computationally expensive ($O(N)$ where N is number of pixels).
     */
    void findPatchList();

    /**
     * @brief Optimized version: Finds source patches only within a user-defined search zone.
     * @param search_zone Binary mask defining where to look for textures.
     */
    void findPatchList_focus(Imagine::Image<octet> search_zone);

    // --- Main Loops ---
    void criminisi(int maxIter = 10000);

    /**
     * @brief Main Inpainting Loop with focused search.
     * 1. Compute priorities.
     * 2. Select best patch on the fill front.
     * 3. Find best match in the search zone (SSD).
     * 4. Copy texture and update mask.
     */
    void criminisi_focus(int maxIter, Imagine::Image<octet> search_zone);

    // --- Getters ---
    int width() const;
    int height() const;
    int getPatchSize() const;

    Imagine::Image<Imagine::Color>& getImage();
    Imagine::Image<octet>& getMask();
    Imagine::Image<float>& getConfidence();
    Imagine::Image<octet>& getFillFront();
    Imagine::Image<float>& getImageGray();
    Imagine::Image<float>& getGradX();
    Imagine::Image<float>& getGradY();
    const std::vector<Point>& getContour() const;
};
