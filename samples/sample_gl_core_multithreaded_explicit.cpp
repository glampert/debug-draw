
// ================================================================================================
// -*- C++ -*-
// File:   sample_gl_core_multithreaded_explicit.cpp
// Author: Guilherme R. Lampert
// Brief:  Debug Draw usage sample with Core Profile OpenGL and separate rendering thread.
//         Demonstrates the use of DEBUG_DRAW_EXPLICIT_CONTEXT with threads/async calls
//         and multiple contexts.
//
// This software is in the public domain. Where that dedication is not recognized,
// you are granted a perpetual, irrevocable license to copy, distribute, and modify
// this file as you see fit.
// ================================================================================================

#define DEBUG_DRAW_IMPLEMENTATION
#define DEBUG_DRAW_EXPLICIT_CONTEXT

#include "debug_draw.hpp"     // Debug Draw API. Notice that we need the DEBUG_DRAW_IMPLEMENTATION macro here!
#include <GL/gl3w.h>          // An OpenGL extension wrangler (https://github.com/skaslev/gl3w).
#include "samples_common.hpp" // Common code shared by the samples (camera, input, etc).

using namespace ddSamplesCommon;

// ========================================================
// Debug Draw RenderInterface for Core OpenGL:
// ========================================================

class DDRenderInterfaceCoreGL final
    : public dd::RenderInterface
{
public:

    //
    // dd::RenderInterface overrides:
    //

    void drawPointList(const dd::DrawVertex * points, int count, bool depthEnabled) override
    {
        assert(points != nullptr);
        assert(count > 0 && count <= DEBUG_DRAW_VERTEX_BUFFER_SIZE);
        assert(isMainThread()); // GL calls from the main thread only!

        glBindVertexArray(linePointVAO);
        glUseProgram(linePointProgram);

        glUniformMatrix4fv(linePointProgram_MvpMatrixLocation,
                           1, GL_FALSE, toFloatPtr(mvpMatrix));

        if (depthEnabled)
        {
            glEnable(GL_DEPTH_TEST);
        }
        else
        {
            glDisable(GL_DEPTH_TEST);
        }

        // NOTE: Could also use glBufferData to take advantage of the buffer orphaning trick...
        glBindBuffer(GL_ARRAY_BUFFER, linePointVBO);
        glBufferSubData(GL_ARRAY_BUFFER, 0, count * sizeof(dd::DrawVertex), points);

        // Issue the draw call:
        glDrawArrays(GL_POINTS, 0, count);

        glUseProgram(0);
        glBindVertexArray(0);
        glBindBuffer(GL_ARRAY_BUFFER, 0);
        checkGLError(__FILE__, __LINE__);
    }

    void drawLineList(const dd::DrawVertex * lines, int count, bool depthEnabled) override
    {
        assert(lines != nullptr);
        assert(count > 0 && count <= DEBUG_DRAW_VERTEX_BUFFER_SIZE);
        assert(isMainThread()); // GL calls from the main thread only!

        glBindVertexArray(linePointVAO);
        glUseProgram(linePointProgram);

        glUniformMatrix4fv(linePointProgram_MvpMatrixLocation,
                           1, GL_FALSE, toFloatPtr(mvpMatrix));

        if (depthEnabled)
        {
            glEnable(GL_DEPTH_TEST);
        }
        else
        {
            glDisable(GL_DEPTH_TEST);
        }

        // NOTE: Could also use glBufferData to take advantage of the buffer orphaning trick...
        glBindBuffer(GL_ARRAY_BUFFER, linePointVBO);
        glBufferSubData(GL_ARRAY_BUFFER, 0, count * sizeof(dd::DrawVertex), lines);

        // Issue the draw call:
        glDrawArrays(GL_LINES, 0, count);

        glUseProgram(0);
        glBindVertexArray(0);
        glBindBuffer(GL_ARRAY_BUFFER, 0);
        checkGLError(__FILE__, __LINE__);
    }

    void drawGlyphList(const dd::DrawVertex * glyphs, int count, dd::GlyphTextureHandle glyphTex) override
    {
        assert(glyphs != nullptr);
        assert(count > 0 && count <= DEBUG_DRAW_VERTEX_BUFFER_SIZE);
        assert(isMainThread()); // GL calls from the main thread only!

        glBindVertexArray(textVAO);
        glUseProgram(textProgram);

        // These doesn't have to be reset every draw call, I'm just being lazy ;)
        glUniform1i(textProgram_GlyphTextureLocation, 0);
        glUniform2f(textProgram_ScreenDimensions,
                    static_cast<GLfloat>(WindowWidth),
                    static_cast<GLfloat>(WindowHeight));

        if (glyphTex != nullptr)
        {
            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_2D, handleToGL(glyphTex));
        }

        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        glDisable(GL_DEPTH_TEST);

        glBindBuffer(GL_ARRAY_BUFFER, textVBO);
        glBufferSubData(GL_ARRAY_BUFFER, 0, count * sizeof(dd::DrawVertex), glyphs);

        glDrawArrays(GL_TRIANGLES, 0, count); // Issue the draw call

        glDisable(GL_BLEND);
        glUseProgram(0);
        glBindVertexArray(0);
        glBindBuffer(GL_ARRAY_BUFFER, 0);
        glBindTexture(GL_TEXTURE_2D,  0);
        checkGLError(__FILE__, __LINE__);
    }

    dd::GlyphTextureHandle createGlyphTexture(int width, int height, const void * pixels) override
    {
        assert(width > 0 && height > 0);
        assert(pixels != nullptr);
        assert(isMainThread()); // GL calls from the main thread only!

        GLuint textureId = 0;
        glGenTextures(1, &textureId);
        glBindTexture(GL_TEXTURE_2D, textureId);

        glPixelStorei(GL_PACK_ALIGNMENT,   1);
        glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

        glTexImage2D(GL_TEXTURE_2D, 0, GL_R8, width, height, 0, GL_RED, GL_UNSIGNED_BYTE, pixels);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

        glBindTexture(GL_TEXTURE_2D, 0);
        checkGLError(__FILE__, __LINE__);

        return GLToHandle(textureId);
    }

    void destroyGlyphTexture(dd::GlyphTextureHandle glyphTex) override
    {
        assert(isMainThread()); // GL calls from the main thread only!
        if (glyphTex == nullptr)
        {
            return;
        }

        const GLuint textureId = handleToGL(glyphTex);
        glBindTexture(GL_TEXTURE_2D, 0);
        glDeleteTextures(1, &textureId);
    }

    void beginDraw() override { assert(isMainThread()); }
    void endDraw()   override { assert(isMainThread()); }

    //
    // Local methods:
    //

    DDRenderInterfaceCoreGL()
        : mvpMatrix(Matrix4::identity())
        , linePointProgram(0)
        , linePointProgram_MvpMatrixLocation(-1)
        , textProgram(0)
        , textProgram_GlyphTextureLocation(-1)
        , textProgram_ScreenDimensions(-1)
        , linePointVAO(0)
        , linePointVBO(0)
        , textVAO(0)
        , textVBO(0)
    {
        std::printf("\n");
        std::printf("GL_VENDOR    : %s\n",   glGetString(GL_VENDOR));
        std::printf("GL_RENDERER  : %s\n",   glGetString(GL_RENDERER));
        std::printf("GL_VERSION   : %s\n",   glGetString(GL_VERSION));
        std::printf("GLSL_VERSION : %s\n\n", glGetString(GL_SHADING_LANGUAGE_VERSION));
        std::printf("DDRenderInterfaceCoreGL MT initializing ...\n");

        // Default OpenGL states:
        glEnable(GL_CULL_FACE);
        glEnable(GL_DEPTH_TEST);
        glDisable(GL_BLEND);

        // This has to be enabled since the point drawing shader will use gl_PointSize.
        glEnable(GL_PROGRAM_POINT_SIZE);

        setupShaderPrograms();
        setupVertexBuffers();

        std::printf("DDRenderInterfaceCoreGL MT ready!\n\n");
    }

    ~DDRenderInterfaceCoreGL()
    {
        glDeleteProgram(linePointProgram);
        glDeleteProgram(textProgram);

        glDeleteVertexArrays(1, &linePointVAO);
        glDeleteBuffers(1, &linePointVBO);

        glDeleteVertexArrays(1, &textVAO);
        glDeleteBuffers(1, &textVBO);
    }

    void prepareDraw(const Matrix4 & mvp)
    {
        assert(isMainThread());

        mvpMatrix = mvp;
        glClearColor(0.2f, 0.2f, 0.2f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    }

    void setupShaderPrograms()
    {
        std::printf("> DDRenderInterfaceCoreGL::setupShaderPrograms()\n");

        //
        // Line/point drawing shader:
        //
        {
            GLuint linePointVS = glCreateShader(GL_VERTEX_SHADER);
            glShaderSource(linePointVS, 1, &linePointVertShaderSrc, nullptr);
            compileShader(linePointVS);

            GLint linePointFS = glCreateShader(GL_FRAGMENT_SHADER);
            glShaderSource(linePointFS, 1, &linePointFragShaderSrc, nullptr);
            compileShader(linePointFS);

            linePointProgram = glCreateProgram();
            glAttachShader(linePointProgram, linePointVS);
            glAttachShader(linePointProgram, linePointFS);

            glBindAttribLocation(linePointProgram, 0, "in_Position");
            glBindAttribLocation(linePointProgram, 1, "in_ColorPointSize");
            linkProgram(linePointProgram);

            linePointProgram_MvpMatrixLocation = glGetUniformLocation(linePointProgram, "u_MvpMatrix");
            if (linePointProgram_MvpMatrixLocation < 0)
            {
                errorF("Unable to get u_MvpMatrix uniform location!");
            }
            checkGLError(__FILE__, __LINE__);
        }

        //
        // Text rendering shader:
        //
        {
            GLuint textVS = glCreateShader(GL_VERTEX_SHADER);
            glShaderSource(textVS, 1, &textVertShaderSrc, nullptr);
            compileShader(textVS);

            GLint textFS = glCreateShader(GL_FRAGMENT_SHADER);
            glShaderSource(textFS, 1, &textFragShaderSrc, nullptr);
            compileShader(textFS);

            textProgram = glCreateProgram();
            glAttachShader(textProgram, textVS);
            glAttachShader(textProgram, textFS);

            glBindAttribLocation(textProgram, 0, "in_Position");
            glBindAttribLocation(textProgram, 1, "in_TexCoords");
            glBindAttribLocation(textProgram, 2, "in_Color");
            linkProgram(textProgram);

            textProgram_GlyphTextureLocation = glGetUniformLocation(textProgram, "u_glyphTexture");
            if (textProgram_GlyphTextureLocation < 0)
            {
                errorF("Unable to get u_glyphTexture uniform location!");
            }

            textProgram_ScreenDimensions = glGetUniformLocation(textProgram, "u_screenDimensions");
            if (textProgram_ScreenDimensions < 0)
            {
                errorF("Unable to get u_screenDimensions uniform location!");
            }

            checkGLError(__FILE__, __LINE__);
        }
    }

    void setupVertexBuffers()
    {
        std::printf("> DDRenderInterfaceCoreGL::setupVertexBuffers()\n");

        //
        // Lines/points vertex buffer:
        //
        {
            glGenVertexArrays(1, &linePointVAO);
            glGenBuffers(1, &linePointVBO);
            checkGLError(__FILE__, __LINE__);

            glBindVertexArray(linePointVAO);
            glBindBuffer(GL_ARRAY_BUFFER, linePointVBO);

            // RenderInterface will never be called with a batch larger than
            // DEBUG_DRAW_VERTEX_BUFFER_SIZE vertexes, so we can allocate the same amount here.
            glBufferData(GL_ARRAY_BUFFER, DEBUG_DRAW_VERTEX_BUFFER_SIZE * sizeof(dd::DrawVertex), nullptr, GL_STREAM_DRAW);
            checkGLError(__FILE__, __LINE__);

            // Set the vertex format expected by 3D points and lines:
            std::size_t offset = 0;

            glEnableVertexAttribArray(0); // in_Position (vec3)
            glVertexAttribPointer(
                /* index     = */ 0,
                /* size      = */ 3,
                /* type      = */ GL_FLOAT,
                /* normalize = */ GL_FALSE,
                /* stride    = */ sizeof(dd::DrawVertex),
                /* offset    = */ reinterpret_cast<void *>(offset));
            offset += sizeof(float) * 3;

            glEnableVertexAttribArray(1); // in_ColorPointSize (vec4)
            glVertexAttribPointer(
                /* index     = */ 1,
                /* size      = */ 4,
                /* type      = */ GL_FLOAT,
                /* normalize = */ GL_FALSE,
                /* stride    = */ sizeof(dd::DrawVertex),
                /* offset    = */ reinterpret_cast<void *>(offset));

            checkGLError(__FILE__, __LINE__);

            // VAOs can be a pain in the neck if left enabled...
            glBindVertexArray(0);
            glBindBuffer(GL_ARRAY_BUFFER, 0);
        }

        //
        // Text rendering vertex buffer:
        //
        {
            glGenVertexArrays(1, &textVAO);
            glGenBuffers(1, &textVBO);
            checkGLError(__FILE__, __LINE__);

            glBindVertexArray(textVAO);
            glBindBuffer(GL_ARRAY_BUFFER, textVBO);

            // NOTE: A more optimized implementation might consider combining
            // both the lines/points and text buffers to save some memory!
            glBufferData(GL_ARRAY_BUFFER, DEBUG_DRAW_VERTEX_BUFFER_SIZE * sizeof(dd::DrawVertex), nullptr, GL_STREAM_DRAW);
            checkGLError(__FILE__, __LINE__);

            // Set the vertex format expected by the 2D text:
            std::size_t offset = 0;

            glEnableVertexAttribArray(0); // in_Position (vec2)
            glVertexAttribPointer(
                /* index     = */ 0,
                /* size      = */ 2,
                /* type      = */ GL_FLOAT,
                /* normalize = */ GL_FALSE,
                /* stride    = */ sizeof(dd::DrawVertex),
                /* offset    = */ reinterpret_cast<void *>(offset));
            offset += sizeof(float) * 2;

            glEnableVertexAttribArray(1); // in_TexCoords (vec2)
            glVertexAttribPointer(
                /* index     = */ 1,
                /* size      = */ 2,
                /* type      = */ GL_FLOAT,
                /* normalize = */ GL_FALSE,
                /* stride    = */ sizeof(dd::DrawVertex),
                /* offset    = */ reinterpret_cast<void *>(offset));
            offset += sizeof(float) * 2;

            glEnableVertexAttribArray(2); // in_Color (vec4)
            glVertexAttribPointer(
                /* index     = */ 2,
                /* size      = */ 4,
                /* type      = */ GL_FLOAT,
                /* normalize = */ GL_FALSE,
                /* stride    = */ sizeof(dd::DrawVertex),
                /* offset    = */ reinterpret_cast<void *>(offset));

            checkGLError(__FILE__, __LINE__);

            // Ditto.
            glBindVertexArray(0);
            glBindBuffer(GL_ARRAY_BUFFER, 0);
        }
    }

    static GLuint handleToGL(dd::GlyphTextureHandle handle)
    {
        const std::size_t temp = reinterpret_cast<std::size_t>(handle);
        return static_cast<GLuint>(temp);
    }

    static dd::GlyphTextureHandle GLToHandle(const GLuint id)
    {
        const std::size_t temp = static_cast<std::size_t>(id);
        return reinterpret_cast<dd::GlyphTextureHandle>(temp);
    }

    static void checkGLError(const char * file, const int line)
    {
        GLenum err;
        while ((err = glGetError()) != GL_NO_ERROR)
        {
            errorF("%s(%d) : GL_CORE_ERROR=0x%X - %s", file, line, err, errorToString(err));
        }
    }

    static void compileShader(const GLuint shader)
    {
        glCompileShader(shader);
        checkGLError(__FILE__, __LINE__);

        GLint status;
        glGetShaderiv(shader, GL_COMPILE_STATUS, &status);
        checkGLError(__FILE__, __LINE__);

        if (status == GL_FALSE)
        {
            GLchar strInfoLog[1024] = {0};
            glGetShaderInfoLog(shader, sizeof(strInfoLog) - 1, nullptr, strInfoLog);
            errorF("\n>>> Shader compiler errors:\n%s", strInfoLog);
        }
    }

    static void linkProgram(const GLuint program)
    {
        glLinkProgram(program);
        checkGLError(__FILE__, __LINE__);

        GLint status;
        glGetProgramiv(program, GL_LINK_STATUS, &status);
        checkGLError(__FILE__, __LINE__);

        if (status == GL_FALSE)
        {
            GLchar strInfoLog[1024] = {0};
            glGetProgramInfoLog(program, sizeof(strInfoLog) - 1, nullptr, strInfoLog);
            errorF("\n>>> Program linker errors:\n%s", strInfoLog);
        }
    }

private:

    // The "model-view-projection" matrix for the scene.
    // In this demo, it consists of the camera's view and projection matrices only.
    Matrix4 mvpMatrix;

    GLuint linePointProgram;
    GLint  linePointProgram_MvpMatrixLocation;

    GLuint textProgram;
    GLint  textProgram_GlyphTextureLocation;
    GLint  textProgram_ScreenDimensions;

    GLuint linePointVAO;
    GLuint linePointVBO;

    GLuint textVAO;
    GLuint textVBO;

    static const char * linePointVertShaderSrc;
    static const char * linePointFragShaderSrc;

    static const char * textVertShaderSrc;
    static const char * textFragShaderSrc;

}; // class DDRenderInterfaceCoreGL

