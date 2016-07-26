
// ================================================================================================
// -*- C++ -*-
// File: debug_draw.hpp
// Author: Guilherme R. Lampert
// Created on: 10/12/15
// Brief: Debug Draw - an immediate-mode, renderer agnostic, lightweight debug drawing API.
// ================================================================================================

#ifndef DEBUG_DRAW_HPP
#define DEBUG_DRAW_HPP

// ========================================================
// Library Overview:
// ========================================================
//
// ---------
//  LICENSE
// ---------
// This software is in the public domain. Where that dedication is not recognized,
// you are granted a perpetual, irrevocable license to copy, distribute, and modify
// this file as you see fit.
//
// The source code is provided "as is", without warranty of any kind, express or implied.
// No attribution is required, but a mention about the author(s) is appreciated.
//
// -------------
//  QUICK SETUP
// -------------
// In *one* C++ source file, *before* including this file, do this:
//
//   #define DEBUG_DRAW_IMPLEMENTATION
//
// To enable the implementation. Further includes of this
// file *should not* redefine DEBUG_DRAW_IMPLEMENTATION.
// Example:
//
// In my_program.cpp:
//
//   #define DEBUG_DRAW_IMPLEMENTATION
//   #include "debug_draw.hpp"
//
// In my_program.hpp:
//
//   #include "debug_draw.hpp"
//
// -------------------
//  COMPILER SWITCHES
// -------------------
// Several compiler switches are provided for library customization. Check the
// following section for a detailed description of each. The noteworthy ones are:
//
// DEBUG_DRAW_CXX11_SUPPORTED
//  Enables the use of some C++11 features. If your compiler supports C++11
//  or better, you should define this switch globally or before every inclusion
//  of this file. If it is not defined, we try to guess it from the value of the
//  '__cplusplus' built-in macro constant.
//
// DEBUG_DRAW_MAX_XYZ
//  Sizes of internal intermediate buffers, which are statically allocated
//  in the implementation. If you need to draw more primitives than the sizes
//  of these buffers, you need to redefine them and recompile.
//
// DEBUG_DRAW_VERTEX_BUFFER_SIZE
//  Size in dd::DrawVertex elements of the intermediate vertex buffer used
//  to batch primitives before sending them to dd::RenderInterface. A bigger
//  buffer will reduce the number of calls to dd::RenderInterface when drawing
//  large sets of debug primitives.
//
// DEBUG_DRAW_OVERFLOWED(message)
//  An error handler called if the DEBUG_DRAW_MAX_XYZ sizes overflow.
//  By default it just prints a message to stderr.
//
// DEBUG_DRAW_USE_STD_MATH
//  If defined to nonzero, use cmath/math.h. If you redefine it to zero before
//  the DD implementation, it will force the use of local replacements
//  for the library. This might be useful if you want to avoid the dependency.
//  It is defined to zero by default (i.e. we use cmath by default).
//
// DEBUG_DRAW_XYZ_TYPE_DEFINED
//  The compound types used by the DD library can also be customized.
//  By default, ddVec3 and ddMat4x4 are plain C-arrays, but you can
//  redefine them to use your own classes or structures (see below).
//  ddStr is by default a std::string, but you can redefine it to
//  a custom string type if necessary. The only requirement is that
//  it provides a 'c_str()' method returning a null terminated const char* string.
//
// DEBUG_DRAW_STR_DEALLOC_FUNC(str)
//  If you define a custom string type for ddStr and it requires some
//  extra cleanup, you might define this function macro to perform the cleanup.
//  It is called by dd::clear() and dd::shutdown() on every instance of the
//  internal DebugString buffer.
//
// -------------------
//  MEMORY ALLOCATION
// -------------------
// Debug Draw will only perform one memory allocation during startup to decompress
// the built-in glyph bitmap used for debug text rendering. All the vertex buffers
// and intermediate draw/batch buffers used internally are declared as static
// C-style arrays with fixed size.
//
// Memory allocation & deallocation for the glyph bitmap decompression will be done via:
//
//   DD_MALLOC(size)
//   DD_MFREE(ptr)
//
// These two macros can be redefined if you'd like to supply you own memory allocator.
// By default, they are defined to use std::malloc and std::free, respectively.
// Note: If you redefine one, you must also provide the other!
//
// --------------------------------
//  INTERFACING WITH YOUR RENDERER
// --------------------------------
// Debug Draw doesn't touch on any renderer-specific aspects or APIs, instead you provide
// DD with all of it's rendering needs via the dd::RenderInterface abstract class.
//
// See the definition of dd::RenderInterface for the details. Not all methods are
// required. In fact, you could also implement a full no-op RenderInterface that
// disables debug drawing by simply inheriting from RenderInterface and not overriding
// any of the methods (or even easier, call dd::initialize(nullptr) to make everything a no-op).
//
// For examples on how to implement your own RenderInterface, see the accompanying samples.
// You can also find them in the source code repository for this project:
// https://github.com/glampert/debug-draw
//
// ------------------
//  CONVENTIONS USED
// ------------------
// Points and lines are always specified in world-space positions. This also
// applies to shapes drawn from lines, like boxes, spheres, cones, etc.
//
// 2D screen-text is in screen-space pixels (from 0,0 in the upper-left
// corner of the screen to screen_width-1 and screen_height-1).
// RenderInterface::drawGlyphList() also receives vertexes in screen-space.
//
// We make some usage of matrices for things like the projected text labels.
// Matrix layout used is column-major and vectors multiply as columns.
// This is the convention normally used by standard OpenGL.
//
// C++ Exceptions are not used. Little error checking is provided or
// done inside the library. We favor simpler, faster and easier to maintain
// code over more sophisticated error handling. The rationale is that a
// debug drawing API doesn't have to be very robust, since it won't make
// into the final release executable in most cases.
//
// END OF DOCUMENTATION

// ========================================================
// Compiler switches:
// ========================================================

//
// If the user didn't specify if C++11 or above are supported, try to guess
// from the value of '__cplusplus'. It should be 199711L for pre-C++11 compilers
// and 201103L in those supporting C++11, but this is not a guarantee that all
// C++11 features will be available and stable, so again, we are making a guess.
// It is recommended to instead supply the DEBUG_DRAW_CXX11_SUPPORTED switch
// yourself before including this file.
//
#ifndef DEBUG_DRAW_CXX11_SUPPORTED
    #if (__cplusplus > 199711L)
        #define DEBUG_DRAW_CXX11_SUPPORTED 1
    #endif // __cplusplus
#endif // DEBUG_DRAW_CXX11_SUPPORTED

//
// Max elements of each type at any given time.
// We supply these reasonable defaults, but you can provide your
// own tunned values to save memory or fit all of your debug data.
// These are hard constraints. If not enough, change and recompile.
//
#ifndef DEBUG_DRAW_MAX_STRINGS
    #define DEBUG_DRAW_MAX_STRINGS 512
#endif // DEBUG_DRAW_MAX_STRINGS

#ifndef DEBUG_DRAW_MAX_POINTS
    #define DEBUG_DRAW_MAX_POINTS 8192
#endif // DEBUG_DRAW_MAX_POINTS

#ifndef DEBUG_DRAW_MAX_LINES
    #define DEBUG_DRAW_MAX_LINES 32768
#endif // DEBUG_DRAW_MAX_LINES

//
// Size in vertexes of a local buffer we use to sort elements
// drawn with and without depth testing before submitting them to
// the RenderInterface. A larger buffer will require less flushes
// (e.g. RenderInterface calls) when drawing large amounts of
// primitives. Less will obviously save more memory. Each DrawVertex
// is about 32 bytes in size, we declare a local static array with
// this many entries.
//
#ifndef DEBUG_DRAW_VERTEX_BUFFER_SIZE
    #define DEBUG_DRAW_VERTEX_BUFFER_SIZE 4096
#endif // DEBUG_DRAW_VERTEX_BUFFER_SIZE

//
// This macro is called with an error message if any of the above
// sizes is overflowed during runtime. In a debug build, you might
// keep this enabled to be able to log and find out if more space
// is needed for the debug data arrays. Default output is stderr.
//
#ifndef DEBUG_DRAW_OVERFLOWED
    #include <iostream>
    #define DEBUG_DRAW_OVERFLOWED(message) std::cerr << message << "\n"
#endif // DEBUG_DRAW_OVERFLOWED

//
// Use <math.h> and <float.h> for trigonometry functions by default.
// If you wish to avoid those dependencies, DD provides local approximations
// of the required functions as a portable replacement. Just define
// DEBUG_DRAW_USE_STD_MATH to zero before including this file.
//
#ifndef DEBUG_DRAW_USE_STD_MATH
    #define DEBUG_DRAW_USE_STD_MATH 1
#endif // DEBUG_DRAW_USE_STD_MATH

// ========================================================
// Overridable Debug Draw types:
// ========================================================

//
// Following typedefs are not members of the dd:: namespace to allow easy redefinition by the user.
// If you provide a custom implementation for them before including this file, be sure to
// also define the proper DEBUG_DRAW_XYZ_TYPE_DEFINED switch to disable the default typedefs.
//
// The only requirement placed on the vector/matrix types is that they provide
// an array subscript operator [] and have the expected number of elements. Apart
// from that, they could be structs, classes, what-have-you. POD types are recommended
// but not mandatory.
//

#ifndef DEBUG_DRAW_VEC3_TYPE_DEFINED
    // ddVec3:
    //  A small array of floats with at least three elements, but
    //  it could have more for alignment purposes, extra slots are ignored.
    //  A custom ddVec3 type must provide the array subscript operator.
    typedef float ddVec3[3];

    // ddVec3Param:
    //  Since our default ddVec3 is a plain C-array, it decays to a pointer
    //  when passed as an input parameter to a function, so we can use it directly.
    //  If you change it to some structured type, it might be more efficient
    //  passing by const reference instead, however, some platforms have optimized
    //  hardware registers for vec3s/vec4s, so passing by value might also be efficient.
    typedef const ddVec3 ddVec3Param;

    #define DEBUG_DRAW_VEC3_TYPE_DEFINED 1
#endif // DEBUG_DRAW_VEC3_TYPE_DEFINED

#ifndef DEBUG_DRAW_MAT4X4_TYPE_DEFINED
    // ddMat4x4:
    //  Homogeneous matrix of 16 floats, representing rotations as well as
    //  translation/scaling and projections. The internal matrix layout used by this
    //  library is COLUMN-MAJOR, vectors multiplying as columns (usual OpenGL convention).
    //  Column-major matrix layout:
    //          c.0   c.1   c.2    c.3
    //    r.0 | 0.x   4.x   8.x    12.x |
    //    r.1 | 1.y   5.y   9.y    13.y |
    //    r.2 | 2.z   6.z   10.z   14.z |
    //    r.3 | 3.w   7.w   11.w   15.w |
    //  If your custom matrix type uses row-major format internally, you'll
    //  have to transpose them before passing your matrices to the DD functions.
    //  We use the array subscript operator internally, so it must also be provided.
    typedef float ddMat4x4[4 * 4];

    // ddMat4x4Param:
    //  Since our default ddMat4x4 is a plain C-array, it decays to a pointer
    //  when passed as an input parameter to a function, so we can use it directly.
    //  If you change it to some structured type, it might be more efficient
    //  passing by const reference instead.
    typedef const ddMat4x4 ddMat4x4Param;

    #define DEBUG_DRAW_MAT4X4_TYPE_DEFINED 1
#endif // DEBUG_DRAW_MAT4X4_TYPE_DEFINED

#ifndef DEBUG_DRAW_STRING_TYPE_DEFINED
    // ddStr:
    //  String type used internally to store the debug text strings.
    //  A custom string type must provide at least an assignment
    //  operator (=) and a 'c_str()' method that returns a
    //  null-terminated const char* string pointer. That's it.
    //  An array subscript operator [] is not required for ddStr.
    #include <string>
    typedef std::string ddStr;

    // ddStrParam:
    //  If we have C++11, the correct usage for std::string is to
    //  pass by value and move when storing it. Pre-11 usage is
    //  pass by const reference and copy-assign.
    #if DEBUG_DRAW_CXX11_SUPPORTED
        typedef std::string ddStrParam;
    #else // !C++11
        typedef const std::string & ddStrParam;
    #endif // DEBUG_DRAW_CXX11_SUPPORTED

    #define DEBUG_DRAW_STRING_TYPE_DEFINED 1
#endif // DEBUG_DRAW_STRING_TYPE_DEFINED

#ifndef DEBUG_DRAW_INT_TYPES_DEFINED
    // ddI64:
    //  64-bits integer type used to keep track of times in milliseconds.
    //  If your platform is missing cstdint/stdint.h you can redefine it
    //  to some platform-specific equivalent. If not compiling C++11, we
    //  assume cstdint is missing and try a MSVC extension or the 'long long' type.
    //
    // ddU32:
    //  32-bits unsigned integer. Used internally for int=>float casts
    //  (mainly only if using the local fast-math approximations).
    //  Make sure it is not smaller than sizeof(float).
    #if DEBUG_DRAW_CXX11_SUPPORTED
        #include <cstdint>
        typedef std::int64_t  ddI64;
        typedef std::uint32_t ddU32;
    #else // !C++11
        #ifdef _MSC_VER
            typedef          __int64 ddI64;
            typedef unsigned __int32 ddU32;
        #else // !_MSC_VER
            typedef    long long ddI64;
            typedef unsigned int ddU32;
        #endif // _MSC_VER
    #endif // DEBUG_DRAW_CXX11_SUPPORTED

    #define DEBUG_DRAW_INT_TYPES_DEFINED 1
#endif // DEBUG_DRAW_INT_TYPES_DEFINED

namespace dd
{

// ========================================================
// Debug Draw functions:
// Durations are always in milliseconds.
// Colors are RGB floats in the [0,1] range.
// Positions are in world-space, unless stated otherwise.
// ========================================================

// Add a point in 3D space to the debug draw queue.
// Point is expressed in world-space coordinates.
// Note that not all renderer support configurable point
// size, so take the 'size' parameter as a hint only
void point(ddVec3Param pos,
           ddVec3Param color,
           float size = 1.0f,
           int durationMillis = 0,
           bool depthEnabled = true);

// Add a 3D line to the debug draw queue. Note that
// lines are expressed in world coordinates, and so are
// all wireframe primitives which are built from lines.
void line(ddVec3Param from,
          ddVec3Param to,
          ddVec3Param color,
          int durationMillis = 0,
          bool depthEnabled = true);

// Add a 2D text string as an overlay to the current view, using a built-in font.
// Position is in screen-space pixels, origin at the top-left corner of the screen.
// The third element (Z) of the position vector is ignored.
// Note: Newlines and tabs are handled (1 tab = 4 spaces).
void screenText(ddStrParam str,
                ddVec3Param pos,
                ddVec3Param color,
                float scaling = 1.0f,
                int durationMillis = 0);

// Add a 3D text label centered at the given world position that
// gets projected to screen-space. The label always faces the viewer.
// sx/sy, sw/sh are the viewport coordinates/size, in pixels.
// 'vpMatrix' is the view * projection transform to map the text from 3D to 2D.
void projectedText(ddStrParam str,
                   ddVec3Param pos,
                   ddVec3Param color,
                   ddMat4x4Param vpMatrix,
                   int sx, int sy,
                   int sw, int sh,
                   float scaling = 1.0f,
                   int durationMillis = 0);

// Add a set of three coordinate axis depicting the position and orientation of the given transform.
// 'size' defines the size of the arrow heads. 'length' defines the length of the arrow's base line.
void axisTriad(ddMat4x4Param transform,
               float size, float length,
               int durationMillis = 0,
               bool depthEnabled = true);

// Add a 3D line with an arrow-like end to the debug draw queue.
// 'size' defines the arrow head size. 'from' and 'to' the length of the arrow's base line.
void arrow(ddVec3Param from,
           ddVec3Param to,
           ddVec3Param color,
           float size,
           int durationMillis = 0,
           bool depthEnabled = true);

// Add an axis-aligned cross (3 lines converging at a point) to the debug draw queue.
// 'length' defines the length of the crossing lines.
// 'center' is the world-space point where the lines meet.
void cross(ddVec3Param center,
           float length,
           int durationMillis = 0,
           bool depthEnabled = true);

// Add a wireframe circle to the debug draw queue.
void circle(ddVec3Param center,
            ddVec3Param planeNormal,
            ddVec3Param color,
            float radius,
            float numSteps,
            int durationMillis = 0,
            bool depthEnabled = true);

// Add a wireframe plane in 3D space to the debug draw queue.
// If 'normalVecScale' is not zero, a line depicting the plane normal is also draw.
void plane(ddVec3Param center,
           ddVec3Param planeNormal,
           ddVec3Param planeColor,
           ddVec3Param normalVecColor,
           float planeScale,
           float normalVecScale,
           int durationMillis = 0,
           bool depthEnabled = true);

// Add a wireframe sphere to the debug draw queue.
void sphere(ddVec3Param center,
            ddVec3Param color,
            float radius,
            int durationMillis = 0,
            bool depthEnabled = true);

// Add a wireframe cone to the debug draw queue.
// The cone 'apex' is the point where all lines meet.
// The length of the 'dir' vector determines the thickness.
// 'baseRadius' & 'apexRadius' are in degrees.
void cone(ddVec3Param apex,
          ddVec3Param dir,
          ddVec3Param color,
          float baseRadius,
          float apexRadius,
          int durationMillis = 0,
          bool depthEnabled = true);

// Wireframe box from the eight points that define it.
void box(const ddVec3 points[8],
         ddVec3Param color,
         int durationMillis = 0,
         bool depthEnabled = true);

// Add a wireframe box to the debug draw queue.
void box(ddVec3Param center,
         ddVec3Param color,
         float width,
         float height,
         float depth,
         int durationMillis = 0,
         bool depthEnabled = true);

// Add a wireframe Axis Aligned Bounding Box (AABB) to the debug draw queue.
void aabb(ddVec3Param mins,
          ddVec3Param maxs,
          ddVec3Param color,
          int durationMillis = 0,
          bool depthEnabled = true);

// Add a wireframe frustum pyramid to the debug draw queue.
// 'invClipMatrix' is the inverse of the matrix defining the frustum
// (AKA clip) volume, which normally consists of the projection * view matrix.
// E.g.: inverse(projMatrix * viewMatrix)
void frustum(ddMat4x4Param invClipMatrix,
             ddVec3Param color,
             int durationMillis = 0,
             bool depthEnabled = true);

// Add a vertex normal for debug visualization.
// The normal vector 'normal' is assumed to be already normalized.
void vertexNormal(ddVec3Param origin,
                  ddVec3Param normal,
                  float length,
                  int durationMillis = 0,
                  bool depthEnabled = true);

// Add a "tangent basis" at a given point in world space.
// Color scheme used is: normal=WHITE, tangent=YELLOW, bi-tangent=MAGENTA.
// The normal vector, tangent and bi-tangent vectors are assumed to be already normalized.
void tangentBasis(ddVec3Param origin,
                  ddVec3Param normal,
                  ddVec3Param tangent,
                  ddVec3Param bitangent,
                  float lengths,
                  int durationMillis = 0,
                  bool depthEnabled = true);

// Makes a 3D square grid of lines along the X and Z planes.
// 'y' defines the height in the Y axis where the grid is placed.
// The grid will go from 'mins' to 'maxs' units in both the X and Z.
// 'step' defines the gap between each line of the grid.
void xzSquareGrid(float mins, float maxs,
                  float y,    float step,
                  ddVec3Param color,
                  int durationMillis = 0,
                  bool depthEnabled = true);

// ========================================================
// Debug Draw vertex type:
// The only drawing type the user has to interface with.
// ========================================================

union DrawVertex
{
    struct
    {
        float x, y, z;
        float r, g, b;
        float size;
    } point;

    struct
    {
        float x, y, z;
        float r, g, b;
    } line;

