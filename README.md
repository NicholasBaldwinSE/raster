# Rasterization project

This project includes a static, one frame .ppm image rasterizer written in C++ using the GLM mathematics library provided by OpenGL. This project was created for a grade for a college-level graphics course. Most code is original, with a few exceptions which will be noted duly below:

- All CMake configuration and setup was created by Marc Olano at UMBC when this course was taken and is considered public domain.
- The Teapot ray file is also considered public domain.

Documentation for the Rayshade file format can be found here: http://paulbourke.net/dataformats/rayshade/

All other code is completely original.

The code contains:
- Rayshade file parser. This is incomplete, as the specifications for the project given only required certain types of data.
- The rasterizer, going through multiple parts of the rasterization process in code to model how it works on a GPU (which is largely why this was not done in real time or with multiple frames, as rasterization is far better suited for the GPU).
- The file writer, which is some setup as well as data manipulation to get it to write the color data into the file. The file is written in the PPM file format, specification found here: https://netpbm.sourceforge.net/doc/ppm.html

To run this code, you first need a valid source of GLM installed somewhere on your computer. For Windows, this can be in the program files directory. On Linux and Mac devices, this can be in /usr/include or /usr/local/include. This can also be sidestepped by exporting the GLM_DIR environment variable with the source location of your choosing.

Once installed, you can then build the project using CMake. The generic command for this would be `cmake -S . -B ./build/` in the top level of this project. This README will make no assumptions as to what kind of build it makes, but for Windows it defaults to a Virtual Studio project and Mac/Linux defaults to a makefile (with Mac also potentially benefitting from use of XCode). Build however you like.

From this point, run using whatever build pipeline you like to get your result. The resulting file will be output to the ./build/ directory as `raster.ppm`. An example is not provided due to the format.