// ========================================================
// Minimal shaders we need for the debug primitives:
// ========================================================

const char * DDRenderInterfaceCoreGL::linePointVertShaderSrc = "\n"
    "#version 150\n"
    "\n"
    "in vec3 in_Position;\n"
    "in vec4 in_ColorPointSize;\n"
    "\n"
    "out vec4 v_Color;\n"
    "uniform mat4 u_MvpMatrix;\n"
    "\n"
    "void main()\n"
    "{\n"
    "    gl_Position  = u_MvpMatrix * vec4(in_Position, 1.0);\n"
    "    gl_PointSize = in_ColorPointSize.w;\n"
    "    v_Color      = vec4(in_ColorPointSize.xyz, 1.0);\n"
    "}\n";

const char * DDRenderInterfaceCoreGL::linePointFragShaderSrc = "\n"
    "#version 150\n"
    "\n"
    "in  vec4 v_Color;\n"
    "out vec4 out_FragColor;\n"
    "\n"
    "void main()\n"
    "{\n"
    "    out_FragColor = v_Color;\n"
    "}\n";

const char * DDRenderInterfaceCoreGL::textVertShaderSrc = "\n"
    "#version 150\n"
    "\n"
    "in vec2 in_Position;\n"
    "in vec2 in_TexCoords;\n"
    "in vec3 in_Color;\n"
    "\n"
    "uniform vec2 u_screenDimensions;\n"
    "\n"
    "out vec2 v_TexCoords;\n"
    "out vec4 v_Color;\n"
    "\n"
    "void main()\n"
    "{\n"
    "    // Map to normalized clip coordinates:\n"
    "    float x = ((2.0 * (in_Position.x - 0.5)) / u_screenDimensions.x) - 1.0;\n"
    "    float y = 1.0 - ((2.0 * (in_Position.y - 0.5)) / u_screenDimensions.y);\n"
    "\n"
    "    gl_Position = vec4(x, y, 0.0, 1.0);\n"
    "    v_TexCoords = in_TexCoords;\n"
    "    v_Color     = vec4(in_Color, 1.0);\n"
    "}\n";