    struct
    {
        float x, y;
        float u, v;
        float r, g, b;
    } glyph;
};

//
// Opaque handle to a texture object.
// Used by the debug text drawing functions.
//
struct OpaqueTextureType;
typedef OpaqueTextureType * GlyphTextureHandle;

// ========================================================
// Debug Draw rendering callbacks:
// Implementation is provided by the user so we don't
// tie this code directly to a specific rendering API.
// ========================================================

class RenderInterface
{
public:

    //
    // These are called by dd::flush() before any drawing and after drawing is finished.
    // User can override these to perform any common setup for subsequent draws and to
    // cleanup afterwards. By default, no-ops stubs are provided.
    //
    virtual void beginDraw();
    virtual void endDraw();

    //
    // Create/free the glyph bitmap texture used by the debug text drawing functions.
    // The debug renderer currently only creates one of those on startup.
    //
    // You're not required to implement these two if you don't care about debug text drawing.
    // Default no-op stubs are provided by default, which disable debug text rendering.
    //
    // Texture dimensions are in pixels, data format is always 8-bits per pixel (Grayscale/GL_RED).
    // The pixel values range from 255 for a pixel within a glyph to 0 for a transparent pixel.
    // If createGlyphTexture() returns null, the renderer will disable all text drawing functions.
    //
    virtual GlyphTextureHandle createGlyphTexture(int width, int height, const void * pixels);
    virtual void destroyGlyphTexture(GlyphTextureHandle glyphTex);

    //
    // Batch drawing methods for the primitives used by the debug renderer.
    // If you don't wish to support a given primitive type, don't override the method.
    //
    virtual void drawPointList(const DrawVertex * points, int count, bool depthEnabled);
    virtual void drawLineList (const DrawVertex * lines,  int count, bool depthEnabled);
    virtual void drawGlyphList(const DrawVertex * glyphs, int count, GlyphTextureHandle glyphTex);

    // User defined cleanup. Nothing by default.
    virtual ~RenderInterface() = 0;
};

// ========================================================
// Housekeeping functions:
// ========================================================

// Flags for dd::flush()
enum FlushFlags
{
    FlushPoints = 1 << 1,
    FlushLines  = 1 << 2,
    FlushText   = 1 << 3,
    FlushAll    = (FlushPoints | FlushLines | FlushText)
};

// Initialize with the user-supplied renderer interface.
// Given object must remain valid until after dd::shutdown() is called!
// If 'renderer' is null, the Debug Draw functions become no-ops, but
// can still be safely called.
void initialize(RenderInterface * renderer);

// After this is called, it is safe to dispose the RenderInterface instance
// you passed to dd::initialize(). Shutdown will also attempt to free the glyph texture.
void shutdown();

// Test if there's data in the debug draw queue and dd::flush() should be called.
bool hasPendingDraws();

// Actually calls the RenderInterface to consume the debug draw queues.
// Objects that have expired their lifetimes get removed.
// Pass the current application time in milliseconds to remove
// timed objects that have expired. Passing zero removes all
// objects after they get drawn, regardless of lifetime.
void flush(ddI64 currTimeMillis, int flags = FlushAll);

// Manually removes all queued debug render data without drawing.
// This is not normally called. To draw stuff, call dd::flush() instead.
void clear();

} // namespace dd {}

// ================== End of header file ==================
#endif // DEBUG_DRAW_HPP
// ================== End of header file ==================

// ================================================================================================
//
//                                  Debug Draw Implementation
//
// ================================================================================================

#ifdef DEBUG_DRAW_IMPLEMENTATION

//
// Suppresses
//   "declaration requires an exit-time destructor [-Wexit-time-destructors]"
// and
//   "declaration requires a global constructor [-Wglobal-constructors]"
// on Clang/llvm-GCC.
//
// We declare static file-level arrays
// for the debug strings, lines and points, which
// might have constructors/destructors if redefined
// by the library user. Our default DebugString type
// uses std::string, so it has an implicit constructor
// and destructor pair, which also triggers these two
// warnings if they are enabled.
//
#ifdef __clang__
    #pragma clang diagnostic push
    #pragma clang diagnostic ignored "-Wexit-time-destructors"
    #pragma clang diagnostic ignored "-Wglobal-constructors"
#endif // __clang__

// ========================================================
// The DD_* macros are for internal use and get
// #undefined at the end of the implementation.
// ========================================================

//
// C++11 goodies:
//
#if DEBUG_DRAW_CXX11_SUPPORTED
    #include <utility>
    #define DD_MOVE(expr) std::move(expr)
    #define DD_NULL       nullptr
#else // !C++11
    #include <cstddef>
    #define DD_MOVE(expr) expr
    #define DD_NULL       NULL
#endif // DEBUG_DRAW_CXX11_SUPPORTED

//
// These are internal and only required for the glyph bitmap texture setup,
// but the user can still override and provided custom allocators if needed.
//
#ifndef DD_MALLOC
    #include <cstdlib>
    #define DD_MALLOC std::malloc
    #define DD_MFREE  std::free
#endif // DD_MALLOC

//
// Optional math.h replacements if you want to avoid the dependency:
//
#if DEBUG_DRAW_USE_STD_MATH
    #include <math.h>
    #include <float.h>
    #ifdef FLT_EPSILON
        #define DD_EPSILON FLT_EPSILON
    #else // !FLT_EPSILON
        #define DD_EPSILON 1e-14
    #endif // FLT_EPSILON
    #ifdef M_PI
        #define DD_PI M_PI
    #else // !M_PI
        #define DD_PI 3.1415926535897931f
    #endif // M_PI
    // NOTE: Using the *f suffix ones because some platforms might not support
    // double precision (e.g. PS2), where these are the only ones available.
    #define DD_FABS(x)       fabsf(x)
    #define DD_FSIN(radians) sinf(radians)
    #define DD_FCOS(radians) cosf(radians)
    #define DD_INV_FSQRT(x)  (1.0f / sqrtf(x))
#else // !DEBUG_DRAW_USE_STD_MATH
    #define DD_EPSILON       1e-14
    #define DD_PI            3.1415926535897931f
    #define DD_FABS(x)       fastFabs(x)
    #define DD_FSIN(radians) fastSin(radians)
    #define DD_FCOS(radians) fastCos(radians)
    #define DD_INV_FSQRT(x)  fastInvSqrt(x)
#endif // DEBUG_DRAW_USE_STD_MATH

//
// Misc helpers:
//
#define DD_TAU              (DD_PI * 2.0f)
#define DD_DEG2RAD(degrees) (static_cast<float>(degrees) * DD_PI / 180.0f)
#define DD_ARRAY_LEN(arr)   (static_cast<int>(sizeof(arr) / sizeof((arr)[0])))
#define DD_CHECK_INIT       if (g_renderInterface == DD_NULL) { return; }

