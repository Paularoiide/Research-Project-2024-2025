#include <iostream>
#include <Imagine/Graphics.h>
#include <Imagine/Images.h>
#include "image.h"
#include "patch.h"
#include "polygons.h"

using namespace Imagine;
using namespace std;

int main() {
    try {
        // Setup: Load image and parameters
        std::string image_path = "assets/image_init.png";
        int patch_size = 9; // Larger patches preserve texture better but blur structures

        std::cout << "Loading image: " << image_path << std::endl;
        CriminisiImage im(image_path, patch_size);

        // Open graphics window
        openWindow(im.width(), im.height(), "Criminisi Inpainting Project");

        // Interaction Loop
        while (true) {
            im.show();

            // Step 1: User selects the region to remove (Target)
            std::cout << "\n--- STEP 1: Define Removal Zone ---" << std::endl;
            std::cout << "Draw a RED polygon around the object to remove." << std::endl;
            std::cout << "[Left Click] Add Point, [Right Click] Close Shape." << std::endl;

            std::vector<int> poly_x, poly_y;
            FillPolygon(poly_x, poly_y, RED);

            // Step 2: User selects the search region (Source)
            // This restricts where the algorithm looks for texture, improving speed and quality.
            std::cout << "\n--- STEP 2: Define Search Zone ---" << std::endl;
            std::cout << "Draw a BLUE polygon around the valid texture area (e.g., background)." << std::endl;
            std::cout << "[Left Click] Add Point, [Right Click] Close Shape." << std::endl;

            std::vector<int> source_x, source_y;
            FillPolygon(source_x, source_y, BLUE);

            // Create masks
            // 'search_zone' is 0 where valid, 1 where invalid
            Image<octet> search_zone = MaskfromPoly(source_x, source_y, im);

            std::cout << "Click anywhere to start processing..." << std::endl;
            click();

            // Step 3: Execution
            // Apply the hole (blackout)
            im.erasePoly(poly_x, poly_y);
            im.show();

            // Run Algorithm
            im.criminisi_focus(50000, search_zone);

            std::cout << "Processing finished. Click to exit." << std::endl;
            click();
            break; // Exit after one successful run
        }

        endGraphics();
        return 0;
    } catch (const std::exception& e) {
        std::cerr << "Critical Error: " << e.what() << std::endl;
        return 1;
    }
}