const char * DDRenderInterfaceCoreGL::textFragShaderSrc = "\n"
    "#version 150\n"
    "\n"
    "in vec2 v_TexCoords;\n"
    "in vec4 v_Color;\n"
    "\n"
    "uniform sampler2D u_glyphTexture;\n"
    "out vec4 out_FragColor;\n"
    "\n"
    "void main()\n"
    "{\n"
    "    out_FragColor = v_Color;\n"
    "    out_FragColor.a = texture(u_glyphTexture, v_TexCoords).r;\n"
    "}\n";

// ========================================================
// GLFW / window management / sample drawing:
// ========================================================

struct ThreadData
{
    dd::ContextHandle ddContext;
    DDRenderInterfaceCoreGL * renderInterface;
    void (*threadDrawFunc)(const ThreadData &);

    void init(void (*fn)(const ThreadData &), DDRenderInterfaceCoreGL * ri)
    {
        ddContext       = nullptr;
        renderInterface = ri;
        threadDrawFunc  = fn;

        dd::initialize(&ddContext, renderInterface);
    }

    void shutdown()
    {
        dd::shutdown(ddContext);

        ddContext       = nullptr;
        renderInterface = nullptr;
        threadDrawFunc  = nullptr;
    }
};

// ========================================================

static void drawLabel(const ThreadData & td, ddVec3_In pos, const char * name)
{
    if (!keys.showLabels)
    {
        return;
    }

    // Only draw labels inside the camera frustum.
    if (camera.isPointInsideFrustum(pos[0], pos[1], pos[2]))
    {
        const ddVec3 textColor = { 0.8f, 0.8f, 1.0f };
        dd::projectedText(td.ddContext, name, pos, textColor, toFloatPtr(camera.vpMatrix),
                          0, 0, WindowWidth, WindowHeight, 0.5f);
    }
}

