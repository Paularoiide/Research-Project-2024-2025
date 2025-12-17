#pragma once

#include <vector>
#include <Imagine/Images.h>
#include <Imagine/Graphics.h>
#include "image.h" // Needed for 'octet' and 'CriminisiImage' definitions

/**
 * @brief Interactive function to draw a polygon on the screen.
 * * The user clicks to add points (Left Click) and closes the shape (Right Click).
 * * @param sommets_x Output vector storing the X coordinates of the vertices.
 * @param sommets_y Output vector storing the Y coordinates of the vertices.
 * @param c The color used to draw the polygon edges and vertices.
 */
void FillPolygon(std::vector<int>& sommets_x, std::vector<int>& sommets_y, Imagine::Color c);

/**
 * @brief Checks if a pixel (x,y) lies inside a polygon using the Ray-Casting algorithm.
 * * This function implements the "Even-Odd Rule": a point is inside if a ray
 * cast from the point to infinity crosses the polygon's edges an odd number of times.
 * * @param x The X coordinate of the point to test.
 * @param y The Y coordinate of the point to test.
 * @param poly_x Vector containing X coordinates of polygon vertices.
 * @param poly_y Vector containing Y coordinates of polygon vertices.
 * @return true if the point is inside the polygon, false otherwise.
 */
bool PointinPoly(int x, int y, const std::vector<int>& poly_x, const std::vector<int>& poly_y);

/**
 * @brief Generates a binary mask from a polygon definition.
 * * Creates an image where pixels inside the polygon are 0 and outside are 1.
 * * @param poly_x Vector containing X coordinates of polygon vertices.
 * @param poly_y Vector containing Y coordinates of polygon vertices.
 * @param width The width of the generated mask.
 * @param height The height of the generated mask.
 * @return Imagine::Image<octet> Binary mask (0=Inside, 1=Outside).
 */
Imagine::Image<octet> MaskfromPoly(const std::vector<int>& poly_x, const std::vector<int>& poly_y, int width, int height);

/**
 * @brief Overload of MaskfromPoly that extracts dimensions from a CriminisiImage object.
 * * @param poly_x Vector containing X coordinates of polygon vertices.
 * @param poly_y Vector containing Y coordinates of polygon vertices.
 * @param im Reference image to match dimensions.
 * @return Imagine::Image<octet> Binary mask (0=Inside, 1=Outside).
 */
Imagine::Image<octet> MaskfromPoly(const std::vector<int>& poly_x, const std::vector<int>& poly_y, const CriminisiImage& im);