namespace dd
{

// Implementation internals will be members of the unnamed namespace.
namespace
{

// ========================================================
// Embedded bitmap font for debug text rendering:
// ========================================================

typedef unsigned char UByte;

struct FontChar
{
    unsigned short x;
    unsigned short y;
};

struct FontCharSet
{
    enum { MaxChars = 256 };
    const UByte * bitmap;
    int bitmapWidth;
    int bitmapHeight;
    int bitmapColorChannels;
    int bitmapDecompressSize;
    int charBaseHeight;
    int charWidth;
    int charHeight;
    int charCount;
    FontChar chars[MaxChars];
};

#if DEBUG_DRAW_CXX11_SUPPORTED
    #define DD_ALIGNED_BUFFER(name) alignas(16) static const UByte name[]
#else // !C++11
    #if defined(__GNUC__) // Clang & GCC
        #define DD_ALIGNED_BUFFER(name) static const UByte name[] __attribute__((aligned(16)))
    #elif defined(_MSC_VER) // Visual Studio
        #define DD_ALIGNED_BUFFER(name) __declspec(align(16)) static const UByte name[]
    #else // Unknown compiler
        #define DD_ALIGNED_BUFFER(name) static const UByte name[] /* hope for the best! */
    #endif // Compiler id
#endif // DEBUG_DRAW_CXX11_SUPPORTED

//
// Data generated from font 'Monoid18' by font-tool.
// Command line: monoid-18.fnt monoid-18.png monoid-18.h Monoid18 --static --compress --structs --hex --encoding=lzw
//
// The Monoid font, copyright (c) 2015 Andreas Larsen and contributors,
// is released under the MIT license. See: https://github.com/larsenwork/monoid
//
// The following glyph bitmap is an LZW compressed graymap.
// ~7.55 KB of data.
//
// It is better to ensure it is aligned to a, say 16 bytes boundary,
// because we cast the first few bytes to uint32s.
//
// font-tool: https://github.com/glampert/font-tool
// LZW compression: https://github.com/glampert/compression-algorithms
//
DD_ALIGNED_BUFFER(g_fontMonoid18Bitmap) =
"\x2F\x1E\x00\x00\x78\xF1\x00\x00\x00\x00\x06\x14\x38\x90\x60\x41\x83\x07\x11\x26\x54\xB8"
"\x90\x61\x43\x87\x0F\x21\x46\x94\x38\x91\x62\x45\x8B\x17\x31\x66\xD4\x88\x70\x43\x13\x8C"
"\x15\x80\x0D\xD8\x98\x10\x0D\x98\x84\x4D\x36\x8C\x8C\x28\x40\xC1\x01\x95\x2F\x1F\x06\x10"
"\x10\x00\x26\xC0\x44\xFE\x5C\x5A\xA4\xF4\x8F\x4E\xCD\x81\x01\xF8\xFD\x4B\x70\x30\x40\x3F"
"\x3F\x3E\x17\x32\xFA\xB7\xB4\x5F\xCA\x8C\x67\xFC\xD1\x04\xB0\x2F\x0D\x52\x82\x74\x96\xF2"
"\x54\xD9\xC4\x5D\xD6\xA5\xBF\x10\x36\x5B\xBA\xE3\x60\x81\x7E\xFD\xA4\x2E\x3C\xC0\xE8\xDB"
"\xD2\x77\xC1\x2A\x34\xF4\xF2\xEF\x52\x42\x7E\x3D\xAD\x1E\x74\xB5\x34\x13\xB4\xA5\x11\x9E"
"\x46\x05\x78\x37\x6F\xC0\x25\x4B\xDB\xFE\xAB\xB2\x51\x0E\xE2\x7E\xFF\xDE\xF9\x73\xA6\xF7"
"\x9F\xBF\xA5\x14\x0E\xEE\xFC\xC2\xD0\x44\x56\x7F\xEF\xB2\xB2\x60\xA8\xCF\xDF\x50\x84\x84"
"\x0B\x13\x8C\xF0\xAF\x5F\xCE\x2F\xFF\xC0\x62\x84\x4A\x13\x28\xDE\xC2\xD8\xFC\xC5\xA5\xE0"
"\xAF\x5B\x40\x1A\x16\x0F\xF8\xF3\x67\x02\x40\x17\x7F\x09\x4B\xB0\x06\x30\xE4\x5F\x6F\x83"
"\xC1\xFB\x31\xDC\xF7\x4F\x4D\x5A\x2B\xFF\xD8\x2D\x9C\xF2\x8F\x94\x42\xD4\xA9\x05\x7E\xF8"
"\x57\x35\xE0\x3E\x6E\x19\x8D\x07\xE4\x47\xBE\xF0\xB2\xE3\x83\x9D\x2B\x1A\x5F\x91\xF9\x66"
"\x00\xB3\x11\x66\xFB\x07\x02\xA0\xB2\x7F\xA2\x0D\x12\xE5\x1F\x26\x14\x3A\xAC\xBB\x81\x70"
"\x4B\xEB\xA0\x7C\xFE\x01\xCC\x2E\xDB\x16\x6A\x86\xC0\x8C\xC4\x3B\x10\x80\x01\xFA\x71\xCE"
"\xA2\x59\xDA\x9A\xC5\xC3\x7F\xBE\x01\x46\x23\x09\x11\x4A\xE4\x1F\x6F\x9A\xE8\x86\x2E\x00"
"\xE4\x6B\xAD\xA2\xC6\x34\x00\x08\x3F\x83\x34\xF8\x67\x32\x80\x6A\xBC\xB1\x20\x08\xFE\xE1"
"\x47\x21\x34\xFE\xF9\x00\xA0\x06\xAC\x00\xC3\x0A\x67\x04\x43\x88\xB9\xD8\x1E\x74\xE8\x3B"
"\x8C\xA6\xC3\x0C\x00\x66\xB4\xB2\x28\x00\xCB\xBC\x5A\x2A\xC9\x81\x5A\x3A\xC0\xCB\x03\x14"
"\x10\xC0\x3B\x08\x0B\xDA\x26\xAB\x76\x00\x30\xB1\x1F\x91\x2A\x92\x61\xC5\xFB\xB6\x24\x48"
"\x97\x7F\x66\x10\xE8\x9E\x06\x0F\xDA\xE5\x1F\xB2\x10\x82\xCA\x00\x00\xD2\xC8\x32\xCE\x82"
"\xD2\xF9\x27\xC6\x31\x9D\x24\xD3\xA2\xC3\xFC\x51\xE3\xB1\xE8\x30\x12\xC0\x0F\x7F\x1A\x20"
"\xA0\x01\x7E\xFE\x10\x93\xA0\x2B\xB3\xFC\xE7\xA8\x26\x13\x32\x81\x08\xE2\xD4\xFC\xD3\x22"
"\x0C\xFF\xA9\x03\x00\x34\xDE\x7B\xCE\xC1\x80\x0E\x78\xB5\xA0\x0C\xFE\x41\xB3\xCF\xE3\x3C"
"\xA8\xAC\x89\x06\x14\xF8\x63\xD0\x81\xDC\xD4\x11\xD4\x83\x36\x88\x6B\x30\xF2\x04\xF8\x8D"
"\xA0\x06\x32\x99\x45\xD5\x80\x76\x90\xC5\xA0\x40\xB3\x7A\x46\x41\x8B\x66\x7C\x72\x20\x60"
"\x3C\xEC\x36\x98\x09\x4F\x53\xF4\x20\x4B\x58\x33\xF5\xA2\x1D\xBC\xF2\xC7\x8A\x8D\x96\xF9"
"\xEF\x56\x00\xCC\xD8\x33\x20\x57\x7E\x15\x48\x3F\xFE\x16\xD2\x96\x20\x7B\x2A\xAB\x03\x84"
"\x7E\x30\x31\x21\x98\xA5\x00\x0C\x08\xA8\xAC\x20\x05\xA0\x92\x5F\x47\xF1\x0A\x2F\x01\x08"
"\x90\x2D\x49\x7D\x21\x92\x49\x00\x01\x0E\xC0\x14\x80\x8B\x35\x35\x88\x5C\x17\x0D\xB2\x98"
"\xE3\x91\x3B\x26\x08\x24\xAF\xFA\x31\xED\xA2\xE4\x34\x2C\x08\x2A\x78\xF7\x0B\x28\x9B\x7A"
"\x01\x62\xB9\x21\x8A\x05\x02\x23\xB1\x4E\x4D\x71\x2A\xA0\x19\xFE\x29\x25\x02\x75\x60\x03"
"\x68\xE6\x6B\x19\xAD\x80\x07\xD6\x44\x92\x83\xE6\x87\xD2\x3B\xD6\xA2\xC6\x3A\xD5\xF2\xDA"
"\x80\x1A\x5E\x13\x21\xAA\xAB\x7E\x7A\xAA\x45\x2A\x50\xD1\xC7\x8C\xB4\x89\xD9\xA0\x97\xE3"
"\xAD\x13\x00\x1E\x9F\x6E\xB7\x60\x44\x15\x52\x00\x0C\x84\xF1\x25\xA8\x31\x21\x03\x98\x6E"
"\x16\x40\xB0\x23\xA8\xEC\x12\x00\x12\x70\xB2\x19\x29\x6A\x62\x16\x7F\xC0\x68\x02\x0C\x7F"
"\x80\xF1\x88\x22\x2A\xBE\x81\x46\x72\xC8\x24\x87\xC6\x9B\xAB\x01\xC8\x5A\x01\xC3\x67\x31"
"\x89\x20\xC8\x2B\x0F\xDD\x72\xCC\xD5\x23\xAF\x6C\x21\x31\x0A\xF6\xA0\x97\x55\xA8\x8C\x07"
"\x1E\x82\xA2\x99\x56\x5B\x19\xC2\x99\xA0\xE4\x96\x6A\x79\x20\xA7\xD9\x8C\x00\x4B\x79\x05"
"\x02\x2A\xBB\x80\xF4\xF9\x87\x08\x2A\xBC\x5E\x28\x80\xAA\x97\x22\x3D\xA2\xF5\x12\x6A\x18"
"\xA7\x4A\xBC\x8A\x45\x23\xD4\x56\x98\x2F\xA3\x42\x8D\x25\xE8\x8C\x7F\x20\x00\x60\xAF\xA5"
"\xDC\x09\x86\x66\x3D\xF9\xAC\x5D\x5C\x82\x50\x25\x42\x2C\x45\x0C\x8A\xA0\x85\xE0\x79\x20"
"\xA2\x81\x4D\xF9\x39\x2F\xA0\x1A\xAD\xB6\xA8\x85\x52\xFC\xC1\x83\x16\xF0\xA0\x1F\xA5\x98"
"\x5F\x41\x82\x92\xA5\xF5\x19\xCC\x76\x99\xAB\x4C\x02\x14\xC0\x9A\x0A\x54\x80\x1D\x86\xCA"
"\x08\x6A\x68\xB5\xC0\x88\xA0\xCB\x17\x06\x69\x0C\xE0\x00\xC0\x03\x40\xA8\x01\x00\x0D\x50"
"\x16\x41\x78\x34\xB6\x9B\x69\x30\x20\xED\xEA\x4E\x70\xFE\x41\x9C\x8A\xE8\xE7\x50\x00\x01"
"\x12\x6B\x9C\xE7\x90\xA8\x01\x00\x7A\x06\xD1\xC7\x3B\x80\x08\x44\x7F\xB0\xE7\x20\x0D\xDC"
"\x07\xA5\x00\x50\x23\x34\x00\xA4\x0B\x66\xB3\x12\x6A\x4C\xF4\xB6\x8B\x30\x48\x56\xD0\xE2"
"\x8E\x43\x04\x04\x2E\xF5\x29\x84\x02\xCA\x01\x08\x73\x84\x25\x11\x2A\x54\x46\x86\x00\x28"
"\x40\xEE\x24\x26\x95\x06\x56\xCC\x28\x08\x69\x80\x83\xC4\x53\x8A\xC0\xFD\x03\x84\x14\x51"
"\x63\x55\xAE\xE3\x0F\x73\x61\x64\x3B\x16\x1A\xC8\x74\x00\x51\xB2\x92\x09\x04\x3A\x10\x59"
"\x23\x40\x22\x20\xAB\x13\x56\xE4\x15\xE4\x6B\x96\x57\x0E\x58\x91\x6C\xB1\x10\x22\x28\x69"
"\x48\xF1\x26\x97\xB0\x89\xD4\xC8\x1F\x6D\xF9\xCC\x52\x86\xA0\x12\xD2\xA8\x4C\x20\x2D\xF0"
"\x0C\x68\x96\x82\xBA\x81\x68\xC6\x90\x94\xF4\x09\x0F\xBA\x92\x15\x40\x5C\x47\x93\x00\x00"
"\x81\x15\x9A\x90\x4B\x5D\xE6\x72\x5D\x67\x4B\x12\x55\xC0\x33\x90\x06\x54\xF0\x1F\xEE\xA8"
"\x62\x44\x28\xF0\x18\xAF\x78\xC3\x67\x1B\x99\xCB\x22\x0C\xA2\x80\x4C\x24\xE6\x33\xDF\xE8"
"\x5E\x40\xCC\x82\x96\x56\x06\xB3\x20\x04\xF0\x12\x42\xF6\xC0\xBC\xE6\x71\x53\x22\xDF\xD4"
"\x88\x02\xFC\x51\x97\x9A\x1C\x8C\x94\x10\xD9\x89\x2B\x09\x72\x48\x72\xAA\xA6\x05\x26\xB0"
"\xE7\x3D\xED\x19\xC9\x79\xEE\x13\x20\x96\xF4\xC9\x17\x3C\x37\x91\x0A\xCC\x82\x4D\x0F\x91"
"\x27\x3F\x11\x9A\x50\x85\xF2\x13\x98\x0B\x75\xE8\x43\x21\x1A\x51\x89\x4E\x94\xA2\x15\xB5"
"\xE8\x45\x31\x9A\x51\x8D\x6E\x94\xA3\x1D\xF5\xE8\x47\x41\x1A\x52\x91\x8E\x94\xA4\x25\x35"
"\xE9\x49\x51\x9A\x52\x95\xAE\x14\xA1\x8C\x00\xC3\xFD\x20\x42\x03\x46\xE8\x73\x20\x44\xB0"
"\xE9\x4D\x15\xB2\x81\x9B\xDA\xB4\x99\x03\xA1\x41\xDD\x20\x12\x50\x96\xA2\x94\x53\xFF\x80"
"\x66\x43\x2A\xA0\x4C\xF8\x15\x64\x79\xE9\xCA\x21\x00\xB8\xB6\x94\xA5\x16\x64\x3A\x47\x75"
"\x48\x04\x82\x42\xC4\xA1\x1E\x04\x62\x83\xAC\x68\x04\x16\x07\xBC\x85\x60\xE3\x1F\xC0\x30"
"\x41\x4E\x0A\x42\x03\xB5\xFE\x2F\x79\x50\x21\x82\x5A\x69\x70\xCD\x81\xF0\xC0\x32\x98\x70"
"\x08\x03\x1E\x03\x0C\xB4\x9E\xB4\xA1\x08\x69\x58\xB5\x8E\x39\x10\xF7\x30\x40\xA1\x6E\x82"
"\x27\x50\x54\xB8\x10\xE3\x3C\xF5\x65\x0C\x61\x40\x50\x16\xC3\x10\x06\x59\x35\xA5\x87\xC4"
"\x0D\x34\x4C\xA1\x4C\x98\x1A\x64\x8C\xEA\x44\x28\x86\xE0\xC9\xC3\xA9\x2E\x84\x70\xBE\x7C"
"\xAA\x40\xBA\xE8\x0F\xC2\x2A\x84\x7A\xD6\x63\xE9\x21\x4B\xA6\x86\x37\x1D\xE4\x88\x7B\x3C"
"\xC8\x25\x42\x3B\x90\x04\xB0\x03\xA8\x06\x39\x28\x0F\xB5\x9A\x90\xD2\xBA\x2C\x79\x03\xE9"
"\xAC\x42\x0A\xE0\x8F\x5A\x56\x36\xB7\x00\x00\x8A\xEE\xEC\xF6\x8F\xD1\x1A\x64\x00\xD3\xB1"
"\xEB\x43\x22\xF0\x98\xE0\xC6\x73\xBB\x06\xD9\x47\x6E\x87\xEB\xBD\xE2\xFE\x71\xB9\x9F\xD3"
"\x9E\x42\x86\x10\x8C\x6F\x2D\x24\xBD\xEB\x45\xE8\x6F\x01\x62\x16\xE8\x0E\x64\x00\xB3\x60"
"\x84\xF2\xD6\xF1\x0F\x69\x35\x04\x04\x96\x89\xAC\x5D\x98\x94\x10\x1E\x81\xB7\xAD\xE3\x15"
"\x88\x80\x7A\x4B\x90\x76\x05\xD6\x65\x4B\x71\xE5\xF7\xAA\xC4\x4F\xF8\x02\x60\x27\xDD\x9D"
"\x48\xA1\xC2\x78\x10\x53\x0E\x48\x79\xFC\xF0\x07\x83\x07\x22\x9F\x4F\x31\x24\xBC\x03\x29"
"\x71\x9F\x9C\x68\x10\x65\x18\x98\x6A\xAE\x6C\x71\x42\xE1\x8B\x2E\x7F\x14\x94\x6C\xB5\x7A"
"\x2A\x11\xC6\xD2\x10\x13\xFD\x23\x18\x8B\x1C\x08\x23\xBA\x82\xC4\x86\x9C\x38\x20\x44\x36"
"\x88\x0E\x52\x5C\x10\x65\x24\x36\x21\x2D\xA8\x43\x1D\x7C\x6C\x10\x27\x43\x39\xA1\xDC\xF8"
"\xAF\x42\xC4\x23\xD6\x8D\x50\x09\x64\x05\x79\x0D\x9D\x1C\x32\x80\xAC\x4C\xD7\x60\x58\xF2"
"\x23\x69\x0B\x7C\xDA\x81\xD0\x2A\xC1\x03\x51\x86\xFE\xB6\xEA\x10\x74\x44\x58\x25\xBD\xF0"
"\x54\xC8\x1E\xD3\x66\x84\x14\xCA\x1B\x34\xD8\xEB\x40\x36\x50\x87\x1C\x0F\x39\xCD\x0F\x19"
"\xA3\x9E\x03\xA2\x8C\xF2\xC6\x99\x28\xFC\x60\xF2\x46\x0A\x50\xC1\x05\x06\x85\x84\x0D\x41"
"\x2C\x43\x82\x40\x67\x85\x18\x19\x4E\x6A\x16\xC8\xF7\x10\x0D\x10\xF7\x78\x9A\xD1\x00\xD0"
"\x87\x85\x2D\xF2\xD8\xA2\x1D\x04\x04\x4B\xA1\xAC\x77\x3C\x9B\x90\xE4\x12\x98\xB1\x06\x4E"
"\xF4\x3F\xE4\x4A\x10\x3D\xA9\xB2\xD4\xF3\xCC\xEE\x3F\x5C\x8B\x10\x0D\xD4\xF5\x66\xA8\x1E"
"\x08\x7C\x39\xCD\x69\xFA\xF6\xE3\xD1\x04\x41\x32\x99\x2F\xE2\x05\x20\xDA\x27\x21\xD2\x7E"
"\x07\xB5\x53\x3A\xEC\x7F\x5C\x57\x21\xBF\x0E\xF6\xB0\xBC\x63\x6C\x13\x17\xBA\x21\x8D\x81"
"\xF6\xB2\x67\xBC\x10\x05\x80\x00\x04\x9B\x45\xB1\xA6\xCF\xE6\x60\x75\xB3\xDB\xDD\x19\x31"
"\x01\x23\x00\x31\x92\x56\x1B\xF5\xAE\x8F\x09\x30\x75\xFB\x11\x5A\x7E\x9C\xDB\xB4\xC2\xB5"
"\x75\x6A\x57\x9B\x10\xEA\xFD\xDB\x83\x4B\x19\xF1\xD6\xE4\xAD\x90\x17\x4B\xDC\xE1\x2A\x99"
"\xCE\x79\x2F\xC2\x80\xA5\xFC\x01\x22\x09\x08\x4A\x07\x11\x62\x58\x86\x00\x85\x76\x8A\x2D"
"\x70\x43\x32\xF6\x0F\x6C\x27\xA4\x78\x1C\x17\x6E\xC4\x13\xB2\x01\x2B\xF8\xC3\x95\x32\xA7"
"\x39\x69\x61\x9E\x91\x83\xDD\x59\x23\x7D\x68\x2E\x36\xD9\x51\x6F\x81\x34\xA0\x09\x8F\xA9"
"\xA3\x42\x0A\x35\x0B\x10\x44\xCC\x20\x34\x68\x41\x3D\x01\x58\x6B\x01\xB6\x20\xAE\x1A\x7E"
"\x8C\x1C\x1B\xE2\xBB\xA0\x31\x3D\xDE\x18\x07\xF7\xD7\xBB\x2E\xEE\x88\x34\x32\xC9\x13\x2D"
"\x2A\xD6\x19\xF2\x6B\xE9\x86\x4C\x50\x4F\x8D\xEA\xDA\x7D\xBB\x6D\x88\x24\xD5\xEB\x15\x01"
"\x1A\x3C\xEF\x9E\xD0\x03\x90\x3A\xA1\x8C\x50\x03\x88\x11\xC2\x83\x4C\x44\x39\x20\x3B\xB5"
"\x69\x4E\x0D\xDF\x53\x81\xF0\xA0\x8C\x10\xA9\x4E\x46\x98\xE3\x2E\xF4\x12\xAC\xD7\x95\xA7"
"\x68\x63\x2A\x4D\xF1\xCC\x5B\x9E\xF3\x0F\xF5\x2A\x42\x3E\xDF\x79\xD1\x8F\x9E\xF4\xA5\x37"
"\xFD\xE9\x51\x9F\x7A\xD5\xAF\x9E\xF5\xAD\x77\xFD\xEB\x61\x1F\x7B\xD9\xCF\x9E\xF6\xB5\x8F"
"\x08\x01\xB8\x9E\x10\x8E\x35\x04\xF7\x2F\xC9\xFD\x45\x03\x80\xFB\xD0\x97\xD4\x77\xDF\x5E"
"\xDD\x2F\x51\x8D\xE3\xA5\x3C\x83\xC6\x05\x29\x45\x56\xF4\x9A\x90\xB9\x2D\xC5\x1B\x42\xBF"
"\x75\xD5\x78\x5D\x10\xDC\x00\x7E\x2A\x9D\x42\x35\xAB\x4C\xD3\x57\x82\x5C\xDC\x2D\x75\x27"
"\xDE\x32\x73\x5D\x90\x06\x74\x43\x9B\x19\x15\x0F\x9C\x6F\xA5\xC6\x05\xF2\x2F\x32\xFF\x80"
"\x3F\x41\xFC\xE3\x0F\x68\x58\xA6\xD9\x01\xB9\x21\x34\x40\x83\x66\x7A\xE1\x33\x82\x08\x95"
"\x42\x8D\x05\x6A\xC5\x3B\x20\x23\x88\xCC\xAF\x85\x26\x66\x81\x7E\x08\x88\x12\xE3\xE1\x8E"
"\xED\x1B\x82\x61\x16\x02\xB0\x9D\x08\xE2\x3A\x2A\x83\xEF\x10\xAA\x46\xEE\xAF\xE0\x78\x68"
"\x81\xF4\x64\x31\x02\x80\x68\xD2\x0F\x00\xBA\xA8\xFD\xF4\x43\x6D\xA8\x2A\x37\x00\x62\x11"
"\x1A\x50\xFB\xD2\xED\x20\x0A\x25\xFB\x10\x28\xB7\xD0\x41\x85\xE4\x89\x01\x84\x83\xFB\x02"
"\x42\x3E\x62\xED\x27\x7E\xA1\x32\xBE\xC1\xD6\x12\x0A\x04\xD1\x4C\xFE\x68\x4B\x85\xF4\xA4"
"\xBB\x80\xE6\xBF\x92\x63\x7D\x6A\xC4\xB3\x92\x2B\x04\x0D\x22\x01\x80\x2D\xE4\x56\x0D\xEC"
"\x4E\x03\xCE\xE4\x49\x3F\xB4\xC8\x3B\xE6\x0B\x20\xA6\x43\x5A\x16\x4B\xA3\x94\x70\xD3\x1E"
"\xB0\x88\xD4\xA9\x8B\xE0\x0D\x20\x60\x24\x20\x40\xCB\x20\xC4\x83\x88\xCC\x83\xC4\x24\xCF"
"\x20\xE6\x4C\x4A\xBC\x30\x7E\x78\x6E\x04\x15\x62\x8C\x18\x0E\x21\x82\x03\x0B\x01\x42\x0D"
"\x94\xC5\xC8\x16\xA1\x5B\x3C\x04\x1A\x52\x50\x0D\x98\x4F\x20\xAA\xE0\x19\x34\x10\x00\x62"
"\x21\x18\xF4\xA9\x14\xA0\xC1\x05\x69\xC4\xFE\x96\x70\x30\x26\x8D\x3C\xC8\x6E\x7D\x40\x4D"
"\x20\xBE\xCB\x20\x7C\x90\x49\x78\xA4\x10\x09\x65\xD1\x70\x84\x35\x4C\x80\x06\xB8\x8F\x1F"
"\x36\x63\x80\x14\x6F\x20\x6A\x84\x3D\xD6\x28\x01\x94\xEB\x21\xFA\x20\x0E\xC5\x2B\x87\xCE"
"\xA1\x53\x8E\x4E\x20\xF8\x00\xCC\x02\x02\x1D\x6A\x70\x20\x80\x24\x61\x98\x43\x6B\x84\xCD"
"\x46\x42\x51\x10\x7D\xAB\x27\x78\x04\x10\xC4\x4F\x20\x80\xA4\x6E\x56\x63\x81\x8E\xE8\x80"
"\x60\xE1\x1F\x42\x89\x8B\x66\xAB\x20\xDE\x6E\xBF\xBC\xEB\x1B\x94\x89\xDF\x0E\x62\x2E\xEA"
"\x66\x8D\xF4\xA3\x1D\x88\xE0\xF7\x0E\x22\x38\xF4\xA8\x1A\x0D\xC2\x9B\x0E\xC0\x52\xCE\x82"
"\x74\x92\x63\xAA\x30\xA4\x0C\x01\xA0\x6C\xC8\x63\x3A\x3C\xF1\x13\x11\x71\x20\x98\xE0\x72"
"\x00\xC2\xCA\x0C\xA2\x8B\x7A\x42\x14\xFA\x41\x01\xF4\x21\xC3\x00\xE2\x30\x6C\x43\xE4\x0C"
"\x02\x01\xFF\x61\x16\xBA\x82\x20\x07\x02\x11\x82\x04\x21\xDA\xA5\x51\x04\x4D\x31\xF0\xAC"
"\x32\xAC\x80\x6F\xB4\x6C\x20\x92\xF1\x55\xD6\x08\x1F\x3C\xC3\xB6\x0C\x62\x2F\x28\xF0\xF8"
"\x3C\xAD\x31\xC6\x30\x78\xF8\x81\x76\x02\xF2\x20\x34\x2E\x2A\x1A\xC3\xF8\x42\x66\xC2\x12"
"\x42\x3C\xD0\xE0\x8C\x8E\x62\x1F\xCA\x90\x47\xFA\xE1\x7E\x08\x80\x68\xAE\x6C\x20\x78\xA4"
"\x7A\x46\x03\x16\xCD\xF0\xC3\x6A\xA6\x39\x0E\xA2\x0E\x80\x41\x2A\x96\xE4\x20\x46\x4D\x3D"
"\x58\x08\x63\x1A\x80\x1B\xE0\x6E\x1A\xB5\x72\xDC\xD4\x2C\x38\x86\x87\x2C\x4B\x03\x20\x92"
"\x51\x8A\x4C\x0C\x32\x70\x68\x8B\x32\xC2\x07\xC1\x60\x52\xFE\x84\x1F\xCE\x4C\x20\xEC\x0C"
"\x44\xB2\xC2\x0F\x09\x82\x41\x00\xE1\x00\x36\x60\x3A\x56\xAE\x20\xB2\x27\x27\xFF\xA8\x65"
"\xF2\xC1\xC0\xF6\xA1\xFF\x00\x00\x1F\x5A\xA6\x87\x90\x8B\x37\x14\x42\x0C\x09\x4D\xCD\xF6"
"\x22\xD4\x00\x60\x2E\x22\x4B\x1B\xF2\x11\x21\xA8\x44\x19\x17\x42\x1F\x1A\xB3\x22\xF6\x21"
"\x18\xFA\xA1\x3B\x80\x02\xD5\x32\x01\x65\x0E\x42\x3C\x58\xD1\x2B\x13\x62\x2F\x6A\x31\xD6"
"\x56\xCC\xD3\x7E\x93\xA9\x28\x0D\x0C\xC0\xA0\x0E\xFC\x21\x18\x84\xCA\x24\xBD\x06\x68\x4A"
"\x8E\x0D\x4F\x4B\x3C\x8C\x92\x20\x34\x6E\x32\xCC\x22\x2E\x0F\xA2\x31\x9A\x91\x9B\xA6\x83"
"\x2B\xAD\x31\x21\x98\x63\x08\x3F\x8D\x8E\xEC\xA5\x2D\x85\x27\xDC\x04\xA2\x2C\x17\x22\x38"
"\x37\xA5\x1D\xB3\x44\xCD\xEE\xC1\x6B\x18\xA4\xF1\x9C\x13\xE9\xE6\x52\x21\x98\x71\x00\x62"
"\x20\x10\x55\xD1\xCC\xF6\x29\x28\x62\x43\x22\x17\x62\xCE\x52\xF0\x3E\x2C\x28\xD1\xBC\xA6"
"\x3E\xBC\xC3\x8F\xD4\x53\x21\xD0\x93\x20\x4C\x00\x04\xEC\x09\x04\x5A\xA0\x80\x44\xD3\x01"
"\x9D\xA7\x89\x30\x32\xFE\x12\x42\x23\x19\x62\x3B\xC0\x40\x06\x71\x50\xB0\x8C\xE7\x6F\x16"
"\xE2\xDE\xF2\x4D\x23\xA2\x04\x20\x90\x4C\xDC\x6C\xE6\x20\x0E\xE3\xCA\xF2\xA1\x2D\xDD\xE3"
"\x12\x7D\xD2\x3A\x9D\x72\xE4\xF8\xC1\x3A\xED\x42\xEC\x06\xA3\x39\x7D\x12\x4F\x1C\xC2\xC8"
"\xF2\xA6\x2D\x7F\xD4\x8B\x0E\x82\x39\xB2\x23\x7B\x2C\x13\x15\x97\x82\x47\x19\x42\x4F\xEC"
"\x83\x41\x06\x54\x66\xF6\xB0\x20\x34\x2E\x65\x00\x02\x2B\xA2\x13\x56\x24\x63\x21\xF4\xE3"
"\x80\x98\x00\x14\x43\x46\x20\xFC\xE0\x1C\xFF\xD0\xB9\x04\x02\x2B\x02\x33\x20\x48\x12\x3C"
"\xE7\xF3\x20\x4C\x24\x7D\x16\x22\x18\xAF\x88\x28\xA6\x03\x84\xDA\x85\x85\x76\x0E\x32\x27"
"\x22\xCB\x66\xE1\x31\x16\x32\x20\x90\x4C\x47\x0B\x42\x40\xFE\xC1\x14\x54\x04\x48\xE3\xAD"
"\x27\x0B\x02\x5D\x78\xCC\x2F\xCA\x4E\x3D\xDE\xC2\x14\x62\xC9\xFA\xAE\xB1\x88\x26\x35\x96"
"\x0C\xF3\xD8\x10\x23\x88\xFC\xA1\x86\x74\xF2\x34\x2A\xC3\x53\xC5\x6D\x3A\xEC\x52\x20\xA8"
"\x47\x47\x38\xC9\x52\x03\x82\xEC\x2A\xF4\x21\x9A\xA0\x5A\x9A\xAF\x20\xC8\xE5\x18\x0D\xC2"
"\x25\x8B\x29\x17\xCF\x33\x3B\x11\xA2\x09\xB0\x24\x80\x10\xA2\x78\xB2\xC2\x1D\xA6\x74\xFC"
"\x2C\x4C\x58\xC9\x47\x57\xCD\xB0\x6A\x40\x55\x04\xBD\xAB\x53\x28\x49\x1F\xB4\xD2\x07\xFF"
"\xC1\x30\xF7\x62\x4D\x07\x62\xEF\x5E\xA2\x25\x1A\x82\x55\x09\x02\x63\xFE\x2C\x23\xCC\x29"
"\x21\x82\x6F\x5C\x75\x8E\x1F\x37\xAA\x4C\xDD\xD4\xF6\xD8\xF5\x21\x64\xE1\x31\xEA\xB3\x5D"
"\xE5\x15\x22\x1E\xE3\x19\x8A\x75\x5E\xF1\x95\x20\x0E\x60\xF8\xF2\xB5\x5F\xFD\xF5\x5F\x01"
"\x36\x60\x05\x76\x60\x09\xB6\x60\x0D\xF6\x60\x11\x36\x61\x15\x76\x61\x19\xB6\x61\x01\x62"
"\x26\x18\x82\x5F\x1D\x76\xA2\xB6\x91\xAA\x9C\x54\x50\x2B\xC3\x5B\x01\xA2\x50\xD6\x75\xA2"
"\x74\xAA\x61\x0F\x09\x29\xA9\xC6\x56\x7F\xE2\x31\x08\xB5\xA2\x9C\xC6\x03\xF1\x35\x64\x7F"
"\xAE\xBF\xDA\x61\x56\x09\x82\x14\x54\x6E\xA3\x76\x48\x61\x59\x16\xA1\x14\x41\x1D\x05\x82"
"\x06\xAC\x89\x20\x24\x91\xC6\x2A\x91\x46\x49\x24\x20\x86\x76\x20\xD2\x8B\x43\x3E\xD2\x43"
"\xDC\xCB\x20\xD4\xE0\x1B\xDE\xE1\x1B\x22\x87\x51\x01\xA2\x19\x42\x09\x10\x9C\xF6\x1D\xAA"
"\xB2\xA6\x9C\x56\x16\x0E\xE0\x17\xA4\x16\x00\x9A\x81\xF1\xDC\xE1\x1D\xB8\x36\x54\x9E\x01"
"\x1A\xBE\xE1\x44\x0F\xA2\x19\x6C\x0A\x6A\x4D\x01\x66\x97\x03\x39\xA1\x36\xAE\xBA\xE1\x1D"
"\xC2\xF2\x4C\xCD\x32\x20\x12\x20\x18\xA0\x01\xC4\x8E\x76\x16\x44\x04\x21\xB8\x65\x16\x96"
"\xD6\x20\x1A\x34\x20\x1A\x23\xC1\x82\xF1\xE8\x70\xC3\x26\x45\x51\x20\x28\x26\x9C\xAA\x86"
"\x4F\x01\xA0\x50\xB2\x84\x72\xF9\x01\x51\x1D\x66\x75\xA0\xB5\x88\xDA\xEE\x48\xB3\x64\x49"
"\x13\x08\x61\xBA\xAE\x6A\x32\x34\x6F\x41\xA5\xBA\xDC\xB1\xE1\xFA\x67\x25\x73\x4E\xC9\x7E"
"\xE5\x14\x07\x02\x68\xF0\xD0\x47\x51\xF7\x71\x09\xC2\x52\x0E\x40\x11\xFC\x61\x03\x1A\xE0"
"\x77\xED\x11\x20\xBA\xA8\xFA\x00\x22\x00\xB4\xF5\x59\xA9\x83\x36\x1A\xCD\x78\x00\xA0\x33"
"\x84\xB1\xFB\xC0\x82\x00\xCC\x04\x1C\xFF\xC1\x23\x08\x40\x2D\x2D\x6C\x3A\xD6\xE5\x00\xD4"
"\xB2\x1C\xEF\xB2\x27\x02\xA5\x35\x0A\xE0\x88\x72\xE8\x66\x03\x80\x41\xB0\x35\x20\x30\x46"
"\x52\xBC\xA6\x1F\x2F\x85\x85\x82\xF3\x00\x4C\xC1\x73\x64\x57\x20\x7C\x50\x47\xBA\x28\x7D"
"\x9F\xA4\x81\x1A\x2B\x21\x6A\x64\x0E\x1A\x62\x3A\xC4\x0E\x3A\xFD\xEF\x79\xC9\x17\x20\x34"
"\x0E\x11\x6B\xC4\x42\x0E\x91\xB6\x34\x44\x3C\xC4\xC5\x4F\xCC\xA8\x1F\x8E\x0A\x11\xBC\x26"
"\x64\x97\x88\x68\x5C\x91\xB8\xD4\x6C\x8D\x82\x13\xC9\x20\xE5\x7B\x42\x34\x46\x03\x62\x3B"
"\xBE\x76\x7F\x35\x48\xD9\x6A\xC4\x1D\x36\x40\xCD\xF6\x81\x48\x01\x62\x2E\x0A\x06\x40\xBD"
"\x4B\x58\xB2\xA1\xFD\xEE\x52\x4E\xD3\x81\x66\xBE\xA3\x36\x38\x58\x6A\x3A\x2D\x5C\xBC\x83"
"\x11\x8A\xF0\x74\x7F\x18\x6E\x8E\xEF\x4F\x42\xA1\x32\x08\xAB\x0F\xEA\x65\x2F\xA4\x84\x37"
"\x8B\x08\x42\xF8\x77\xBC\xDA\x45\x4B\x66\x01\xF0\xF2\x10\xE7\x54\x09\x82\x1D\x58\x20\x16"
"\x34\x3C\x79\x2D\x8C\x21\x97\x8A\x23\xB8\x0D\x83\x78\x8A\x15\x82\xFC\x68\x2D\x5F\x58\x48"
"\x84\x2F\x13\x94\x00\x60\x19\x70\x58\x20\x94\x60\x66\x4D\x4D\x2B\x51\x58\x43\x17\xC2\x0A"
"\x82\x01\x95\x70\x4D\x8D\x71\xAE\x6E\x7C\xD8\xB7\x1E\x8E\x8C\x65\x24\xC5\x10\x19\x77\x19"
"\xF9\xD3\xD0\xD8\xC8\x42\xD6\x33\x12\xEE\x4D\xF1\x16\x00\xC4\x83\x0A\xCE\xA8\x09\xD2\xA1"
"\x3B\xF2\xA1\x0C\x83\x63\x32\x68\x25\x7D\xBB\x73\x94\x87\xF1\x21\x04\x40\x06\x17\xF2\xA0"
"\xEA\x37\x83\x68\x4B\x58\x16\x19\x8E\x0D\x94\x74\xF6\xB8\x94\xAD\x11\x92\x29\xE9\x27\x01"
"\xE0\x8E\x8D\xD8\x2D\xDD\x38\x21\x82\x23\x0D\x68\xE0\x1F\x0E\x60\x12\xF8\x21\x63\x3A\xD6"
"\x34\x01\x20\x0E\x54\x13\x81\x88\xA8\x8A\x55\x16\x00\xD0\x01\x16\x0F\xEA\x6E\x02\x82\x95"
"\x6B\x18\x8C\xEB\xE5\x83\xEC\x85\x87\xCD\xF8\x88\x51\xC3\x96\xCD\xB3\x72\x5F\x32\x54\x91"
"\xF8\x20\xF4\x01\x18\x32\xC1\x47\x64\x00\x71\x54\xD2\x20\xE6\xC2\x0A\xB6\x41\x2B\x81\x82"
"\x49\x68\x38\xDE\x9A\x95\x21\x8E\xE6\x52\x17\x22\x39\x5C\x0E\x00\x90\x0C\x1C\x55\x08\x43"
"\x4E\x16\x00\x10\x70\xC4\x8A\x62\xA0\x47\x99\x62\x26\x49\x46\x2E\xF8\x96\xF1\x42\x3C\xEA"
"\x98\x9C\x2B\x19\x00\xF4\x64\x45\xCC\x62\x29\xEE\x99\x4B\x97\x42\x94\xA7\x02\x06\x01\x80"
"\x5C\x34\x68\x2E\xD0\xAE\x21\xC2\x14\xA1\x0F\x0A\x1F\x01\x43\x75\xA9\x97\x3C\xA8\x67\x5D"
"\x93\xCB\xA3\xC9\xA5\x63\x67\x99\x97\x6B\xF9\x69\x48\x0E\x89\x5B\xC1\x4C\xDD\x79\xBC\x0E"
"\x09\xC2\xD4\x66\xCE\x22\x7A\x20\x2A\x3A\xA3\x91\x71\x29\x66\xA1\x1D\x5E\x77\x20\x2E\xDA"
"\x1D\x4C\xE1\x1B\xB2\x96\x78\xA0\x61\x16\x9A\x7A\x29\x12\xB2\x8C\x1B\x62\x89\xFF\x61\xFF"
"\x92\xBA\x3C\x3A\x75\x29\x26\x79\x20\xE8\x34\x13\x54\x24\x5E\xE3\xA9\x9B\x69\xF9\x9B\xBD"
"\x66\xCE\xBE\x61\x16\xBC\xB7\xAC\xB1\x29\xCF\x14\x82\x5E\x3A\x98\x92\x52\x8B\x4D\x1A\x63"
"\x83\x01\x42\x3C\x76\x39\x06\xB5\x84\x07\x2A\x76\x20\x28\x28\x2B\xA8\x77\x58\x09\xAF\x3C"
"\x2E\x16\x13\x98\x02\x04\x52\xD1\xB7\xD4\x40\xB6\xC8\x28\x21\x1C\xFB\x44\xB4\x18\x0F\xB7"
"\x6B\x46\x1A\xCA\xC8\x1A\xE0\xE2\x28\x57\xFC\xE6\x62\xAF\x99\xD1\x85\x2F\x16\x26\x64\xE2"
"\x21\xBC\x49\x62\x83\x0F\x62\x54\xE2\x62\x1A\xF9\xD8\x00\x98\x00\x24\x96\x90\x82\x97\x9C"
"\xBC\xE4\x99\x11\xE2\x35\x02\x95\x60\x6D\x07\x29\x57\xAA\x1B\x82\xA1\x2F\x96\x42\x59\x0B"
"\x36\xB7\x7F\x6E\xA5\x46\xD7\x1F\x5E\x35\x60\x8B\x82\x4F\x93\x3B\xF5\x20\x46\x01\x26\xB6"
"\x9F\x94\xD5\x9F\x9E\x7B\xBA\xA9\xBB\xBA\xAD\xFB\xBA\xB1\x3B\xBB\xB5\x7B\xBB\xB9\xBB\xBB"
"\xBD\x1B\xA5\x84\xEF\xBB\x33\xEA\xAF\x33\x62\xFA\xB6\xFA\x22\xC8\x1B\x22\xD2\xDB\xBA\x75"
"\x9B\x21\xF6\xCD\x43\xDC\xA1\x35\x2F\xA2\xBD\xDD\xDA\xBB\xE9\x9B\xE2\xE4\xF4\x25\xEE\xDB"
"\xAA\xED\x7B\xB8\x49\x0C\x27\x90\x62\xBF\x6F\x97\xBB\xF9\x41\x0D\x88\xC0\x6D\xE1\x36\x20"
"\xB8\x82\x6C\xBB\xF6\x6B\xE1\xA4\x21\x78\x40\x72\xBE\x21\x13\xC2\x55\x6B\x43\xA4\x02\xF4"
"\x41\x83\x36\xC0\x14\x24\xB0\x67\x7D\xEB\x0B\x4C\xC0\x1D\xBE\xA1\x6C\x0F\x22\x02\xA0\x16"
"\x6A\xA1\xA1\xB0\xFF\x95\xFC\x98\x02\xE2\x14\xA8\x9B\x1A\x60\x77\x7B\xF7\x77\x2B\xE5\x20"
"\x6E\xC8\x33\x12\x1C\xC2\x36\xD7\x20\x80\x26\x4B\x48\x76\x59\x9D\xEA\x36\x3B\xE5\xB8\xDB"
"\x55\x7B\x01\x80\x7B\xC9\xF1\x28\xFF\xE1\x37\x36\xE0\x31\xC4\x45\x72\x3B\x85\x4F\x07\xC0"
"\xC3\xE6\x47\x00\x62\x41\x3F\x7F\x94\x08\x9A\xF7\xBC\x4D\xED\x1F\x02\x2A\xB6\x97\xF5\x17"
"\x82\x6F\x7A\x0D\x51\x2A\x02\x85\x72\xFB\xF5\x46\xF3\x5A\x18\x17\xB8\xC8\xE2\x70\x08\x4C"
"\x61\x9A\x0E\x35\x13\x32\xC1\x14\xDA\x3A\x3C\xD6\x12\x43\x10\x91\x80\x6D\x88\xCD\xBB\x70"
"\x8D\x93\x24\x81\x1D\xEB\x79\xFB\xB5\x87\x71\xA6\x89\xEA\xA6\x46\x34\xA8\x66\xDF\x11\xCC"
"\x02\x80\x26\xA2\x19\x73\x64\x38\x20\xEA\x79\x20\xE6\xA2\xAA\x5B\x79\x3C\x3D\x6D\xB1\x71"
"\xDB\xAC\xBD\x67\x0F\x7D\x30\x85\x8B\x4B\xC7\x83\xFC\xD3\xBB\x58\x18\x81\x24\x44\xBD\xAB"
"\x65\x16\x39\x11\x77\x86\xD0\xF3\x75\xA6\x9D\xD1\x89\x2A\x9D\x96\xDF\xED\xC4\xA1\xB6\x0C"
"\xEB\xB7\x90\x09\x82\x39\x88\x9C\x1F\x0E\x59\x00\xDD\x02\x6A\x61\x7D\x65\x3D\xDD\x68\x55"
"\xD3\x8B\x25\x9A\x73\x55\xFD\xD3\xEB\xC6\x9A\x4D\x3D\x80\x5D\xB9\x5E\x6A\xC4\x1B\x9C\xDB"
"\xB9\x04\xDC\xF5\x64\x1D\xB5\xC4\x54\x61\x08\x5D\xD9\xB4\x79\x21\xA8\x19\x20\xA0\x7D\xD6"
"\x7F\x9C\xAA\xA6\x1D\x73\xBE\xC7\x7B\x15\xDD\x60\xB7\x5D\x20\x18\x04\x13\x1A\xC0\xB1\x45"
"\xFD\xB4\x82\x72\x9F\xFF\xA1\x9F\xFF\xD9\x20\xEE\x7D\x21\x2A\x93\x0E\xFB\x61\xA0\xC3\x3D"
"\x97\x8B\x7D\x5E\xDF\x3D\x20\x36\xCC\xA8\x6F\x6E\xD9\x9F\xC3\x32\x5E\x15\xA5\x2F\x64\x3A"
"\xD6\xC7\x2C\x7A\x57\x3A\xB4\xA7\xA5\x8F\x6C\xD5\x0E\x40\xE2\xDD\xFD\xD8\x97\x05\x10\x32"
"\x81\x07\x1A\xE0\xDB\xC7\x2B\x19\x0F\xD5\x14\xFE\xD8\xC2\xB0\xFA\x70\xB8\xFC\x86\x2E\xD0"
"\x7C\xB2\x77\x29\xD0\xD6\xAB\xCD\xB9\x98\x66\x41\xB0\x0D\x56\xFC\xD6\x5B\x20\xF2\xB3\xDE"
"\x19\x42\x0D\x7E\x67\x06\x01\x00\xB1\x5D\x67\xBD\xC3\x2A\x2B\xB2\xB7\xB1\xB5\x44\x3E\x85"
"\x89\x98\xDA\x01\xC3\x43\x5B\x61\x07\xA0\x78\x88\x9C\x21\xFA\x11\x62\x75\xAF\xC2\xEF\xD1"
"\xB5\x19\xC2\x9B\xAC\x5E\xEB\xAB\x1B\x72\xA4\xDA\x32\xF0\x5A\xBC\x19\x8D\xD4\xB9\xCD\xEC"
"\x47\xAF\x5C\xC1\x5C\xED\xDD\xFE\xED\xE1\x3E\xEE\xE5\x7E\xEE\xE9\xBE\xEE\xED\xFE\xEE\xF1"
"\x3E\xEF\xF5\x7E\xEF\xF9\xBE\xEF\xFD\xFE\xEF\x01\x3F\xF0\x05\x7F\xF0\x09\xBF\xF0\x0D\xFF"
"\xF0\x11\x3F\xF1\x15\x7F\xF1\x19\xBF\xF1\x1D\xFF\xF1\x21\x3F\xF2\x25\x7F\xF2\x29\xBF\xF2"
"\x2D\xFF\xF2\x31\x3F\xF3\x35\x7F\xF3\x39\xBF\xF3\x3D\xFF\xF3\x41\x3F\xF4\x45\x7F\xF4\x37"
"\x4A\xF8\x68\x9B\xAB\x7A\x6F\x24\x20\xA6\xED\x1D\x8A\x7F\x4C\xD7\x6F\x4E\x53\x20\x1A\x23"
"\x21\x83\x51\xAB\x02\x80\x34\x6C\x8B\x22\xAF\x06\x2A\xAA\x48\xD1\x0A\xE2\xD0\x68\xDA\x91"
"\xAF\x85\xD4\xB3\x24\xA8\x01\x02\x76\xB2\xC4\x15\xC9\xEF\x2D\x78\xC0\x10\x65\xC1\x33\xB4"
"\x9C\xB6\xB2\x82\xF9\x9D\x1F\xFA\xE3\x2D\xBF\xD3\xD8\xCD\xAB\x5A\x1B\x8C\x7F\xE7\xF7\x67"
"\x01\x0B\x70\x29\x42\xB0\x31\x82\xEB\x88\x9A\x6F\x28\x69\xAC\x28\x15\x2C\x76\x93\x6C\x17"
"\x84\x0C\x55\x03\x59\xF8\x6D\x3D\x86\xC1\xDF\x2D\x80\xC8\xF8\x6F\xE8\x1B\x50\xFE\x08\x33"
"\x34\x28\x80\x28\x2B\x9C\x01\x08\x28\x30\x60\x03\x7F\xFF\xFC\x99\x82\xF6\xEF\x1F\xA6\x81"
"\x0E\xF9\xFD\x7B\xF7\x6E\xE1\x3F\x80\x0E\x09\x1A\x44\xA8\xF0\x1F\xA9\x8B\x00\xCE\xFC\xEB"
"\x77\xF1\x8C\xBF\x00\x0E\x41\xF2\x73\xA8\xE3\xA0\xC9\x8B\xFC\xE8\x78\x8C\xE9\x31\xDD\xBF"
"\x19\x17\x0B\xF8\x13\x39\x50\xC9\xBF\x4B\x1E\x13\xFD\x5B\x24\xB0\x40\x3F\x7F\x09\x2E\x2E"
"\x2B\x39\xF2\x1F\x08\x87\x5E\x6A\x5E\xD4\xA6\x73\x20\xC9\x96\x54\x95\xCA\x7C\x29\x33\x60"
"\x82\x83\x2D\xB6\x0E\xD4\x1A\x10\xC4\xBE\x9E\x1E\xB5\x31\x14\x08\xA2\xDF\x3F\x16\x31\xC5"
"\x02\x20\x6B\x36\xEA\xBF\x52\x6A\xD9\xBA\x3D\xB9\xF0\xCB\x49\xAC\x02\x41\xFE\x1B\x32\x10"
"\x1F\x4B\x8F\x70\xC1\x7A\xA4\x52\x31\xA6\xA8\xC0\x83\xFF\x31\xF0\x38\x80\x9F\xD1\x80\x94"
"\xFE\xFD\xF1\x28\xC7\xEF\x5F\x7F\x03\x1C\x7E\xC0\xEC\x30\x00\xBF\x6E\x23\x39\x07\xAC\xBA"
"\xF5\xB0\x47\x9E\x55\x10\x0B\x3C\x1C\x40\x9F\x3F\x03\x0E\x4B\x2C\x1E\x18\xDA\xB4\x61\x98"
"\x02\x67\xD7\x76\x08\xE2\x1F\x37\xD0\xFF\x78\x53\x5D\xD8\xCF\xEA\x47\xD4\x80\xA7\x0E\x59"
"\x88\x3A\x20\x6B\xD8\x01\x27\xFB\x8B\xEC\x91\xC1\xBF\x94\x01\x57\xC6\x92\xF9\x34\x3C\xF7"
"\xE5\x1E\x55\xF7\x65\x0E\x60\x40\x3F\xE4\x01\x35\x88\x4E\x7F\x5A\xFD\x43\xDF\x32\xE5\xFC"
"\xFB\x60\x1D\x00\xEB\x38\x4C\x1D\x22\xF2\x9F\x43\xBD\x54\xE6\x92\x7D\x01\xF9\xD7\xD4\x40"
"\x40\x29\x38\x10\x81\x47\x51\xD5\x0F\x18\xF1\xA5\xE6\x5C\x3F\x80\xFC\xF3\x1A\x00\xF9\xB8"
"\xC3\xC8\x74\xFC\x1D\xB8\x1F\x00\x8D\x85\xE8\x50\x25\xFF\x30\x11\x50\x3E\xD9\x6D\x95\xCF"
"\x3F\x11\x34\xA6\xE1\x69\x02\x45\x70\x40\x40\x5E\x4C\x87\x0E\x67\x5D\x40\x25\x9F\x8F\x6F"
"\x95\xE8\xD0\x0C\x73\xC1\xC6\xDA\x24\x02\x0A\xB4\xCC\x54\x03\xF1\x58\x42\x6F\x0E\x1D\xD9"
"\x60\x40\x4A\x7A\xD4\xE4\x49\xFD\x08\x90\x4D\x81\xCD\xA9\x77\x46\x3F\x07\xE4\x23\x12\x4F"
"\x43\x50\xF1\x61\x75\xB0\xC1\xB7\xA4\x47\x38\xF9\x03\x80\x19\x44\xC6\xB4\xD2\x42\xDE\x9D"
"\xF7\x4F\x05\x00\x84\xC6\x4E\x40\xAE\x4C\x07\x94\x06\x03\xED\xB2\x65\x67\xF4\xA1\x07\x24"
"\x58\xEC\xFD\x03\x06\x7D\x4F\x0E\x74\xE2\x9F\x03\x29\x33\x1D\x48\x52\xC6\x16\xA2\xA3\x0E"
"\x45\x4A\x28\x92\x15\xE2\xC9\xD1\xA0\x7D\x01\xC0\x13\x20\xEC\x88\x64\x86\x99\x41\x22\x46"
"\x93\x4D\x60\xE1\xF7\x8D\x3F\x9E\x21\x86\x56\x8F\x9A\xFD\x87\x46\x61\x49\xD1\x37\x64\x1A"
"\x03\xE9\x43\xE7\x55\x84\x7E\x58\x29\x62\x3B\x50\xF4\x4D\x26\x3C\xAC\x76\xE0\x70\x6A\x02"
"\x90\xE9\x52\x94\x52\xA7\x6C\x48\x17\x39\xAB\x17\xA5\xAA\x29\xF3\x0F\x05\x15\x76\xA9\x94"
"\x3E\x0B\xBD\x56\x68\x58\xA8\x6E\xA5\x98\x45\x88\xB9\x88\x22\x6C\xAD\x2C\xA4\x9D\x47\x2C"
"\xFC\xF3\x67\xB6\xF9\x35\xAB\xE7\x4D\xFD\x9C\x0B\x9F\x4F\xF3\xF1\x9B\x15\xB9\x03\x6D\x10"
"\x0C\x45\x21\x7D\x66\xD8\x1F\x01\x10\xD0\x80\x1A\x0B\xA5\xE8\xD0\x2B\xFF\xD8\x66\xAD\xC1"
"\x08\x2B\xCC\xF0\x45\x0F\x47\x9C\xDC\xB5\x4A\x51\x90\x9B\xB8\x5C\x02\xB0\xC4\xB4\x20\x1B"
"\x28\xE2\x7A\x45\xB9\x0B\x56\xC7\xBF\xC0\x06\x01\x45\xE1\xC5\x04\xDF\x6B\xFC\xD4\xA1\xCF"
"\x1C\x1B\x16\x77\x16\x56\xB6\xE6\xF5\xE3\xA7\xFE\x8A\x48\x80\x09\xDD\xFC\xD3\x4E\x4C\x65"
"\x0D\xFC\x4F\x90\x4A\x6A\x0A\x2D\x00\x48\x0F\xBC\xB4\x79\x12\xFF\x1A\x50\x63\x4E\x7E\x8C"
"\x1E\x08\x0D\x70\xCB\xE8\x7E\x24\x8A\xB8\xCF\xAE\x88\x65\x6B\xC5\x36\xF4\x7A\x44\x1A\x1A"
"\x11\xBC\xD8\x4A\x71\x67\x02\xC0\x73\x40\xDA\x7C\x48\x92\x47\x38\x2E\x2A\xAC\xC9\x00\xD0"
"\xF4\xE8\x43\xEE\xCC\x12\xCC\x2C\x07\xC5\x54\xAD\xC6\x86\x01\x2E\x78\x1D\x11\x14\x2E\xE9"
"\xA6\x24\x1F\xE0\x8F\xD1\xA6\x76\x0B\xEC\xA2\x70\xC7\x14\x1A\xB3\x60\x61\x2E\x10\x6E\x46"
"\xC7\x70\x5C\x4C\xA4\xA5\x51\x85\x48\x53\xF4\xD3\xC0\x3E\xA8\xB2\xED\x53\xE4\x2C\xD7\xC9"
"\xB8\x43\xA2\x04\x1B\xED\xDE\x00\xAC\xD4\xF0\x43\x63\x03\x70\xA2\x8C\x90\x62\x65\x15\x48"
"\xFA\xB9\xC4\xFB\x56\x86\xFF\xF5\x78\xA1\x01\xCE\x50\x66\xE5\xFD\xD6\x27\xA2\xAA\x26\x77"
"\x1E\x10\x4D\x79\x65\xF3\xCF\x0E\x31\x61\xF3\x0C\x30\x00\x75\x5C\xC7\x3F\x4E\xC6\x94\xCE"
"\xE9\x8E\x79\x64\x2B\xF9\x03\x61\x53\x3B\x88\xB7\x77\xEC\x87\xD7\x2E\xFB\x2A\x90\xAD\x82"
"\x05\xD0\x8F\x6F\xEA\x9C\x0A\xDB\xFD\x17\xE1\xC3\x39\x7E\x21\x4D\x3B\x00\x21\x40\xCB\x79"
"\x0D\x31\x8A\x81\x9D\x88\xAA\x17\x9D\x9C\x75\x6A\x73\x00\xE8\xC5\x5E\x02\xF2\xAD\x78\xC9"
"\xC4\x56\x2D\x60\x46\xB0\xA2\xD3\x91\x81\x44\x4E\x82\x7A\x33\x19\x50\x7A\x36\x2E\x48\xB5"
"\xE5\x22\x1D\x33\xDA\xFB\xE0\x75\xAE\xE8\x21\x66\x85\x0E\x71\x21\xF4\x3E\x22\x9D\xE7\xF9"
"\x6C\x84\x60\x21\x8A\xA0\xAC\x53\xBD\x6F\xF9\x0D\x00\xBB\xF8\x07\x5F\x96\x62\xA7\x80\x0C"
"\xB1\x76\x09\x30\x88\xA7\x64\x52\x16\x30\x08\x44\x01\xDC\x20\x22\xE7\xFE\x15\x10\xF5\x34"
"\xE1\x20\x19\x3B\xA1\x40\x56\x90\x1B\x87\x64\xAF\x14\x04\x10\x5B\x04\xF0\x62\x28\xD8\x64"
"\xAF\x21\x01\xD9\x80\x19\x73\xF8\xAD\x03\xD6\x50\x87\x5B\x01\xCC\x37\x24\x62\xC7\x77\x4C"
"\xED\x8C\x31\xC1\x0F\x03\x03\xE2\x32\x58\x09\x69\x64\x00\x50\x0C\x0B\x65\x42\x41\x0C\xCA"
"\x84\x58\x07\x31\x85\xC0\x8A\x86\x98\xEA\xC1\xE7\x1D\xD0\x98\x85\x29\xDC\xB1\x10\xC1\x24"
"\x10\x00\x84\x91\xDD\x40\x0A\x92\xB4\xB4\xE8\x11\x2C\x11\xC8\x48\x42\x16\x62\x97\x38\xF2"
"\x04\x8E\x39\x0C\xC8\x3E\xFC\x71\x47\x89\xE4\x31\x79\x9F\x54\x4E\xDE\x58\x69\xBC\x8B\xD0"
"\x26\x65\x01\x39\x51\x11\x07\xC2\x36\x06\xB2\x0D\x66\x32\x09\xCD\x0B\x63\x52\x01\x60\x50"
"\xC4\x1F\x50\x44\x8C\xD8\x54\xC6\x96\x81\x79\x63\x03\x4E\xBC\xA5\xC8\x6E\x19\x90\x03\x0C"
"\x6E\x21\xEF\x68\xC2\x56\x9A\x69\x1D\x6C\x26\x93\x9B\xE7\x99\x8E\xF9\x70\x58\xB5\x8B\x40"
"\x6D\x60\xB1\xBC\x1D\x3B\xDB\xE9\x4E\x99\x08\xE0\x00\x04\x78\x67\x4C\x04\x40\x80\x7B\xD2"
"\xD3\x23\x14\xB4\x26\x3D\xE3\x69\xA3\x7C\x02\x34\xA0\x02\x1D\x28\x41\x0B\xFA\xCE\x8E\xFD"
"\xE3\x1B\xA6\x78\x86\x41\x1B\xEA\xD0\x87\x42\x34\xA2\x12\xBD\x88\x09\xDA\x71\xC3\x89\x62"
"\x34\xA3\x1A\xDD\x28\x47\x05\x42\x80\x03\x08\xA0\xA3\x22\x1D\x29\x49\x4B\x6A\xD2\x93\xA2"
"\x34\xA5\x2A\x5D\x29\x4B\x5B\xEA\xD2\x97\xC2\x34\xA6\x32\x9D\x29\x4D\x6B\x6A\xD3\x9B\xE2"
"\x34\xA7\x3A\xDD\x29\x4F\x7B\xEA\xD3\x9F\x02\x35\xA8\x42\x1D\xEA\x46\xED\x49\x80\x90\xCE"
"\xF4\x9E\x47\x25\x6A\x41\x3F\x46\xC1\x55\xA1\x73\x96\xEB\x64\xE5\x27\xF9\xF9\x31\x92\x5D"
"\x84\x07\xDF\x48\x1A\x3F\x9F\x26\xD5\x45\xA5\x93\x22\xEB\x04\x8C\x74\x82\x21\xCE\xAD\x58"
"\xE1\x99\x2F\x33\xA2\x74\xA0\x71\xD6\xAD\x44\xA7\x90\x40\x15\x97\x25\x9A\xE8\x11\x7D\xBC"
"\x72\x22\x1F\xC2\xEB\x44\x22\xF2\x0F\xAB\x4A\xEA\x43\x67\xFB\x87\x3B\x4C\x31\x0B\x68\xF8"
"\xA3\x97\x0F\x89\xC8\x2B\xA7\xCA\x1F\xC6\xDE\x71\xAC\x90\x65\x62\x3F\x20\x74\x91\x00\xAC"
"\x63\x21\x0A\x3D\xAC\x3F\x20\x98\x3C\x3B\xAE\x15\x2C\x17\x54\xAC\x4F\x0B\x65\x2B\xF7\xC0"
"\xE6\xAA\x5C\x8C\xC9\x55\xAF\xDA\x18\x77\x48\xB3\x48\x56\x5C\xED\x1C\xFD\x12\x81\x66\x08"
"\xD2\x21\xAE\xF8\xC7\x33\xB8\x06\x96\x42\x55\xA0\x2C\xBA\xF3\x88\xC8\x66\xC1\x0F\x11\xE6"
"\x14\x3D\x2B\x41\x2E\x6B\xDD\xD7\xB9\xD6\xA2\x86\x6D\xCC\x85\xA1\x6C\x53\x8B\x1A\x45\xC0"
"\x29\x20\x1D\x9B\x6E\xD7\x04\xC2\x80\xCE\x6E\x25\x1D\x6D\xF2\xCF\xF6\xE6\xAA\x94\x0A\x1C"
"\xC4\xB2\xFB\x51\xAD\x1C\x57\xF9\xB1\xFF\xED\xA7\x7A\xB4\x95\xC9\x55\xF5\xC1\x2C\xF8\x22"
"\xE6\x63\xFB\xF0\xAC\x43\x32\xE0\xA9\xC8\x15\x93\xA7\x76\x2B\x00\x44\x9C\x96\x5F\xE7\xFE"
"\x0B\xBA\xEA\x41\xDE\x23\x67\xDB\xDE\x71\xD2\xE7\x52\xC0\xAB\xA5\xCF\x00\x2C\x93\x00\x11"
"\x2F\x52\x5B\x14\xF0\x97\xEE\x81\xBE\xBD\xB1\xD7\x76\xF4\x6D\xD3\x45\xBA\x80\x1A\x06\x73"
"\xAE\xAB\x86\x51\xF1\x39\x07\xF2\x14\x4A\xA1\xB8\xB9\x03\x09\x00\x6E\x49\x3B\x90\x30\x09"
"\x44\x64\xBF\x13\xF0\xC0\x0C\x7C\x60\x0A\x3F\xF7\x1F\x80\xA8\x03\x91\x89\xAC\x86\x60\x9C"
"\xD8\x2F\x3C\x30\x45\x26\x32\x61\x8A\xB7\x3C\x43\x0D\x45\xAE\x03\x0D\xB2\xF2\x0D\x46\x34"
"\xD9\xC9\x98\x74\xEF\xE3\x62\x5C\x27\x26\x67\x62\x16\x06\x09\xB0\x40\xE0\x23\x4C\xEE\x90"
"\x39\xB9\xEA\x64\x67\x88\xDF\x47\xDF\x59\x5E\x74\xC2\x02\xD9\x43\x32\xC1\xFA\x49\x45\x38"
"\xF1\xCE\x10\xBE\x08\x28\xBA\xEC\xBE\xE4\x7C\xD2\x1F\x06\xC6\xCF\x96\x35\x09\xC8\x9E\x82"
"\xC4\x34\xEC\x12\xA6\x88\xDA\xFC\x5C\x7F\xB4\x60\x03\x92\x96\x74\x05\x2E\x91\x64\xAB\x24"
"\xEC\x00\x8A\x30\x13\x26\x2A\x30\xE9\xAD\x65\x65\x11\x07\x68\x00\xA9\x1B\x30\xCF\x38\x02"
"\xE0\xC5\x98\xFA\x73\x67\x36\x50\xEA\x0A\x4C\xE8\x1F\xBE\x75\x48\x52\x0A\x16\x90\x00\xF9"
"\xD8\xA6\x24\xB1\x4D\x00\xCA\x52\x68\xEB\x38\x3A\xC1\x24\x76\x08\xE5\x68\x1D\xD8\xCB\xB1"
"\x78\x77\xD6\x6D\x9A\xB1\x29\x7C\x95\x8B\x10\x8B\xD1\x02\x29\x4B\x1D\xC0\x60\x6D\x2B\x64"
"\x36\xD9\x34\x45\x0F\x6E\x82\x23\xA2\x3E\x20\xB8\xB6\x08\x1C\x08\xED\x36\x8C\xD5\xF9\x26"
"\x6B\xD9\xD4\xF2\xC7\xAC\xAD\xE6\xED\x1F\xBB\x84\xBF\x00\x60\x1B\x9C\xA5\xAD\xD3\x42\x01"
"\x25\xCD\x32\xD1\x12\x90\x85\x3D\x6E\x81\xF0\xA8\x44\xAA\x95\xEF\x83\xE3\xC8\x9E\xFA\x0D"
"\x52\x69\xC0\x46\xCD\x00\xBE\xE8\xB9\xBA\x6C\xC0\x04\x12\x07\x01\x0D\xDA\xF3\x53\x71\xB9"
"\xC8\xC6\x01\xE1\xA4\x40\x5C\x26\xD7\x4C\xBA\x17\x35\x21\x44\xB5\x9B\xAB\x0B\xEF\x81\x2C"
"\xE3\x1F\xF2\x73\x48\x08\x9D\xDD\xDD\x8E\x83\x57\x2F\x50\x15\x08\xBF\x4B\xCB\x19\xF8\xBC"
"\xE8\x22\x5D\xF1\x06\x27\x23\x50\x16\x8D\xA3\xBB\xC5\x3F\x1B\x08\xBB\xBC\xD1\x6E\x34\xF4"
"\xCF\x87\x0E\x2E\x54\x04\x32\x2B\xC1\x50\xFC\x83\xE7\x27\x27\xC8\xB7\x86\xBB\x4B\x44\x02"
"\x8A\x45\x88\x46\x0D\x7E\x10\x7E\x80\x0B\xBE\x63\x16\x5B\x75\xB8\x43\xF8\x7A\x90\x77\xF8"
"\x03\xB0\xFF\x16\xC8\x00\x2E\x08\x8D\x30\x6F\x35\xE9\xB0\x69\x65\x5E\x25\x2B\x91\x3A\x4A"
"\x47\xBD\x33\x4E\xD7\x66\x83\x81\xC7\xA5\xDC\x91\x22\x64\xC6\xD5\xB3\x6C\x4E\x1F\xB4\xEC"
"\x38\x20\x4D\x18\x3B\x61\xAD\xD0\xCD\x4F\x06\x49\xC1\x31\x01\x83\x5A\x23\x22\x8B\xA3\x69"
"\x7B\xDA\x5F\xD5\x4B\x32\xDD\x8A\x98\x26\x58\x52\x9D\x5D\x4A\x9A\x3F\xBE\xB1\x4C\xA4\x4C"
"\x15\x24\x26\x60\xAA\x75\x02\x20\x00\xA4\x8E\xF4\xA3\x04\x70\x39\x4A\x65\x0F\x7B\xD6\xE3"
"\x3E\xF7\xBA\xDF\x3D\xEF\x7B\xEF\xFB\xDF\x03\x3F\xF8\xC2\x1F\x3E\xF1\x8B\x6F\xFC\xE3\x23"
"\x3F\xF9\xCA\x5F\x3E\xF3\x9B\xEF\xFC\xE7\x43\x3F\xFA\xD2\x9F\x3E\xF5\xAB\x6F\xFD\xEB\x63"
"\x3F\xFB\xDA\xDF\x3E\xF7\xBB\xEF\xFD\xEF\x83\x3F\xFC\xE2\x1F\x3F\xF9\xCB\x6F\xFE\xF3\xA3"
"\x3F\xFD\xEA\x5F\x3F\xFB\xDB\xEF\xFE\xF7\xC3\xFF\xF8";
#undef DD_ALIGNED_BUFFER

static const FontCharSet g_fontMonoid18CharSet = {
  /* bitmap               = */ g_fontMonoid18Bitmap,
  /* bitmapWidth          = */ 256,
  /* bitmapHeight         = */ 256,
  /* bitmapColorChannels  = */ 1,
  /* bitmapDecompressSize = */ 65536,
  /* charBaseHeight       = */ 20,
  /* charWidth            = */ 17,
  /* charHeight           = */ 30,
  /* charCount            = */ 96,
  {
   {   0,   0 }, {   0,   0 }, {   0,   0 }, {   0,   0 },
   {   0,   0 }, {   0,   0 }, {   0,   0 }, {   0,   0 },
   {   0,   0 }, {   0,   0 }, {   0, 150 }, {   0,   0 },
   {   0,   0 }, {   0,   0 }, {   0,   0 }, {   0,   0 },
   {   0,   0 }, {   0,   0 }, {   0,   0 }, {   0,   0 },
   {   0,   0 }, {   0,   0 }, {   0,   0 }, {   0,   0 },
   {   0,   0 }, {   0,   0 }, {   0,   0 }, {   0,   0 },
   {   0,   0 }, {   0,   0 }, {   0,   0 }, {   0,   0 },
   {   0,   0 }, {   0,  60 }, {  17,  60 }, {  68,   0 },
   { 153,   0 }, { 119,   0 }, {  34,   0 }, { 204,  30 },
   { 119,  30 }, { 102,  30 }, {   0,   0 }, { 102,   0 },
   { 170,  30 }, { 136,   0 }, { 187,  30 }, { 221,   0 },
   {  34,  60 }, { 187,  60 }, { 170,  60 }, { 153,  60 },
   { 136,  60 }, { 119,  60 }, { 102,  60 }, {  85,  60 },
   {  68,  60 }, {  51,  60 }, { 136,  30 }, { 153,  30 },
   {  17,  30 }, {  85,   0 }, {   0,  30 }, { 221,  30 },
   { 204,   0 }, { 170, 210 }, { 153, 210 }, { 136, 210 },
   { 119, 210 }, { 102, 210 }, {  85, 210 }, {  68, 210 },
   {  51, 210 }, {  34, 210 }, {  17, 210 }, {   0, 210 },
   { 238, 180 }, { 221, 180 }, { 204, 180 }, { 187, 180 },
   { 170, 180 }, { 153, 180 }, { 136, 180 }, { 119, 180 },
   { 102, 180 }, {  85, 180 }, {  68, 180 }, {  51, 180 },
   {  34, 180 }, {  17, 180 }, {   0, 180 }, {  85,  30 },
   { 187,   0 }, {  68,  30 }, { 170,   0 }, {  51,   0 },
   { 238,  30 }, { 119, 120 }, { 102, 120 }, {  85, 120 },
   {  68, 120 }, {  51, 120 }, {  34, 120 }, {  17, 120 },
   {   0, 120 }, { 238,  90 }, { 221,  90 }, { 204,  90 },
   { 187,  90 }, { 170,  90 }, { 153,  90 }, { 136,  90 },
   { 119,  90 }, { 102,  90 }, {  85,  90 }, {  68,  90 },
   {  51,  90 }, {  34,  90 }, {  17,  90 }, {   0,  90 },
   { 238,  60 }, { 221,  60 }, { 204,  60 }, {  51,  30 },
   { 238,   0 }, {  34,  30 }, {  17,   0 }, {   0,   0 },
   {   0,   0 }, {   0,   0 }, {   0,   0 }, {   0,   0 },
   {   0,   0 }, {   0,   0 }, {   0,   0 }, {   0,   0 },
   {   0,   0 }, {   0,   0 }, {   0,   0 }, {   0,   0 },
   {   0,   0 }, {   0,   0 }, {   0,   0 }, {   0,   0 },
   {   0,   0 }, {   0,   0 }, {   0,   0 }, {   0,   0 },
   {   0,   0 }, {   0,   0 }, {   0,   0 }, {   0,   0 },
   {   0,   0 }, {   0,   0 }, {   0,   0 }, {   0,   0 },
   {   0,   0 }, {   0,   0 }, {   0,   0 }, {   0,   0 },
   {   0,   0 }, {   0,   0 }, {   0,   0 }, {   0,   0 },
   {   0,   0 }, {   0,   0 }, {   0,   0 }, {   0,   0 },
   {   0,   0 }, {   0,   0 }, {   0,   0 }, {   0,   0 },
   {   0,   0 }, {   0,   0 }, {   0,   0 }, {   0,   0 },
   {   0,   0 }, {   0,   0 }, {   0,   0 }, {   0,   0 },
   {   0,   0 }, {   0,   0 }, {   0,   0 }, {   0,   0 },
   {   0,   0 }, {   0,   0 }, {   0,   0 }, {   0,   0 },
   {   0,   0 }, {   0,   0 }, {   0,   0 }, {   0,   0 },
   {   0,   0 }, {   0,   0 }, {   0,   0 }, {   0,   0 },
   {   0,   0 }, {   0,   0 }, {   0,   0 }, {   0,   0 },
   {   0,   0 }, {   0,   0 }, {   0,   0 }, {   0,   0 },
   {   0,   0 }, {   0,   0 }, {   0,   0 }, {   0,   0 },
   {   0,   0 }, {   0,   0 }, {   0,   0 }, {   0,   0 },
   {   0,   0 }, {   0,   0 }, {   0,   0 }, {   0,   0 },
   {   0,   0 }, {   0,   0 }, {   0,   0 }, {   0,   0 },
   {   0,   0 }, {   0,   0 }, {   0,   0 }, {   0,   0 },
   {   0,   0 }, {   0,   0 }, {   0,   0 }, {   0,   0 },
   {   0,   0 }, {   0,   0 }, {   0,   0 }, {   0,   0 },
   {   0,   0 }, {   0,   0 }, {   0,   0 }, {   0,   0 },
   {   0,   0 }, {   0,   0 }, {   0,   0 }, {   0,   0 },
   {   0,   0 }, {   0,   0 }, {   0,   0 }, {   0,   0 },
   {   0,   0 }, {   0,   0 }, {   0,   0 }, {   0,   0 },
   {   0,   0 }, {   0,   0 }, {   0,   0 }, {   0,   0 },
   {   0,   0 }, {   0,   0 }, {   0,   0 }, {   0,   0 }
  }
};

// ========================================================
// LZW decompression helpers for the font bitmap:
// ========================================================

// These must match the font-tool encoder!
static const int LzwNil            = -1;
static const int LzwMaxDictBits    = 12;
static const int LzwStartBits      = 9;
static const int LzwFirstCode      = (1 << (LzwStartBits - 1)); // 256
static const int LzwMaxDictEntries = (1 << LzwMaxDictBits);     // 4096

struct LzwDictionary
{
    // Dictionary entries 0-255 are always reserved to the byte/ASCII range.
    struct Entry
    {
        int code;
        int value;
    };