static void drawGrid(const ThreadData & td)
{
    if (keys.showGrid)
    {
        dd::xzSquareGrid(td.ddContext, -50.0f, 50.0f, -1.0f, 1.7f, dd::colors::Green); // Grid from -50 to +50 in both X & Z
    }
}

static void drawMiscObjects(const ThreadData & td)
{
    // Start a row of objects at this position:
    ddVec3 origin = { -15.0f, 0.0f, 0.0f };

    // Box with a point at it's center:
    drawLabel(td, origin, "box");
    dd::box(td.ddContext, origin, dd::colors::Blue, 1.5f, 1.5f, 1.5f);
    dd::point(td.ddContext, origin, dd::colors::White, 15.0f);
    origin[0] += 3.0f;

    // Sphere with a point at its center
    drawLabel(td, origin, "sphere");
    dd::sphere(td.ddContext, origin, dd::colors::Red, 1.0f);
    dd::point(td.ddContext, origin, dd::colors::White, 15.0f);
    origin[0] += 4.0f;

    // Two cones, one open and one closed:
    const ddVec3 condeDir = { 0.0f, 2.5f, 0.0f };
    origin[1] -= 1.0f;

    drawLabel(td, origin, "cone (open)");
    dd::cone(td.ddContext, origin, condeDir, dd::colors::Yellow, 1.0f, 2.0f);
    dd::point(td.ddContext, origin, dd::colors::White, 15.0f);
    origin[0] += 4.0f;

    drawLabel(td, origin, "cone (closed)");
    dd::cone(td.ddContext, origin, condeDir, dd::colors::Cyan, 0.0f, 1.0f);
    dd::point(td.ddContext, origin, dd::colors::White, 15.0f);
    origin[0] += 4.0f;

    // Axis-aligned bounding box:
    const ddVec3 bbMins = { -1.0f, -0.9f, -1.0f };
    const ddVec3 bbMaxs = {  1.0f,  2.2f,  1.0f };
    const ddVec3 bbCenter = {
        (bbMins[0] + bbMaxs[0]) * 0.5f,
        (bbMins[1] + bbMaxs[1]) * 0.5f,
        (bbMins[2] + bbMaxs[2]) * 0.5f
    };
    drawLabel(td, origin, "AABB");
    dd::aabb(td.ddContext, bbMins, bbMaxs, dd::colors::Orange);
    dd::point(td.ddContext, bbCenter, dd::colors::White, 15.0f);

    // Move along the Z for another row:
    origin[0] = -15.0f;
    origin[2] += 5.0f;

    // A big arrow pointing up:
    const ddVec3 arrowFrom = { origin[0], origin[1], origin[2] };
    const ddVec3 arrowTo   = { origin[0], origin[1] + 5.0f, origin[2] };
    drawLabel(td, arrowFrom, "arrow");
    dd::arrow(td.ddContext, arrowFrom, arrowTo, dd::colors::Magenta, 1.0f);
    dd::point(td.ddContext, arrowFrom, dd::colors::White, 15.0f);
    dd::point(td.ddContext, arrowTo, dd::colors::White, 15.0f);
    origin[0] += 4.0f;

    // Plane with normal vector:
    const ddVec3 planeNormal = { 0.0f, 1.0f, 0.0f };
    drawLabel(td, origin, "plane");
    dd::plane(td.ddContext, origin, planeNormal, dd::colors::Yellow, dd::colors::Blue, 1.5f, 1.0f);
    dd::point(td.ddContext, origin, dd::colors::White, 15.0f);
    origin[0] += 4.0f;

    // Circle on the Y plane:
    drawLabel(td, origin, "circle");
    dd::circle(td.ddContext, origin, planeNormal, dd::colors::Orange, 1.5f, 15.0f);
    dd::point(td.ddContext, origin, dd::colors::White, 15.0f);
    origin[0] += 3.2f;

    // Tangent basis vectors:
    const ddVec3 normal    = { 0.0f, 1.0f, 0.0f };
    const ddVec3 tangent   = { 1.0f, 0.0f, 0.0f };
    const ddVec3 bitangent = { 0.0f, 0.0f, 1.0f };
    origin[1] += 0.1f;
    drawLabel(td, origin, "tangent basis");
    dd::tangentBasis(td.ddContext, origin, normal, tangent, bitangent, 2.5f);
    dd::point(td.ddContext, origin, dd::colors::White, 15.0f);

    // And a set of intersecting axes:
    origin[0] += 4.0f;
    origin[1] += 1.0f;
    drawLabel(td, origin, "cross");
    dd::cross(td.ddContext, origin, 2.0f);
    dd::point(td.ddContext, origin, dd::colors::White, 15.0f);
}

