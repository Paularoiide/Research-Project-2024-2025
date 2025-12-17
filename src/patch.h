#pragma once
#include "image.h"

/**
 * @brief Represents a square region around a central pixel.
 * Used for comparing textures (SSD) and copying content.
 */
class Patch {
private:
    CriminisiImage& image; /**< Reference to the parent image */

public:
    Point center; /**< Coordinates of the patch center */
    int half_window; /**< Half-size of the patch (radius). e.g., if size=9, half_window=4 */

    /**
     * @brief Construct a new Patch object
     * @param center Center point (x,y)
     * @param image Reference to the CriminisiImage object
     */
    Patch(const Point& center, CriminisiImage& image);

    /**
     * @brief Extracts the RGB colors of the patch region.
     * @return Imagine::Image<Imagine::Color> Small image containing the patch data.
     */
    Imagine::Image<Imagine::Color> extractRegion() const;

    /**
     * @brief Computes the Sum of Squared Differences (SSD) between this patch and another.
     * Crucial: Only compares valid pixels (already existing data).
     * @param other The source patch to compare against.
     * @return float The distance score (lower is better).
     */
    float ssd(const Patch& other);

    /**
     * @brief Copies data from the 'other' patch to the current patch.
     * Only copies into pixels that are currently masked (missing).
     */
    void updatePatch(const Patch& other);

    /**
     * @brief Computes the Confidence Term C(p).
     * C(p) is the average confidence of all known pixels in the patch.
     */
    float computeConfidence();

    /**
     * @brief Computes the final Priority P(p) = C(p) * D(p).
     * Combines Confidence and Data terms (Isophote direction).
     */
    float computePriority();
};
