
### Debug Draw Samples

Three sample programs showing basic usage of the Debug Draw library are currently provided:

- `sample_null_renderer.cpp`: Just a tiny `main()` that calls a couple functions and exists.
  This sample is meant only to test if the library will compile without errors.

- `sample_gl_core.cpp`: Sample using core (shader-based) OpenGL that draws a handful of debug
  shapes using the library. You can move around using `W,A,S,D` or the arrow keys. Click on the
  window and drag the mouse to rotate the camera. This sample requires OpenGL version 3.2 or newer.

- `sample_gl_legacy.cpp`: Exact same as the core GL sample, but uses a legacy (AKA fixed-function)
  OpenGL renderer instead.

- `sample_gl_core_multithreaded_tls.cpp` and `sample_gl_core_multithreaded_explicit.cpp` are
  similar to the Core OpenGL sample but demonstrate how to use the TLS or explicit context modes.

- `sample_d3d11.cpp`: Windows sample using D3D11 as the renderer interface for Debug Draw.

- `samples_common.hpp`: Contains code shared by all samples, such as input handling and camera/controls.

----

To build the samples on Linux or MacOS, run the provided Makefile.
For Windows, Visual Studio projects are included for VS2015.

The only external dependency required to build the samples is [GLFW](http://www.glfw.org/). Make sure to install
it before attempting to build them. [GL3W](https://github.com/skaslev/gl3w) is our extension wrangler
for the Core OpenGL sample. It is included here and builds with DD. Sony's Vectormath library is also
a dependency, but it is included as well. Those libraries are only required for the samples and
not by Debug Draw itself.

The samples have only been officially tested on MacOS and Windows 7 & 10, though should build and run fine
on Linux, provided that you have GLFW installed. Running them requires at least OpenGL v3.2, but due
to hardware differences there's no guarantee that they will render the expected outputs, even if your machine supports GL3+.
Little to no handling is done to account for GL version and hardware differences, so the samples might not run properly on your hardware.

The main idea is that they serve as a rough guide on how to integrate Debug Draw into your own projects,
so there's more value in the code itself than in the demo applications, which are just drawing some line
shapes in the 3D world and some HUD text, nothing more.