static void drawFrustum(const ThreadData & td)
{
    const ddVec3 color  = {  0.8f, 0.3f, 1.0f  };
    const ddVec3 origin = { -8.0f, 0.5f, 14.0f };
    drawLabel(td, origin, "frustum + axes");

    // The frustum will depict a fake camera:
    const Matrix4 proj = Matrix4::perspective(degToRad(45.0f), 800.0f / 600.0f, 0.5f, 4.0f);
    const Matrix4 view = Matrix4::lookAt(Point3(-8.0f, 0.5f, 14.0f), Point3(-8.0f, 0.5f, -14.0f), Vector3::yAxis());
    const Matrix4 clip = inverse(proj * view);
    dd::frustum(td.ddContext, toFloatPtr(clip), color);

    // A white dot at the eye position:
    dd::point(td.ddContext, origin, dd::colors::White, 15.0f);

    // A set of arrows at the camera's origin/eye:
    const Matrix4 transform = Matrix4::translation(Vector3(-8.0f, 0.5f, 14.0f)) * Matrix4::rotationZ(degToRad(60.0f));
    dd::axisTriad(td.ddContext, toFloatPtr(transform), 0.3f, 2.0f);
}

static void drawText(const ThreadData & td)
{
    // HUD text:
    const ddVec3 textColor = { 1.0f,  1.0f,  1.0f };
    const ddVec3 textPos2D = { 10.0f, 15.0f, 0.0f };
    dd::screenText(td.ddContext,
                   "Welcome to the multi-threaded Core OpenGL Debug Draw demo.\n\n"
                   "[SPACE]  to toggle labels on/off\n"
                   "[RETURN] to toggle grid on/off",
                   textPos2D, textColor, 0.55f);
}

