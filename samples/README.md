
### Debug Draw Samples

Three sample programs showing basic usage of the Debug Draw library are currently provided:

- `sample_null_renderer.cpp`: Just a tiny `main()` that calls a couple functions and exists.
  This sample is meant to test if the library will compile without errors.

- `sample_gl_core.cpp`: Sample using core (shader-based) OpenGL that draws a handful of debug
  shapes using the library. You can move around using `W,A,S,D` or the arrow keys. Click on the
  window and drag the mouse to rotate the camera. This sample requires OpenGL version 3.2 or better.

- `sample_gl_legacy.cpp`: Exact same as the core GL sample, but uses a legacy (AKA fixed-function)
  OpenGL renderer instead.

- `samples_common.hpp`: Contains code shared by all samples, such as input handling and camera/controls.

----

To build the samples, run the provided Makefile. Currently there's no build environment for Windows.

The only external dependency to build the samples is [GLFW](http://www.glfw.org/). Make sure to install
it before attempting to build them. [GL3W](https://github.com/skaslev/gl3w) is our extension wrangler
for the core GL sample. It is included here and builds with it. Sony's Vectormath library is also
a dependency, but it is included as well. Those libraries are only required for the samples and
not by Debug Draw itself.

The samples have only been officially tested on Mac OSX, though should build fine on Linux if you have GLFW
installed properly. Running them requires at least OpenGL v3.2, but due to hardware differences
there's no guarantee that they will render the expected outputs. Little to no handling is done to
account for GL version and hardware differences, so the samples might not run properly on your machine.

The main idea is that they serve as a rough guide on how to integrate Debug Draw into your own projects,
so there's more value in the code itself than in the demo applications, which are just drawing some line
shapes in the 3D world and HUD text, nothing more.

