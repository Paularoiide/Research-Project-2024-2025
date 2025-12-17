Criminisi Inpainting Algorithm Implementation
------------------------------------------------------------------------------------------
This project is a C++ implementation of the region-filling algorithm described in the paper:
A. Criminisi, P. Pérez, and K. Toyama, "Region Filling and Object Removal by Exemplar-Based Image Inpainting," IEEE Transactions on Image Processing, vol. 13, no. 9, pp. 1200–1212, 2004.

Research Project in collaboration with Antoine Salomon, supervised by Pascal Monasse (Computer Vision Researcher at LIGM).

----------------------------------------Overview----------------------------------------
This algorithm fills missing portions of an image with a visually coherent background. It leverages the structural information of the image and operates by dividing it into small square regions called patches.
A brief research paper included in this repository explains the core concepts of the method and analyzes the results achieved with this implementation.

----------------------------------------Usage------------------------------------------
Select the Removal Zone (Red):
Left-click points on the image to define the polygon you want to remove.
Right-click to close the polygon.

Select the Source Zone (Blue):
Left-click points to define the area where the algorithm should search for texture.
Right-click to close the polygon.

----------------------------------------Process----------------------------------------
Click anywhere to start the inpainting process. The red area will be filled automatically.

----------------------------------------Configuration----------------------------------------
You can modify the following parameters in main.cpp:
patch_size: Increasing this value speeds up computation but may lead to less precise results (must be an odd number).
Image source: Change the filename in main.cpp to process a different image.