    int size;
    Entry entries[LzwMaxDictEntries];

    LzwDictionary();
    int findIndex(int code, int value) const;
    bool add(int code, int value);
    bool flush(int & codeBitsWidth);
};

struct LzwBitStreamReader
{
    const UByte * stream; // Pointer to the external bit stream. Not owned by the reader.
    int sizeInBytes;      // Size of the stream in bytes. Might include padding.
    int sizeInBits;       // Size of the stream in bits, padding not include.
    int currBytePos;      // Current byte being read in the stream.
    int nextBitPos;       // Bit position within the current byte to access next. 0 to 7.
    int numBitsRead;      // Total bits read from the stream so far. Never includes byte-rounding.

    LzwBitStreamReader(const UByte * bitStream, int byteCount, int bitCount);
    bool readNextBit(int & outBit);
    int readBits(int bitCount);
};

//
// LzwDictionary implementation:
//

LzwDictionary::LzwDictionary()
{
    // First 256 dictionary entries are reserved to the byte/ASCII
    // range. Additional entries follow for the character sequences
    // found in the input. Up to 4096 - 256 (LzwMaxDictEntries - LzwFirstCode).
    size = LzwFirstCode;
    for (int i = 0; i < size; ++i)
    {
        entries[i].code  = LzwNil;
        entries[i].value = i;
    }
}

int LzwDictionary::findIndex(const int code, const int value) const
{
    if (code == LzwNil)
    {
        return value;
    }
    for (int i = 0; i < size; ++i)
    {
        if (entries[i].code == code && entries[i].value == value)
        {
            return i;
        }
    }
    return LzwNil;
}

bool LzwDictionary::add(const int code, const int value)
{
    if (size == LzwMaxDictEntries)
    {
        return false;
    }
    entries[size].code  = code;
    entries[size].value = value;
    ++size;
    return true;
}

bool LzwDictionary::flush(int & codeBitsWidth)
{
    if (size == (1 << codeBitsWidth))
    {
        ++codeBitsWidth;
        if (codeBitsWidth > LzwMaxDictBits)
        {
            // Clear the dictionary (except the first 256 byte entries).
            codeBitsWidth = LzwStartBits;
            size = LzwFirstCode;
            return true;
        }
    }
    return false;
}

//
// LzwBitStreamReader implementation:
//

LzwBitStreamReader::LzwBitStreamReader(const UByte * bitStream, const int byteCount, const int bitCount)
    : stream(bitStream)
    , sizeInBytes(byteCount)
    , sizeInBits(bitCount)
    , currBytePos(0)
    , nextBitPos(0)
    , numBitsRead(0)
{ }

bool LzwBitStreamReader::readNextBit(int & outBit)
{
    if (numBitsRead >= sizeInBits)
    {
        return false; // We are done.
    }

    const int mask = 1 << nextBitPos;
    outBit = !!(stream[currBytePos] & mask);
    ++numBitsRead;

    if (++nextBitPos == 8)
    {
        nextBitPos = 0;
        ++currBytePos;
    }
    return true;
}

int LzwBitStreamReader::readBits(const int bitCount)
{
    int num = 0;
    for (int b = 0; b < bitCount; ++b)
    {
        int bit;
        if (!readNextBit(bit))
        {
            break;
        }
        const int mask = 1 << b;
        num = (num & ~mask) | (-bit & mask);
    }
    return num;
}

// ========================================================
// lzwDecompress() and helpers:
// ========================================================

bool lzwOutputByte(int code, UByte *& output, int outputSizeBytes, int & bytesDecodedSoFar)
{
    if (code < 0 || code >= 256) { return false; }
    if (bytesDecodedSoFar >= outputSizeBytes) { return false; }
    *output++ = static_cast<UByte>(code);
    ++bytesDecodedSoFar;
    return true;
}

bool lzwOutputSequence(const LzwDictionary & dict, int code,
                       UByte *& output, int outputSizeBytes,
                       int & bytesDecodedSoFar, int & firstByte)
{
    // A sequence is stored backwards, so we have to write
    // it to a temp then output the buffer in reverse.
    int i = 0;
    UByte sequence[LzwMaxDictEntries];
    do
    {
        sequence[i++] = dict.entries[code].value & 0xFF;
        code = dict.entries[code].code;
    } while (code >= 0);

    firstByte = sequence[--i];
    for (; i >= 0; --i)
    {
        if (!lzwOutputByte(sequence[i], output, outputSizeBytes, bytesDecodedSoFar))
        {
            return false;
        }
    }
    return true;
}

int lzwDecompress(const void * compressedData, int compressedSizeBytes,
                  int compressedSizeBits, void * uncompressedData,
                  int uncompressedSizeBytes)
{
    if (compressedData == DD_NULL || uncompressedData == DD_NULL)
    {
        return 0;
    }
    if (compressedSizeBytes <= 0 || compressedSizeBits <= 0 || uncompressedSizeBytes <= 0)
    {
        return 0;
    }

    int code          = LzwNil;
    int prevCode      = LzwNil;
    int codeBitsWidth = LzwStartBits;
    int firstByte     = 0;
    int bytesDecoded  = 0;

    const UByte * compressedPtr = reinterpret_cast<const UByte *>(compressedData);
    UByte * uncompressedPtr = reinterpret_cast<UByte *>(uncompressedData);

    // We'll reconstruct the dictionary based on the bit stream codes.
    LzwBitStreamReader bitStream(compressedPtr, compressedSizeBytes, compressedSizeBits);
    LzwDictionary dictionary;

    // We check to avoid an overflow of the user buffer.
    // If the buffer is smaller than the decompressed size, we
    // break the loop and return the current decompression count.
    while (bitStream.numBitsRead < bitStream.sizeInBits)
    {
        if (codeBitsWidth > LzwMaxDictBits)
        {
            break;
        }
        code = bitStream.readBits(codeBitsWidth);

        if (prevCode == LzwNil)
        {
            if (!lzwOutputByte(code, uncompressedPtr, uncompressedSizeBytes, bytesDecoded))
            {
                break;
            }
            firstByte = code;
            prevCode  = code;
            continue;
        }

        if (code >= dictionary.size)
        {
            if (!lzwOutputSequence(dictionary, prevCode, uncompressedPtr,
                 uncompressedSizeBytes, bytesDecoded, firstByte))
            {
                break;
            }
            if (!lzwOutputByte(firstByte, uncompressedPtr, uncompressedSizeBytes, bytesDecoded))
            {
                break;
            }
        }
        else
        {
            if (!lzwOutputSequence(dictionary, code, uncompressedPtr,
                 uncompressedSizeBytes, bytesDecoded, firstByte))
            {
                break;
            }
        }

        if (!dictionary.add(prevCode, firstByte))
        {
            break;
        }

        if (dictionary.flush(codeBitsWidth))
        {
            prevCode = LzwNil;
        }
        else
        {
            prevCode = code;
        }
    }

    return bytesDecoded;
}

// ========================================================
// Built-in font glyph bitmap decompression:
// ========================================================

// If you decide to change the font, these are the only things that
// need to be updated. The g_fontXYZ variables are never referenced
// directly in the code, these functions are used instead.
inline const UByte * getRawFontBitmapData() { return g_fontMonoid18Bitmap;  }
inline const FontCharSet & getFontCharSet() { return g_fontMonoid18CharSet; }

UByte * decompressFontBitmap()
{
    const ddU32 * compressedData = reinterpret_cast<const ddU32 *>(getRawFontBitmapData());

    // First two uint32s are the compressed size in
    // bytes followed by the compressed size in bits.
    const int compressedSizeBytes = *compressedData++;
    const int compressedSizeBits  = *compressedData++;

    // Allocate the decompression buffer:
    const int uncompressedSizeBytes = getFontCharSet().bitmapDecompressSize;
    UByte * uncompressedData = reinterpret_cast<UByte *>(DD_MALLOC(uncompressedSizeBytes));

    // Out of memory? Font rendering will be disable.
    if (uncompressedData == DD_NULL)
    {
        return DD_NULL;
    }

    // Decode the bitmap pixels (stored with an LZW-flavor of compression):
    const int bytesDecoded = lzwDecompress(compressedData,
                                           compressedSizeBytes,
                                           compressedSizeBits,
                                           uncompressedData,
                                           uncompressedSizeBytes);

    // Unexpected decompression size? Probably a data mismatch in the font-tool.
    if (bytesDecoded != uncompressedSizeBytes)
    {
        DD_MFREE(uncompressedData);
        return DD_NULL;
    }

    // Must later free with DD_MFREE().
    return uncompressedData;
}

// ========================================================
// Internal Debug Draw queue and helper types/functions:
// ========================================================

struct DebugString
{
    ddI64  expiryDateMillis;
    ddVec3 color;
    float  posX;
    float  posY;
    float  scaling;
    ddStr  text;
    bool   centered;
};

struct DebugPoint
{
    ddI64  expiryDateMillis;
    ddVec3 position;
    ddVec3 color;
    float  size;
    bool   depthEnabled;
};

struct DebugLine
{
    ddI64  expiryDateMillis;
    ddVec3 posFrom;
    ddVec3 posTo;
    ddVec3 color;
    bool   depthEnabled;
};

// Debug strings queue (2D screen-space strings + 3D projected labels):
static int g_debugStringsCount = 0;
static DebugString g_debugStrings[DEBUG_DRAW_MAX_STRINGS];

// 3D debug points queue:
static int g_debugPointsCount = 0;
static DebugPoint g_debugPoints[DEBUG_DRAW_MAX_POINTS];

// 3D debug lines queue:
static int g_debugLinesCount = 0;
static DebugLine g_debugLines[DEBUG_DRAW_MAX_LINES];

// Temporary vertex buffer we use to expand the lines/points before calling on RenderInterface.
static int g_vertexBufferUsed = 0;
static DrawVertex g_vertexBuffer[DEBUG_DRAW_VERTEX_BUFFER_SIZE];

// Latest time value (in milliseconds) from dd::flush().
static ddI64 g_currentTimeMillis = 0;

// Ref to the external renderer. Can be null for a no-op debug draw.
static RenderInterface * g_renderInterface = DD_NULL;

// Our built-in glyph bitmap. If kept null, no text is rendered.
static GlyphTextureHandle g_glyphTex = DD_NULL;

// ========================================================
// Fast approximations of math functions used by DD.
// ========================================================

// We only need these if the user didn't want the math.h dependency.
#if !DEBUG_DRAW_USE_STD_MATH

union Float2UInt
{
    float asFloat;
    ddU32 asUInt;
};

inline float roundFloat(float x)
{
    // Probably slower than std::floor(), also depends of FPU settings,
    // but we only need this for that special sin/cos() case anyways...
    const int i = static_cast<int>(x);
    return (x >= 0.0f) ? static_cast<float>(i) : static_cast<float>(i - 1);
}

inline float fastFabs(float x)
{
    // Mask-off the sign bit
    Float2UInt i;
    i.asFloat = x;
    i.asUInt &= 0x7FFFFFFF;
    return i.asFloat;
}

inline float fastInvSqrt(float x)
{
    // Modified version of the emblematic Q_rsqrt() from Quake 3.
    // See: http://en.wikipedia.org/wiki/Fast_inverse_square_root
    Float2UInt i;
    float y, r;
    y = x * 0.5f;
    i.asFloat = x;
    i.asUInt = 0x5F3759DF - (i.asUInt >> 1);
    r = i.asFloat;
    r = r * (1.5f - (r * r * y));
    return r;
}

inline float fastSin(float radians)
{
    static const float A = -2.39e-08;
    static const float B =  2.7526e-06;
    static const float C =  1.98409e-04;
    static const float D =  8.3333315e-03;
    static const float E =  1.666666664e-01;
    static const float HALFPI = DD_PI * 0.5f;

    if (radians < 0.0f || radians >= DD_TAU)
    {
        radians -= roundFloat(radians / DD_TAU) * DD_TAU;
    }

    if (radians < DD_PI)
    {
        if (radians > HALFPI)
        {
            radians = DD_PI - radians;
        }
    }
    else
    {
        radians = (radians > (DD_PI + HALFPI)) ? (radians - DD_TAU) : (DD_PI - radians);
    }

    const float s = radians * radians;
    return radians * (((((A * s + B) * s - C) * s + D) * s - E) * s + 1.0f);
}

inline float fastCos(float radians)
{
    static const float A = -2.605e-07;
    static const float B =  2.47609e-05;
    static const float C =  1.3888397e-03;
    static const float D =  4.16666418e-02;
    static const float E =  4.999999963e-01;
    static const float HALFPI = DD_PI * 0.5f;

    if (radians < 0.0f || radians >= DD_TAU)
    {
        radians -= roundFloat(radians / DD_TAU) * DD_TAU;
    }

    float d;
    if (radians < DD_PI)
    {
        if (radians > HALFPI)
        {
            radians = DD_PI - radians;
            d = -1.0f;
        }
        else
        {
            d = 1.0f;
        }
    }
    else
    {
        if (radians > (DD_PI + HALFPI))
        {
            radians = radians - DD_TAU;
            d = 1.0f;
        }
        else
        {
            radians = DD_PI - radians;
            d = -1.0f;
        }
    }

    const float s = radians * radians;
    return d * (((((A * s + B) * s - C) * s + D) * s - E) * s + 1.0f);
}

#endif // DEBUG_DRAW_USE_STD_MATH

// ========================================================
// ddVec3 helpers:
// ========================================================

enum { X, Y, Z, W };

inline void vecSet(ddVec3 & dest, const float x, const float y, const float z)
{
    dest[X] = x;
    dest[Y] = y;
    dest[Z] = z;
}

inline void vecCopy(ddVec3 & dest, ddVec3Param src)
{
    dest[X] = src[X];
    dest[Y] = src[Y];
    dest[Z] = src[Z];
}

inline void vecAdd(ddVec3 & result, ddVec3Param a, ddVec3Param b)
{
    result[X] = a[X] + b[X];
    result[Y] = a[Y] + b[Y];
    result[Z] = a[Z] + b[Z];
}

inline void vecSub(ddVec3 & result, ddVec3Param a, ddVec3Param b)
{
    result[X] = a[X] - b[X];
    result[Y] = a[Y] - b[Y];
    result[Z] = a[Z] - b[Z];
}

inline void vecScale(ddVec3 & result, ddVec3Param v, const float s)
{
    result[X] = v[X] * s;
    result[Y] = v[Y] * s;
    result[Z] = v[Z] * s;
}

inline void vecNormalize(ddVec3 & result, ddVec3Param v)
{
    const float lenSqr = v[X] * v[X] + v[Y] * v[Y] + v[Z] * v[Z];
    const float invLen = DD_INV_FSQRT(lenSqr);
    result[X] = v[X] * invLen;
    result[Y] = v[Y] * invLen;
    result[Z] = v[Z] * invLen;
}

inline void vecOrthogonalBasis(ddVec3 & left, ddVec3 & up, ddVec3Param v)
{
    // Produces two orthogonal vectors for normalized vector v.
    float lenSqr, invLen;
    if (DD_FABS(v[Z]) > 0.7f)
    {
        lenSqr = v[Y] * v[Y] + v[Z] * v[Z];
        invLen = DD_INV_FSQRT(lenSqr);
        up[X] = 0.0f;
        up[Y] =  v[Z] * invLen;
        up[Z] = -v[Y] * invLen;
        left[X] = lenSqr * invLen;
        left[Y] = -v[X] * up[Z];
        left[Z] =  v[X] * up[Y];
    }
    else
    {
        lenSqr = v[X] * v[X] + v[Y] * v[Y];
        invLen = DD_INV_FSQRT(lenSqr);
        left[X] = -v[Y] * invLen;
        left[Y] =  v[X] * invLen;
        left[Z] = 0.0f;
        up[X] = -v[Z] * left[Y];
        up[Y] =  v[Z] * left[X];
        up[Z] = lenSqr * invLen;
    }
}

// ========================================================
// ddMat4x4 helpers:
// ========================================================

inline void matTransformPointXYZ(ddVec3 & result, ddVec3Param p, ddMat4x4Param m)
{
    result[X] = (m[0] * p[X]) + (m[4] * p[Y]) + (m[8]  * p[Z]) + m[12]; // p[W] assumed to be 1
    result[Y] = (m[1] * p[X]) + (m[5] * p[Y]) + (m[9]  * p[Z]) + m[13];
    result[Z] = (m[2] * p[X]) + (m[6] * p[Y]) + (m[10] * p[Z]) + m[14];
}

inline void matTransformPointXYZW(float result[4], ddVec3Param p, ddMat4x4Param m)
{
    result[X] = (m[0] * p[X]) + (m[4] * p[Y]) + (m[8]  * p[Z]) + m[12]; // p[W] assumed to be 1
    result[Y] = (m[1] * p[X]) + (m[5] * p[Y]) + (m[9]  * p[Z]) + m[13];
    result[Z] = (m[2] * p[X]) + (m[6] * p[Y]) + (m[10] * p[Z]) + m[14];
    result[W] = (m[3] * p[X]) + (m[7] * p[Y]) + (m[11] * p[Z]) + m[15];
}

inline float matTransformPointXYZW2(ddVec3 & result, const float p[3], ddMat4x4Param m)
{
    result[X] = (m[0] * p[X]) + (m[4] * p[Y]) + (m[8]  * p[Z]) + m[12]; // p[W] assumed to be 1
    result[Y] = (m[1] * p[X]) + (m[5] * p[Y]) + (m[9]  * p[Z]) + m[13];
    result[Z] = (m[2] * p[X]) + (m[6] * p[Y]) + (m[10] * p[Z]) + m[14];
    float rw  = (m[3] * p[X]) + (m[7] * p[Y]) + (m[11] * p[Z]) + m[15];
    return rw;
}

// ========================================================
// Misc local functions for draw queue management:
// ========================================================

enum DrawMode
{
    DrawModePoints,
    DrawModeLines,
    DrawModeText
};

void flushDebugVerts(const DrawMode mode, const bool depthEnabled)
{
    if (g_vertexBufferUsed == 0)
    {
        return;
    }

    switch (mode)
    {
    case DrawModePoints :
        g_renderInterface->drawPointList(g_vertexBuffer, g_vertexBufferUsed, depthEnabled);
        break;
    case DrawModeLines :
        g_renderInterface->drawLineList(g_vertexBuffer, g_vertexBufferUsed, depthEnabled);
        break;
    case DrawModeText :
        g_renderInterface->drawGlyphList(g_vertexBuffer, g_vertexBufferUsed, g_glyphTex);
        break;
    } // switch (mode)

    g_vertexBufferUsed = 0;
}

void pushPointVert(const DebugPoint & point)
{
    // Make room for one more vert:
    if ((g_vertexBufferUsed + 1) >= DEBUG_DRAW_VERTEX_BUFFER_SIZE)
    {
        flushDebugVerts(DrawModePoints, point.depthEnabled);
    }

    DrawVertex & v = g_vertexBuffer[g_vertexBufferUsed++];
    v.point.x    = point.position[X];
    v.point.y    = point.position[Y];
    v.point.z    = point.position[Z];
    v.point.r    = point.color[X];
    v.point.g    = point.color[Y];
    v.point.b    = point.color[Z];
    v.point.size = point.size;
}

void pushLineVert(const DebugLine & line)
{
    // Make room for two more verts:
    if ((g_vertexBufferUsed + 2) >= DEBUG_DRAW_VERTEX_BUFFER_SIZE)
    {
        flushDebugVerts(DrawModeLines, line.depthEnabled);
    }

    DrawVertex & v0 = g_vertexBuffer[g_vertexBufferUsed++];
    DrawVertex & v1 = g_vertexBuffer[g_vertexBufferUsed++];

    v0.line.x = line.posFrom[X];
    v0.line.y = line.posFrom[Y];
    v0.line.z = line.posFrom[Z];
    v0.line.r = line.color[X];
    v0.line.g = line.color[Y];
    v0.line.b = line.color[Z];

    v1.line.x = line.posTo[X];
    v1.line.y = line.posTo[Y];
    v1.line.z = line.posTo[Z];
    v1.line.r = line.color[X];
    v1.line.g = line.color[Y];
    v1.line.b = line.color[Z];
}

void pushGlyphVerts(const DrawVertex verts[4])
{
    static const int indexes[6] = { 0, 1, 2, 2, 1, 3 };

    // Make room for one more glyph (2 tris):
    if ((g_vertexBufferUsed + 6) >= DEBUG_DRAW_VERTEX_BUFFER_SIZE)
    {
        flushDebugVerts(DrawModeText, false);
    }

    for (int i = 0; i < 6; ++i)
    {
        g_vertexBuffer[g_vertexBufferUsed++].glyph = verts[indexes[i]].glyph;
    }
}

void pushStringGlyphs(float x, float y, const char * text, ddVec3Param color, const float scaling)
{
    // Invariants for all characters:
    const float initialX    = x;
    const float scaleU      = static_cast<float>(getFontCharSet().bitmapWidth);
    const float scaleV      = static_cast<float>(getFontCharSet().bitmapHeight);
    const float fixedWidth  = static_cast<float>(getFontCharSet().charWidth);
    const float fixedHeight = static_cast<float>(getFontCharSet().charHeight);
    const float tabW        = fixedWidth  * 4.0f * scaling; // TAB = 4 spaces.
    const float chrW        = fixedWidth  * scaling;
    const float chrH        = fixedHeight * scaling;

    for (; *text != '\0'; ++text)
    {
        const int charVal = *text;
        if (charVal >= FontCharSet::MaxChars)
        {
            continue;
        }
        if (charVal == ' ')
        {
            x += chrW;
            continue;
        }
        if (charVal == '\t')
        {
            x += tabW;
            continue;
        }
        if (charVal == '\n')
        {
            y += chrH;
            x  = initialX;
            continue;
        }

        const FontChar fontChar = getFontCharSet().chars[charVal];
        const float u0 = (fontChar.x + 0.5f) / scaleU;
        const float v0 = (fontChar.y + 0.5f) / scaleV;
        const float u1 = u0 + (fixedWidth  / scaleU);
        const float v1 = v0 + (fixedHeight / scaleV);

        DrawVertex verts[4];
        verts[0].glyph.x = x;
        verts[0].glyph.y = y;
        verts[0].glyph.u = u0;
        verts[0].glyph.v = v0;
        verts[0].glyph.r = color[X];
        verts[0].glyph.g = color[Y];
        verts[0].glyph.b = color[Z];
        verts[1].glyph.x = x;
        verts[1].glyph.y = y + chrH;
        verts[1].glyph.u = u0;
        verts[1].glyph.v = v1;
        verts[1].glyph.r = color[X];
        verts[1].glyph.g = color[Y];
        verts[1].glyph.b = color[Z];
        verts[2].glyph.x = x + chrW;
        verts[2].glyph.y = y;
        verts[2].glyph.u = u1;
        verts[2].glyph.v = v0;
        verts[2].glyph.r = color[X];
        verts[2].glyph.g = color[Y];
        verts[2].glyph.b = color[Z];
        verts[3].glyph.x = x + chrW;
        verts[3].glyph.y = y + chrH;
        verts[3].glyph.u = u1;
        verts[3].glyph.v = v1;
        verts[3].glyph.r = color[X];
        verts[3].glyph.g = color[Y];
        verts[3].glyph.b = color[Z];

        pushGlyphVerts(verts);
        x += chrW;
    }
}

float calcTextWidth(const char * text, const float scaling)
{
    const float fixedWidth = static_cast<float>(getFontCharSet().charWidth);
    const float tabW = fixedWidth * 4.0f * scaling; // TAB = 4 spaces.
    const float chrW = fixedWidth * scaling;

    float x = 0.0f;
    for (; *text != '\0'; ++text)
    {
        // Tabs are handled differently (4 spaces)
        if (*text == '\t')
        {
            x += tabW;
        }
        else // Non-tab char (including whitespace)
        {
            x += chrW;
        }
    }

    return x;
}

void drawDebugStrings()
{
    if (g_debugStringsCount == 0)
    {
        return;
    }

    for (int i = 0; i < g_debugStringsCount; ++i)
    {
        const DebugString & dstr = g_debugStrings[i];
        if (dstr.centered)
        {
            // 3D Labels are centered at the point of origin, e.g. center-aligned.
            const float offset = calcTextWidth(dstr.text.c_str(), dstr.scaling) * 0.5f;
            pushStringGlyphs(dstr.posX - offset, dstr.posY, dstr.text.c_str(), dstr.color, dstr.scaling);
        }
        else
        {
            // Left-aligned
            pushStringGlyphs(dstr.posX, dstr.posY, dstr.text.c_str(), dstr.color, dstr.scaling);
        }
    }

    flushDebugVerts(DrawModeText, false);
}

void drawDebugPoints()
{
    if (g_debugPointsCount == 0)
    {
        return;
    }

    //
    // First pass, points with depth test ENABLED:
    //
    int numDepthlessPoints = 0;
    for (int i = 0; i < g_debugPointsCount; ++i)
    {
        const DebugPoint & point = g_debugPoints[i];
        if (point.depthEnabled)
        {
            pushPointVert(point);
        }
        numDepthlessPoints += !point.depthEnabled;
    }
    flushDebugVerts(DrawModePoints, true);

    //
    // Second pass draws points with depth DISABLED:
    //
    if (numDepthlessPoints > 0)
    {
        for (int i = 0; i < g_debugPointsCount; ++i)
        {
            const DebugPoint & point = g_debugPoints[i];
            if (!point.depthEnabled)
            {
                pushPointVert(point);
            }
        }
        flushDebugVerts(DrawModePoints, false);
    }
}

void drawDebugLines()
{
    if (g_debugLinesCount == 0)
    {
        return;
    }

    //
    // First pass, lines with depth test ENABLED:
    //
    int numDepthlessLines = 0;
    for (int i = 0; i < g_debugLinesCount; ++i)
    {
        const DebugLine & line = g_debugLines[i];
        if (line.depthEnabled)
        {
            pushLineVert(line);
        }
        numDepthlessLines += !line.depthEnabled;
    }
    flushDebugVerts(DrawModeLines, true);

    //
    // Second pass draws lines with depth DISABLED:
    //
    if (numDepthlessLines > 0)
    {
        for (int i = 0; i < g_debugLinesCount; ++i)
        {
            const DebugLine & line = g_debugLines[i];
            if (!line.depthEnabled)
            {
                pushLineVert(line);
            }
        }
        flushDebugVerts(DrawModeLines, false);
    }
}

template<typename T>
void clearDebugQueue(T * queue, int & queueCount)
{
    if (g_currentTimeMillis == 0)
    {
        queueCount = 0;
        return;
    }

    int index = 0;
    T * pElem = queue;

    // Concatenate elements that still need to be draw on future frames:
    for (int i = 0; i < queueCount; ++i, ++pElem)
    {
        if (pElem->expiryDateMillis > g_currentTimeMillis)
        {
            if (index != i)
            {
                queue[index] = *pElem;
            }
            ++index;
        }
    }

    queueCount = index;
}

void setupGlyphTexture()
{
    if (g_renderInterface == DD_NULL)
    {
        return;
    }

    if (g_glyphTex != DD_NULL)
    {
        g_renderInterface->destroyGlyphTexture(g_glyphTex);
        g_glyphTex = DD_NULL;
    }

    UByte * decompressedBitmap = decompressFontBitmap();
    if (decompressedBitmap == DD_NULL)
    {
        return; // Failed to decompressed. No font rendering available.
    }

    g_glyphTex = g_renderInterface->createGlyphTexture(
                         getFontCharSet().bitmapWidth,
                         getFontCharSet().bitmapHeight,
                         decompressedBitmap);

    // No longer needed.
    DD_MFREE(decompressedBitmap);
}

} // namespace unnamed {}

// ========================================================
// Public Debug Draw interface:
// ========================================================

void initialize(RenderInterface * renderer)
{
    if (g_renderInterface != DD_NULL) // Reinitializing?
    {
        shutdown(); // Shutdown first.
    }

    g_renderInterface = renderer;
    g_currentTimeMillis = 0;
    g_vertexBufferUsed  = 0;
    g_debugStringsCount = 0;
    g_debugPointsCount  = 0;
    g_debugLinesCount   = 0;

    setupGlyphTexture();
}

void shutdown()
{
    //
    // If this macro is defined, the user-provided ddStr type
    // needs some extra cleanup before shutdown, so we run for
    // all entries in the g_debugStrings[] array.
    //
    // We could call std::string::clear() here, but clear()
    // doesn't deallocate memory in std string, so we might
    // as well let the default destructor do the cleanup,
    // when using the default (AKA std::string) ddStr.
    //
    #ifdef DEBUG_DRAW_STR_DEALLOC_FUNC
    for (int i = 0; i < DD_ARRAY_LEN(g_debugStrings); ++i)
    {
        DEBUG_DRAW_STR_DEALLOC_FUNC(g_debugStrings[i].text);
    }
    #endif // DEBUG_DRAW_STR_DEALLOC_FUNC

    if (g_renderInterface != DD_NULL && g_glyphTex != DD_NULL)
    {
        g_renderInterface->destroyGlyphTexture(g_glyphTex);
        g_glyphTex = DD_NULL;
    }

    g_renderInterface = DD_NULL;
}

bool hasPendingDraws()
{
    return (g_debugStringsCount + g_debugPointsCount + g_debugLinesCount) > 0;
}

void flush(const ddI64 currTimeMillis, const int flags)
{
    DD_CHECK_INIT;

    if (!hasPendingDraws())
    {
        return;
    }

    // Save the last know time value for next dd::line/dd::point calls.
    g_currentTimeMillis = currTimeMillis;

    // Let the user set common render states...
    g_renderInterface->beginDraw();

    // Issue the render calls:
    if (flags & FlushLines)  { drawDebugLines();   }
    if (flags & FlushPoints) { drawDebugPoints();  }
    if (flags & FlushText)   { drawDebugStrings(); }

    // And cleanup if needed...
    g_renderInterface->endDraw();

    // Remove all expired objects, regardless of draw flags:
    clearDebugQueue(g_debugStrings, g_debugStringsCount);
    clearDebugQueue(g_debugPoints,  g_debugPointsCount);
    clearDebugQueue(g_debugLines,   g_debugLinesCount);
}

void clear()
{
    DD_CHECK_INIT;

    // Let the user cleanup the debug strings:
    #ifdef DEBUG_DRAW_STR_DEALLOC_FUNC
    for (int i = 0; i < DD_ARRAY_LEN(g_debugStrings); ++i)
    {
        DEBUG_DRAW_STR_DEALLOC_FUNC(g_debugStrings[i].text);
    }
    #endif // DEBUG_DRAW_STR_DEALLOC_FUNC

    g_vertexBufferUsed  = 0;
    g_debugStringsCount = 0;
    g_debugPointsCount  = 0;
    g_debugLinesCount   = 0;
}

void point(ddVec3Param pos, ddVec3Param color, const float size, const int durationMillis, const bool depthEnabled)
{
    DD_CHECK_INIT;

    if (g_debugPointsCount == DEBUG_DRAW_MAX_POINTS)
    {
        DEBUG_DRAW_OVERFLOWED("DEBUG_DRAW_MAX_POINTS limit reached! Dropping further debug point draws.");
        return;
    }

    DebugPoint & point     = g_debugPoints[g_debugPointsCount++];
    point.expiryDateMillis = g_currentTimeMillis + durationMillis;
    point.depthEnabled     = depthEnabled;
    point.size             = size;

    vecCopy(point.position, pos);
    vecCopy(point.color, color);
}

void line(ddVec3Param from, ddVec3Param to, ddVec3Param color, const int durationMillis, const bool depthEnabled)
{
    DD_CHECK_INIT;

    if (g_debugLinesCount == DEBUG_DRAW_MAX_LINES)
    {
        DEBUG_DRAW_OVERFLOWED("DEBUG_DRAW_MAX_LINES limit reached! Dropping further debug line draws.");
        return;
    }

    DebugLine & line      = g_debugLines[g_debugLinesCount++];
    line.expiryDateMillis = g_currentTimeMillis + durationMillis;
    line.depthEnabled     = depthEnabled;

    vecCopy(line.posFrom, from);
    vecCopy(line.posTo, to);
    vecCopy(line.color, color);
}

void screenText(ddStrParam str, ddVec3Param pos, ddVec3Param color, const float scaling, const int durationMillis)
{
    DD_CHECK_INIT;
    if (g_glyphTex == DD_NULL)
    {
        return;
    }

    if (g_debugStringsCount == DEBUG_DRAW_MAX_STRINGS)
    {
        DEBUG_DRAW_OVERFLOWED("DEBUG_DRAW_MAX_STRINGS limit reached! Dropping further debug string draws.");
        return;
    }

    DebugString & dstr    = g_debugStrings[g_debugStringsCount++];
    dstr.expiryDateMillis = g_currentTimeMillis + durationMillis;
    dstr.posX             = pos[X];
    dstr.posY             = pos[Y];
    dstr.scaling          = scaling;
    dstr.text             = DD_MOVE(str);
    dstr.centered         = false;
    vecCopy(dstr.color, color);
}

void projectedText(ddStrParam str, ddVec3Param pos, ddVec3Param color, ddMat4x4Param vpMatrix,
                   const int sx, const int sy, const int sw, const int sh, const float scaling,
                   const int durationMillis)
{
    DD_CHECK_INIT;
    if (g_glyphTex == DD_NULL)
    {
        return;
    }

    if (g_debugStringsCount == DEBUG_DRAW_MAX_STRINGS)
    {
        DEBUG_DRAW_OVERFLOWED("DEBUG_DRAW_MAX_STRINGS limit reached! Dropping further debug string draws.");
        return;
    }

    float tempPoint[4];
    matTransformPointXYZW(tempPoint, pos, vpMatrix);

    // Bail if W ended up as zero.
    if (DD_FABS(tempPoint[W]) < DD_EPSILON)
    {
        return;
    }

    // Perspective divide (we only care about the 2D part now):
    tempPoint[X] /= tempPoint[W];
    tempPoint[Y] /= tempPoint[W];

    // Map to window coordinates:
    float scrX = ((tempPoint[X] * 0.5f) + 0.5f) * sw + sx;
    float scrY = ((tempPoint[Y] * 0.5f) + 0.5f) * sh + sy;

    // Need to invert the direction because on OGL the screen origin is the bottom-left corner.
    // NOTE: This is not renderer agnostic, I think... Should add a #define or something!
    scrY = static_cast<float>(sh) - scrY;

    DebugString & dstr    = g_debugStrings[g_debugStringsCount++];
    dstr.expiryDateMillis = g_currentTimeMillis + durationMillis;
    dstr.posX             = scrX;
    dstr.posY             = scrY;
    dstr.scaling          = scaling;
    dstr.text             = DD_MOVE(str);
    dstr.centered         = true;
    vecCopy(dstr.color, color);
}

void axisTriad(ddMat4x4Param transform, const float size, const float length,
               const int durationMillis, const bool depthEnabled)
{
    DD_CHECK_INIT;

    ddVec3 p0, p1, p2, p3;
    ddVec3 xEnd, yEnd, zEnd;
    ddVec3 origin, cR, cG, cB;

    vecSet(cR, 1.0f, 0.0f, 0.0f);
    vecSet(cG, 0.0f, 1.0f, 0.0f);
    vecSet(cB, 0.0f, 0.0f, 1.0f);

    vecSet(origin, 0.0f, 0.0f, 0.0f);
    vecSet(xEnd, length, 0.0f, 0.0f);
    vecSet(yEnd, 0.0f, length, 0.0f);
    vecSet(zEnd, 0.0f, 0.0f, length);

    matTransformPointXYZ(p0, origin, transform);
    matTransformPointXYZ(p1, xEnd, transform);
    matTransformPointXYZ(p2, yEnd, transform);
    matTransformPointXYZ(p3, zEnd, transform);

    arrow(p0, p1, cR, size, durationMillis, depthEnabled); // X: red axis
    arrow(p0, p2, cG, size, durationMillis, depthEnabled); // Y: green axis
    arrow(p0, p3, cB, size, durationMillis, depthEnabled); // Z: blue axis
}

void arrow(ddVec3Param from, ddVec3Param to, ddVec3Param color, const float size,
           const int durationMillis, const bool depthEnabled)
{
    DD_CHECK_INIT;

    static const float arrowStep = 30.0f; // In degrees
    static bool sinCosTablesComputed = false;
    static float arrowSin[45];
    static float arrowCos[45];

    // Calculate sine and cosine tables only once:
    if (!sinCosTablesComputed)
    {
        int i = 0;
        float degrees = 0.0f;
        for (; degrees < 360.0f; degrees += arrowStep, ++i)
        {
            arrowSin[i] = DD_FSIN(DD_DEG2RAD(degrees));
            arrowCos[i] = DD_FCOS(DD_DEG2RAD(degrees));
        }

        arrowSin[i] = arrowSin[0];
        arrowCos[i] = arrowCos[0];
        sinCosTablesComputed = true;
    }

    // Body line:
    line(from, to, color, durationMillis, depthEnabled);

    // Aux vectors to compute the arrowhead:
    ddVec3 up, right, forward;
    vecSub(forward, to, from);
    vecNormalize(forward, forward);
    vecOrthogonalBasis(right, up, forward);
    vecScale(forward, forward, size);

    // Arrowhead is a cone (sin/cos tables used here):
    float degrees = 0.0f;
    for (int i = 0; degrees < 360.0f; degrees += arrowStep, ++i)
    {
        float scale;
        ddVec3 v1, v2, temp;

        scale = 0.5f * size * arrowCos[i];
        vecScale(temp, right, scale);
        vecSub(v1, to, forward);
        vecAdd(v1, v1, temp);

        scale = 0.5f * size * arrowSin[i];
        vecScale(temp, up, scale);
        vecAdd(v1, v1, temp);

        scale = 0.5f * size * arrowCos[i + 1];
        vecScale(temp, right, scale);
        vecSub(v2, to, forward);
        vecAdd(v2, v2, temp);

        scale = 0.5f * size * arrowSin[i + 1];
        vecScale(temp, up, scale);
        vecAdd(v2, v2, temp);

        line(v1, to, color, durationMillis, depthEnabled);
        line(v1, v2, color, durationMillis, depthEnabled);
    }
}

void cross(ddVec3Param center, const float length, const int durationMillis, const bool depthEnabled)
{
    DD_CHECK_INIT;

    ddVec3 from, to;
    ddVec3 cR, cG, cB;

    vecSet(cR, 1.0f, 0.0f, 0.0f);
    vecSet(cG, 0.0f, 1.0f, 0.0f);
    vecSet(cB, 0.0f, 0.0f, 1.0f);

    const float cx = center[X];
    const float cy = center[Y];
    const float cz = center[Z];
    const float hl = length * 0.5f; // Half on each side.

    // Red line: X - length/2 to X + length/2
    vecSet(from, cx - hl, cy, cz);
    vecSet(to,   cx + hl, cy, cz);
    line(from, to, cR, durationMillis, depthEnabled);

    // Green line: Y - length/2 to Y + length/2
    vecSet(from, cx, cy - hl, cz);
    vecSet(to,   cx, cy + hl, cz);
    line(from, to, cG, durationMillis, depthEnabled);

    // Blue line: Z - length/2 to Z + length/2
    vecSet(from, cx, cy, cz - hl);
    vecSet(to,   cx, cy, cz + hl);
    line(from, to, cB, durationMillis, depthEnabled);
}

void circle(ddVec3Param center, ddVec3Param planeNormal, ddVec3Param color, const float radius,
            const float numSteps, const int durationMillis, const bool depthEnabled)
{
    DD_CHECK_INIT;

    ddVec3 left, up;
    ddVec3 point, lastPoint;

    vecOrthogonalBasis(left, up, planeNormal);

    vecScale(up, up, radius);
    vecScale(left, left, radius);
    vecAdd(lastPoint, center, up);

    for (int i = 1; i <= numSteps; ++i)
    {
        const float radians = DD_TAU * i / numSteps;

        ddVec3 vs, vc;
        vecScale(vs, left, DD_FSIN(radians));
        vecScale(vc, up,   DD_FCOS(radians));

        vecAdd(point, center, vs);
        vecAdd(point, point,  vc);

        line(lastPoint, point, color, durationMillis, depthEnabled);
        vecCopy(lastPoint, point);
    }
}

void plane(ddVec3Param center, ddVec3Param planeNormal, ddVec3Param planeColor, ddVec3Param normalVecColor,
           const float planeScale, const float normalVecScale, const int durationMillis, const bool depthEnabled)
{
    DD_CHECK_INIT;

    ddVec3 v1, v2, v3, v4;
    ddVec3 tangent, bitangent;
    vecOrthogonalBasis(tangent, bitangent, planeNormal);

    // A little bit of preprocessor voodoo to make things more interesting :P
    #define DD_PLANE_V(v, op1, op2) \
    v[X] = (center[X] op1 (tangent[X] * planeScale) op2 (bitangent[X] * planeScale)); \
    v[Y] = (center[Y] op1 (tangent[Y] * planeScale) op2 (bitangent[Y] * planeScale)); \
    v[Z] = (center[Z] op1 (tangent[Z] * planeScale) op2 (bitangent[Z] * planeScale))
    DD_PLANE_V(v1, -, -);
    DD_PLANE_V(v2, +, -);
    DD_PLANE_V(v3, +, +);
    DD_PLANE_V(v4, -, +);
    #undef DD_PLANE_V

    // Draw the wireframe plane quadrilateral:
    line(v1, v2, planeColor, durationMillis, depthEnabled);
    line(v2, v3, planeColor, durationMillis, depthEnabled);
    line(v3, v4, planeColor, durationMillis, depthEnabled);
    line(v4, v1, planeColor, durationMillis, depthEnabled);

    // Optionally add a line depicting the plane normal:
    if (normalVecScale != 0.0f)
    {
        ddVec3 normalVec;
        normalVec[X] = (planeNormal[X] * normalVecScale) + center[X];
        normalVec[Y] = (planeNormal[Y] * normalVecScale) + center[Y];
        normalVec[Z] = (planeNormal[Z] * normalVecScale) + center[Z];
        line(center, normalVec, normalVecColor, durationMillis, depthEnabled);
    }
}

void sphere(ddVec3Param center, ddVec3Param color, const float radius, const int durationMillis, const bool depthEnabled)
{
    DD_CHECK_INIT;

    static const int stepSize = 15;
    ddVec3 cache[360 / stepSize];
    ddVec3 radiusVec;

    vecSet(radiusVec, 0.0f, 0.0f, radius);
    vecAdd(cache[0], center, radiusVec);

    for (int n = 1; n < DD_ARRAY_LEN(cache); ++n)
    {
        vecCopy(cache[n], cache[0]);
    }

    ddVec3 lastPoint, temp;
    for (int i = stepSize; i <= 360; i += stepSize)
    {
        const float s = DD_FSIN(DD_DEG2RAD(i));
        const float c = DD_FCOS(DD_DEG2RAD(i));

        lastPoint[X] = center[X];
        lastPoint[Y] = center[Y] + radius * s;
        lastPoint[Z] = center[Z] + radius * c;

        for (int n = 0, j = stepSize; j <= 360; j += stepSize, ++n)
        {
            temp[X] = center[X] + DD_FSIN(DD_DEG2RAD(j)) * radius * s;
            temp[Y] = center[Y] + DD_FCOS(DD_DEG2RAD(j)) * radius * s;
            temp[Z] = lastPoint[Z];

            line(lastPoint, temp, color, durationMillis, depthEnabled);
            line(lastPoint, cache[n], color, durationMillis, depthEnabled);

            vecCopy(cache[n], lastPoint);
            vecCopy(lastPoint, temp);
        }
    }
}

void cone(ddVec3Param apex, ddVec3Param dir, ddVec3Param color, const float baseRadius,
          const float apexRadius, const int durationMillis, const bool depthEnabled)
{
    DD_CHECK_INIT;

    static const int stepSize = 20;
    ddVec3 axis[3];
    ddVec3 top, temp0, temp1, temp2;
    ddVec3 p1, p2, lastP1, lastP2;

    vecCopy(axis[2], dir);
    vecNormalize(axis[2], axis[2]);
    vecOrthogonalBasis(axis[0], axis[1], axis[2]);

    axis[1][X] = -axis[1][X];
    axis[1][Y] = -axis[1][Y];
    axis[1][Z] = -axis[1][Z];

    vecAdd(top, apex, dir);
    vecScale(temp1, axis[1], baseRadius);
    vecAdd(lastP2, top, temp1);

    if (apexRadius == 0.0f)
    {
        for (int i = stepSize; i <= 360; i += stepSize)
        {
            vecScale(temp1, axis[0], DD_FSIN(DD_DEG2RAD(i)));
            vecScale(temp2, axis[1], DD_FCOS(DD_DEG2RAD(i)));
            vecAdd(temp0, temp1, temp2);

            vecScale(temp0, temp0, baseRadius);
            vecAdd(p2, top, temp0);

            line(lastP2, p2, color, durationMillis, depthEnabled);
            line(p2, apex, color, durationMillis, depthEnabled);

            vecCopy(lastP2, p2);
        }
    }
    else // A degenerate cone with open apex:
    {
        vecScale(temp1, axis[1], apexRadius);
        vecAdd(lastP1, apex, temp1);

        for (int i = stepSize; i <= 360; i += stepSize)
        {
            vecScale(temp1, axis[0], DD_FSIN(DD_DEG2RAD(i)));
            vecScale(temp2, axis[1], DD_FCOS(DD_DEG2RAD(i)));
            vecAdd(temp0, temp1, temp2);

            vecScale(temp1, temp0, apexRadius);
            vecScale(temp2, temp0, baseRadius);

            vecAdd(p1, apex, temp1);
            vecAdd(p2, top,  temp2);

            line(lastP1, p1, color, durationMillis, depthEnabled);
            line(lastP2, p2, color, durationMillis, depthEnabled);
            line(p1, p2, color, durationMillis, depthEnabled);

            vecCopy(lastP1, p1);
            vecCopy(lastP2, p2);
        }
    }
}

void box(const ddVec3 points[8], ddVec3Param color, const int durationMillis, const bool depthEnabled)
{
    // Build the lines from points using clever indexing tricks:
    // (& 3 is a fancy way of doing % 4, but avoids the expensive modulo operation)
    for (int i = 0; i < 4; ++i)
    {
        line(points[i], points[(i + 1) & 3], color, durationMillis, depthEnabled);
        line(points[4 + i], points[4 + ((i + 1) & 3)], color, durationMillis, depthEnabled);
        line(points[i], points[4 + i], color, durationMillis, depthEnabled);
    }
}

void box(ddVec3Param center, ddVec3Param color, const float width, const float height,
         const float depth, const int durationMillis, const bool depthEnabled)
{
    DD_CHECK_INIT;

    const float cx = center[X];
    const float cy = center[Y];
    const float cz = center[Z];
    const float w  = width  * 0.5f;
    const float h  = height * 0.5f;
    const float d  = depth  * 0.5f;

    // Create all the 8 points:
    ddVec3 points[8];
    #define DD_BOX_V(v, op1, op2, op3) \
    v[X] = cx op1 w; \
    v[Y] = cy op2 h; \
    v[Z] = cz op3 d
    DD_BOX_V(points[0], -, +, +);
    DD_BOX_V(points[1], -, +, -);
    DD_BOX_V(points[2], +, +, -);
    DD_BOX_V(points[3], +, +, +);
    DD_BOX_V(points[4], -, -, +);
    DD_BOX_V(points[5], -, -, -);
    DD_BOX_V(points[6], +, -, -);
    DD_BOX_V(points[7], +, -, +);
    #undef DD_BOX_V

    box(points, color, durationMillis, depthEnabled);
}

void aabb(ddVec3Param mins, ddVec3Param maxs, ddVec3Param color, const int durationMillis, const bool depthEnabled)
{
    DD_CHECK_INIT;

    ddVec3 bb[2];
    ddVec3 points[8];

    vecCopy(bb[0], mins);
    vecCopy(bb[1], maxs);

    // Expand min/max bounds:
    for (int i = 0; i < DD_ARRAY_LEN(points); ++i)
    {
        points[i][X] = bb[(i ^ (i >> 1)) & 1][X];
        points[i][Y] = bb[(i >> 1) & 1][Y];
        points[i][Z] = bb[(i >> 2) & 1][Z];
    }

    // Build the lines:
    box(points, color, durationMillis, depthEnabled);
}

void frustum(ddMat4x4Param invClipMatrix, ddVec3Param color, const int durationMillis, const bool depthEnabled)
{
    DD_CHECK_INIT;

    // Start with the standard clip volume, then bring it back to world space.
    static const float planes[8][3] = {
        // near plane
        { -1.0f, -1.0f, -1.0f }, {  1.0f, -1.0f, -1.0f },
        {  1.0f,  1.0f, -1.0f }, { -1.0f,  1.0f, -1.0f },
        // far plane
        { -1.0f, -1.0f,  1.0f }, {  1.0f, -1.0f,  1.0f },
        {  1.0f,  1.0f,  1.0f }, { -1.0f,  1.0f,  1.0f }
    };

    ddVec3 points[8];
    float wCoords[8];

    // Transform the planes by the inverse clip matrix:
    for (int i = 0; i < DD_ARRAY_LEN(planes); ++i)
    {
        wCoords[i] = matTransformPointXYZW2(points[i], planes[i], invClipMatrix);
    }

    // Divide by the W component of each:
    for (int i = 0; i < DD_ARRAY_LEN(planes); ++i)
    {
        // But bail if any W ended up as zero.
        if (DD_FABS(wCoords[W]) < DD_EPSILON)
        {
            return;
        }

        points[i][X] /= wCoords[i];
        points[i][Y] /= wCoords[i];
        points[i][Z] /= wCoords[i];
    }

    // Connect the dots:
    box(points, color, durationMillis, depthEnabled);
}

void vertexNormal(ddVec3Param origin, ddVec3Param normal, const float length,
                  const int durationMillis, const bool depthEnabled)
{
    DD_CHECK_INIT;

    ddVec3 normalVec;
    ddVec3 normalColor;

    vecSet(normalColor, 1.0f, 1.0f, 1.0f);

    normalVec[X] = (normal[X] * length) + origin[X];
    normalVec[Y] = (normal[Y] * length) + origin[Y];
    normalVec[Z] = (normal[Z] * length) + origin[Z];

    line(origin, normalVec, normalColor, durationMillis, depthEnabled);
}

void tangentBasis(ddVec3Param origin, ddVec3Param normal, ddVec3Param tangent, ddVec3Param bitangent,
                  const float lengths, const int durationMillis, const bool depthEnabled)
{
    DD_CHECK_INIT;

    ddVec3 cN, cT, cB;
    ddVec3 vN, vT, vB;

    vecSet(cN, 1.0f, 1.0f, 1.0f); // Vertex normals are WHITE
    vecSet(cT, 1.0f, 1.0f, 0.0f); // Tangents are YELLOW
    vecSet(cB, 1.0f, 0.0f, 1.0f); // Bi-tangents are MAGENTA

    vN[X] = (normal[X] * lengths) + origin[X];
    vN[Y] = (normal[Y] * lengths) + origin[Y];
    vN[Z] = (normal[Z] * lengths) + origin[Z];

    vT[X] = (tangent[X] * lengths) + origin[X];
    vT[Y] = (tangent[Y] * lengths) + origin[Y];
    vT[Z] = (tangent[Z] * lengths) + origin[Z];

    vB[X] = (bitangent[X] * lengths) + origin[X];
    vB[Y] = (bitangent[Y] * lengths) + origin[Y];
    vB[Z] = (bitangent[Z] * lengths) + origin[Z];

    line(origin, vN, cN, durationMillis, depthEnabled);
    line(origin, vT, cT, durationMillis, depthEnabled);
    line(origin, vB, cB, durationMillis, depthEnabled);
}

void xzSquareGrid(const float mins, const float maxs, const float y, const float step,
                  ddVec3Param color, const int durationMillis, const bool depthEnabled)
{
    DD_CHECK_INIT;

    ddVec3 from, to;
    for (float i = mins; i <= maxs; i += step)
    {
        // Horizontal line (along the X)
        vecSet(from, mins, y, i);
        vecSet(to,   maxs, y, i);
        line(from, to, color, durationMillis, depthEnabled);

        // Vertical line (along the Z)
        vecSet(from, i, y, mins);
        vecSet(to,   i, y, maxs);
        line(from, to, color, durationMillis, depthEnabled);
    }
}

// ========================================================
// RenderInterface stubs:
// ========================================================

RenderInterface::~RenderInterface() { }
void RenderInterface::beginDraw() { }
void RenderInterface::endDraw() { }
void RenderInterface::drawPointList(const DrawVertex *, int, bool) { }
void RenderInterface::drawLineList(const DrawVertex *, int, bool) { }
void RenderInterface::drawGlyphList(const DrawVertex *, int, GlyphTextureHandle) { }
void RenderInterface::destroyGlyphTexture(GlyphTextureHandle) { }
GlyphTextureHandle RenderInterface::createGlyphTexture(int, int, const void *) { return DD_NULL; }

} // namespace dd {}

// Cleanup the local macros:
#undef DD_NULL
#undef DD_MOVE
#undef DD_FSIN
#undef DD_FCOS
#undef DD_FABS
#undef DD_INV_FSQRT
#undef DD_PI
#undef DD_TAU
#undef DD_EPSILON
#undef DD_DEG2RAD
#undef DD_ARRAY_LEN
#undef DD_CHECK_INIT
#undef DD_MALLOC
#undef DD_MFREE

// Restore the warnings we have selectively silenced in the implementation:
#ifdef __clang__
    #pragma clang diagnostic pop
#endif // __clang__

// ================ End of implementation =================
#endif // DEBUG_DRAW_IMPLEMENTATION
// ================ End of implementation =================
