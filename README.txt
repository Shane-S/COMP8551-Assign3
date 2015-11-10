README
=======
AUTHORS: Trevor Ware (A00844405), Shane Spoor (A00852406)

On Mac
* Move SDL2.framework and SDL2_image.framework to /Library/Frameworks
* cd /Library/Frameworks/SDL2.framework
* Fix the SDL2 framework’s signature by running (in the directory you just changed to) codesign -f -s - SDL2
* Open the Xcode project and build
* Move Filter.cl and cat.png to /Users/<your_username>/Library/Developer/Xcode/DerivedData/COMP8551-Assign3-<…>/Build/Products/Debug
* Run the project from Xcode

On Windows
* Open the Visual Studio project and build (making sure that the selected configuration is x64)
* Move the .dll files located in \Assign3\lib into \Assign3 (relative to the .sln file) to run from Visual Studio,
  into the output directory (x64\<Debug | Release>\) to run directly from there, or into System32 (recommended) to run from either location
* Move "Assign3\cat.png" and "Assign3\src\Filter.cl" to the output directory or \Assign3
* Run the project either from Visual Studio or directly from the output directory

For both:
* Running times are displayed in the console (note that the programs will be re-compiled each time, and this time isn’t included)
* The “G” key runs the program on the GPU
* The “C” key runs the program on the CPU (with OpenCL)
* The “B” key runs the program on both devices (if available)
* The “S” key runs the program serially on the CPU

Notes
* On Windows AMD drivers, the CPU and GPU combined don't correctly write to part of the image. We tested this on Mac and on
  NVidia drivers and both worked fine, so we don't know why this is. If you find some mistake that we haven't noticed, please let us
  know.