static void sampleAppDraw(DDRenderInterfaceCoreGL & ddRenderIfaceGL, ThreadData tds[4], JobQueue & jobQ)
{
    // Camera input update (the 'camera' object is declared in samples_common.hpp):
    camera.checkKeyboardMovement();
    camera.checkMouseRotation();
    camera.updateMatrices();

    // Kick async render jobs:
    for (int i = 0; i < 4; ++i)
    {
        const ThreadData & td = tds[i];
        jobQ.pushJob([td]() { td.threadDrawFunc(td); });
    }

    // Begin a frame:
    ddRenderIfaceGL.prepareDraw(camera.vpMatrix);

    // Wait async draws to complete. In a more real life scenario this would
    // be the place to perform some other non-dependent work to avoid blocking.
    jobQ.waitAll();

    // Flush each individual context from the main thread:
    for (int i = 0; i < 4; ++i)
    {
        dd::flush(tds[i].ddContext);
    }
}

static void sampleAppStart()
{
    printDDBuildConfig();

    if (!glfwInit())
    {
        errorF("Failed to initialize GLFW!");
        return;
    }

    // Things we need for the window / GL render context:
    glfwWindowHint(GLFW_RESIZABLE, false);
    glfwWindowHint(GLFW_DEPTH_BITS, 32);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, true);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 2);

    GLFWwindow * window = glfwCreateWindow(WindowWidth, WindowHeight,
                                           "Debug Draw Sample - Core OpenGL (MT, explicit context)",
                                           nullptr, nullptr);
    if (!window)
    {
        errorF("Failed to create GLFW window!");
        return;
    }

    glfwMakeContextCurrent(window);

    if (!gl3wInit())
    {
        errorF("Failed to initialize GL3W extension library!");
        return;
    }

    if (!gl3wIsSupported(3, 2))
    {
        errorF("This sample application requires at least OpenGL version 3.2 to run!");
        return;
    }

    // From samples_common.hpp
    initInput(window);

    // Set up an OpenGL renderer:
    DDRenderInterfaceCoreGL ddRenderIfaceGL;

    // Draw contexts:
    ThreadData threads[4];
    threads[0].init(&drawGrid,        &ddRenderIfaceGL);
    threads[1].init(&drawMiscObjects, &ddRenderIfaceGL);
    threads[2].init(&drawFrustum,     &ddRenderIfaceGL);
    threads[3].init(&drawText,        &ddRenderIfaceGL);

    // Each draw function will be pushed into the async job queue
    // by the main thread every frame, then main will wait on it
    // and submit the GL draw commands with dd::flush().
    JobQueue jobQ;
    jobQ.launch();

    // Loop until the user closes the window:
    while (!glfwWindowShouldClose(window))
    {
        const double t0s = glfwGetTime();

        sampleAppDraw(ddRenderIfaceGL, threads, jobQ);
        glfwSwapBuffers(window);
        glfwPollEvents();

        const double t1s = glfwGetTime();

        deltaTime.seconds      = static_cast<float>(t1s - t0s);
        deltaTime.milliseconds = static_cast<std::int64_t>(deltaTime.seconds * 1000.0);
    }

    jobQ.waitAll();
    for (int i = 0; i < 4; ++i)
    {
        threads[i].shutdown();
    }
}

// ========================================================
// main():
// ========================================================

int main()
{
    sampleAppStart();
    gl3wShutdown();
    glfwTerminate();
}

