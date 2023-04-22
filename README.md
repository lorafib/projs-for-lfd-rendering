# Efficient Rendering for Light Field Displays using Tailored Projective Mappings
![ThumbnailLightFieldDisplaysI3D](https://user-images.githubusercontent.com/115994315/233783803-a709f662-48e4-40d2-914f-39bfeca1929d.png)

## About using our code
* You do not need a light field display to run our code.
* You can add in your specific display calibration values per hand in lightfield.h -> Lightfield::setLightfieldParameters() if you do have a light field display. For the Looking Glass these can be found under /LKG_calibration/visual.json
* You can toggle between our algorithm and the standard procedure by pressing T. Our algorithm is the default.
* You can move the window to the light field display and back by pressing M. If you don't have a light field display, you can still view the interlaced image on your monitor and pressing M doubles the size of the displayed image.
* You can view timers and debug information like the individual textures by pressing F1.
* To take a screenshot, press the enter key. The screenshot can then be found in the same folder as the .exe

## Build instructions Windows
Download and unzip the code from Github. Open the unzipped folder as a new project in Visual Studio. You will need to have Desktop development with C++ and CMake-tools for Windows installed for Visual Studio. CMake configure and generate should automatically run in the console. After successful configuration, select the build target lfd_rendering.exe and click on the green arrow to build and run the program.

## Build instructions Linux
Download and unzip the code from Github. Build and run the program with CMake.
