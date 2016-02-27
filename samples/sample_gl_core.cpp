
// ================================================================================================
// -*- C++ -*-
// File: sample_gl_core.cpp
// Author: Guilherme R. Lampert
// Created on: 10/12/15
// Brief: Debug Draw usage sample with Core Profile OpenGL 3+
//
// This software is in the public domain. Where that dedication is not recognized,
// you are granted a perpetual, irrevocable license to copy, distribute, and modify
// this file as you see fit.
// ================================================================================================

#define DEBUG_DRAW_IMPLEMENTATION
#include "debug_draw.hpp"     // Debug Draw API. Notice that we need the DEBUG_DRAW_IMPLEMENTATION macro here!
#include <GL/gl3w.h>          // An OpenGL extension wrangler (https://github.com/skaslev/gl3w).
#include "samples_common.hpp" // Common code shared by the samples (camera, input, etc).

// ========================================================
// Minimal shaders we need for the debug primitives:
// ========================================================

static const char * linePointVertShaderSrc = "\n"
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

// ------------------------------------------------------------------

static const char * linePointFragShaderSrc = "\n"
"#version 150\n"
"\n"
"in vec4 v_Color;\n"
"out vec4 out_FragColor;\n"
"\n"
"void main()\n"
"{\n"
"    out_FragColor = v_Color;\n"
"}\n";

// ------------------------------------------------------------------

static const char * textVertShaderSrc = "\n"
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

// ------------------------------------------------------------------

static const char * textFragShaderSrc = "\n"
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
// Debug Draw RenderInterface for Core OpenGL:
// ========================================================

class DDRenderInterfaceCoreGL FINAL_CLASS
    : public dd::RenderInterface
{
public:

    //
    // dd::RenderInterface overrides:
    //

    void drawPointList(const dd::DrawVertex * points, const int count,
                       const bool depthEnabled) OVERRIDE_METHOD
    {
        assert(points != NULLPTR);
        assert(count > 0 && count <= DEBUG_DRAW_VERTEX_BUFFER_SIZE);

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

    void drawLineList(const dd::DrawVertex * lines, const int count,
                      const bool depthEnabled) OVERRIDE_METHOD
    {
        assert(lines != NULLPTR);
        assert(count > 0 && count <= DEBUG_DRAW_VERTEX_BUFFER_SIZE);

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

    void drawGlyphList(const dd::DrawVertex * glyphs, const int count,
                       dd::GlyphTextureHandle glyphTex) OVERRIDE_METHOD
    {
        assert(glyphs != NULLPTR);
        assert(count > 0 && count <= DEBUG_DRAW_VERTEX_BUFFER_SIZE);

        glBindVertexArray(textVAO);
        glUseProgram(textProgram);

        // These doesn't have to be reset every draw call, I'm just being lazy ;)
        glUniform1i(textProgram_GlyphTextureLocation, 0);
        glUniform2f(textProgram_ScreenDimensions, windowWidth, windowHeight);

        if (glyphTex != NULLPTR)
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

    dd::GlyphTextureHandle createGlyphTexture(const int width, const int height,
                                              const void * pixels) OVERRIDE_METHOD
    {
        assert(width > 0 && height > 0);
        assert(pixels != NULLPTR);

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

    void destroyGlyphTexture(dd::GlyphTextureHandle glyphTex) OVERRIDE_METHOD
    {
        if (glyphTex == NULLPTR)
        {
            return;
        }

        const GLuint textureId = handleToGL(glyphTex);
        glBindTexture(GL_TEXTURE_2D, 0);
        glDeleteTextures(1, &textureId);
    }

    // These two can also be implemented to perform GL render
    // state setup/cleanup, but we don't use them in this demo.
    /*
    void beginDraw() OVERRIDE_METHOD { }
    void endDraw()   OVERRIDE_METHOD { }
    */

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
        std::cout << std::endl;
        std::cout << "GL_VENDOR    : " << glGetString(GL_VENDOR)   << "\n";
        std::cout << "GL_RENDERER  : " << glGetString(GL_RENDERER) << "\n";
        std::cout << "GL_VERSION   : " << glGetString(GL_VERSION)  << "\n";
        std::cout << "GLSL_VERSION : " << glGetString(GL_SHADING_LANGUAGE_VERSION) << "\n\n";
        std::cout << "DDRenderInterfaceCoreGL initializing ...\n";

        // Default OpenGL states:
        glEnable(GL_CULL_FACE);
        glEnable(GL_DEPTH_TEST);
        glDisable(GL_BLEND);

        // This has to be enabled since the point drawing shader will use gl_PointSize.
        glEnable(GL_PROGRAM_POINT_SIZE);

        setupShaderPrograms();
        setupVertexBuffers();

        std::cout << "DDRenderInterfaceCoreGL ready!\n";
        std::cout << std::endl;
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

    void setupShaderPrograms()
    {
        std::cout << "> DDRenderInterfaceCoreGL::setupShaderPrograms()" << std::endl;

        //
        // Line/point drawing shader:
        //
        {
            GLuint linePointVS = glCreateShader(GL_VERTEX_SHADER);
            glShaderSource(linePointVS, 1, &linePointVertShaderSrc, NULLPTR);
            compileShader(linePointVS);

            GLint linePointFS = glCreateShader(GL_FRAGMENT_SHADER);
            glShaderSource(linePointFS, 1, &linePointFragShaderSrc, NULLPTR);
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
                std::cerr << "Unable to get u_MvpMatrix uniform location!" << std::endl;
            }
            checkGLError(__FILE__, __LINE__);
        }

        //
        // Text rendering shader:
        //
        {
            GLuint textVS = glCreateShader(GL_VERTEX_SHADER);
            glShaderSource(textVS, 1, &textVertShaderSrc, NULLPTR);
            compileShader(textVS);

            GLint textFS = glCreateShader(GL_FRAGMENT_SHADER);
            glShaderSource(textFS, 1, &textFragShaderSrc, NULLPTR);
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
                std::cerr << "Unable to get u_glyphTexture uniform location!" << std::endl;
            }

            textProgram_ScreenDimensions = glGetUniformLocation(textProgram, "u_screenDimensions");
            if (textProgram_ScreenDimensions < 0)
            {
                std::cerr << "Unable to get u_screenDimensions uniform location!" << std::endl;
            }

            checkGLError(__FILE__, __LINE__);
        }
    }

    void setupVertexBuffers()
    {
        std::cout << "> DDRenderInterfaceCoreGL::setupVertexBuffers()" << std::endl;

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
            glBufferData(GL_ARRAY_BUFFER, DEBUG_DRAW_VERTEX_BUFFER_SIZE * sizeof(dd::DrawVertex), NULLPTR, GL_STREAM_DRAW);
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
            glBufferData(GL_ARRAY_BUFFER, DEBUG_DRAW_VERTEX_BUFFER_SIZE * sizeof(dd::DrawVertex), NULLPTR, GL_STREAM_DRAW);
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
        GLenum err = 0;
        char msg[1024];
        while ((err = glGetError()) != 0)
        {
            std::snprintf(msg, sizeof(msg), "%s(%d) : GL_CORE_ERROR=0x%X - %s",
                          file, line, err, errorToString(err));
            std::cerr << msg << std::endl;
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
            glGetShaderInfoLog(shader, sizeof(strInfoLog) - 1, NULLPTR, strInfoLog);
            std::cerr << "\n>>> Shader compiler errors: \n" << strInfoLog << std::endl;
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
            glGetProgramInfoLog(program, sizeof(strInfoLog) - 1, NULLPTR, strInfoLog);
            std::cerr << "\n>>> Program linker errors: \n" << strInfoLog << std::endl;
        }
    }

    // The "model-view-projection" matrix for the scene.
    // In this demo, it consists of the camera's view and projection matrices only.
    Matrix4 mvpMatrix;

private:

    GLuint linePointProgram;
    GLint  linePointProgram_MvpMatrixLocation;

    GLuint textProgram;
    GLint  textProgram_GlyphTextureLocation;
    GLint  textProgram_ScreenDimensions;

    GLuint linePointVAO;
    GLuint linePointVBO;

    GLuint textVAO;
    GLuint textVBO;

}; // class DDRenderInterfaceCoreGL

// ========================================================
// GLFW / window management / sample drawing:
// ========================================================

static void drawGrid()
{
    if (!keys.showGrid)
    {
        return;
    }

    const ddVec3 green = { 0.0f, 0.6f, 0.0f };
    dd::xzSquareGrid(-50.0f, 50.0f, -1.0f, 1.7f, green); // Grid from -50 to +50 in both X & Z
}

static void drawLabel(ddVec3Param pos, const char * name)
{
    if (!keys.showLabels)
    {
        return;
    }

    // NOTE: Labels that are not in the view volume should not be added.
    // In this demo, we skip checking for simplicity, so projected labels
    // that go out of view might still show up in the corners. A frustum
    // check before adding them would fix the issue.
    const ddVec3 textColor = { 0.8f, 0.8f, 1.0f };
    dd::projectedText(name, pos, textColor, toFloatPtr(camera.vpMatrix),
                      0, 0, windowWidth, windowHeight, 0.5f);
}

static void drawMiscObjects()
{
    const ddVec3 red     = { 1.0f, 0.0f, 0.0f };
    const ddVec3 blue    = { 0.0f, 0.0f, 1.0f };
    const ddVec3 cyan    = { 0.0f, 1.0f, 1.0f };
    const ddVec3 magenta = { 1.0f, 0.2f, 0.8f };
    const ddVec3 yellow  = { 1.0f, 1.0f, 0.0f };
    const ddVec3 orange  = { 1.0f, 0.5f, 0.0f };
    const ddVec3 white   = { 1.0f, 1.0f, 1.0f };

    // Start a row of objects at this position:
    ddVec3 origin = { -15.0f, 0.0f, 0.0f };

    // Box with a point at it's center:
    drawLabel(origin, "box");
    dd::box(origin, blue, 1.5f, 1.5f, 1.5f);
    dd::point(origin, white, 15.0f);
    origin[0] += 3.0f;

    // Sphere with a point at its center
    drawLabel(origin, "sphere");
    dd::sphere(origin, red, 1.0f);
    dd::point(origin, white, 15.0f);
    origin[0] += 4.0f;

    // Two cones, one open and one closed:
    const ddVec3 condeDir = { 0.0f, 2.5f, 0.0f };
    origin[1] -= 1.0f;

    drawLabel(origin, "cone (open)");
    dd::cone(origin, condeDir, yellow, 1.0f, 2.0f);
    dd::point(origin, white, 15.0f);
    origin[0] += 4.0f;

    drawLabel(origin, "cone (closed)");
    dd::cone(origin, condeDir, cyan, 0.0f, 1.0f);
    dd::point(origin, white, 15.0f);
    origin[0] += 4.0f;

    // Axis-aligned bounding box:
    const ddVec3 bbMins = { -1.0f, -0.9f, -1.0f };
    const ddVec3 bbMaxs = {  1.0f,  2.2f,  1.0f };
    const ddVec3 bbCenter = {
        (bbMins[0] + bbMaxs[0]) * 0.5f,
        (bbMins[1] + bbMaxs[1]) * 0.5f,
        (bbMins[2] + bbMaxs[2]) * 0.5f
    };
    drawLabel(origin, "AABB");
    dd::aabb(bbMins, bbMaxs, orange);
    dd::point(bbCenter, white, 15.0f);

    // Move along the Z for another row:
    origin[0] = -15.0f;
    origin[2] += 5.0f;

    // A big arrow pointing up:
    const ddVec3 arrowFrom = { origin[0], origin[1], origin[2] };
    const ddVec3 arrowTo   = { origin[0], origin[1] + 5.0f, origin[2] };
    drawLabel(arrowFrom, "arrow");
    dd::arrow(arrowFrom, arrowTo, magenta, 1.0f);
    dd::point(arrowFrom, white, 15.0f);
    dd::point(arrowTo, white, 15.0f);
    origin[0] += 4.0f;

    // Plane with normal vector:
    const ddVec3 planeNormal = { 0.0f, 1.0f, 0.0f };
    drawLabel(origin, "plane");
    dd::plane(origin, planeNormal, yellow, blue, 1.5f, 1.0f);
    dd::point(origin, white, 15.0f);
    origin[0] += 4.0f;

    // Circle on the Y plane:
    drawLabel(origin, "circle");
    dd::circle(origin, planeNormal, orange, 1.5f, 15.0f);
    dd::point(origin, white, 15.0f);
    origin[0] += 3.2f;

    // Tangent basis vectors:
    const ddVec3 normal    = { 0.0f, 1.0f, 0.0f };
    const ddVec3 tangent   = { 1.0f, 0.0f, 0.0f };
    const ddVec3 bitangent = { 0.0f, 0.0f, 1.0f };
    origin[1] += 0.1f;
    drawLabel(origin, "tangent basis");
    dd::tangentBasis(origin, normal, tangent, bitangent, 2.5f);
    dd::point(origin, white, 15.0f);

    // And a set of intersecting axes:
    origin[0] += 4.0f;
    origin[1] += 1.0f;
    drawLabel(origin, "cross");
    dd::cross(origin, 2.0f);
    dd::point(origin, white, 15.0f);
}

static void drawFrustum()
{
    const ddVec3 color  = {  0.8f, 0.3f, 1.0f  };
    const ddVec3 origin = { -8.0f, 0.5f, 14.0f };
    drawLabel(origin, "frustum + axes");

    // The frustum will depict a fake camera:
    const Matrix4 proj = Matrix4::perspective(degToRad(45.0f), 800.0f / 600.0f, 0.5f, 4.0f);
    const Matrix4 view = Matrix4::lookAt(Point3(-8.0f, 0.5f, 14.0f), Point3(-8.0f, 0.5f, -14.0f), Vector3::yAxis());
    const Matrix4 clip = inverse(proj * view);
    dd::frustum(toFloatPtr(clip), color);

    // A white dot at the eye position:
    const ddVec3 white = { 1.0f, 1.0f, 1.0f };
    dd::point(origin, white, 15.0f);

    // A set of arrows at the camera's origin/eye:
    const Matrix4 transform = Matrix4::translation(Vector3(-8.0f, 0.5f, 14.0f)) * Matrix4::rotationZ(degToRad(60.0f));
    dd::axisTriad(toFloatPtr(transform), 0.3f, 2.0f);
}

static void drawText()
{
    // HUD text:
    const ddVec3 textColor = { 1.0f,  1.0f,  1.0f };
    const ddVec3 textPos2D = { 10.0f, 15.0f, 0.0f };
    dd::screenText("Welcome to the Core OpenGL Debug Draw demo.\n\n"
                   "[SPACE]  to toggle labels on/off\n"
                   "[RETURN] to toggle grid on/off",
                   textPos2D, textColor, 0.55f);
}

static void sampleAppDraw(DDRenderInterfaceCoreGL & ddRenderIfaceGL)
{
    // Camera input update (the 'camera' object is declared in samples_common.hpp):
    camera.checkKeyboardMovement();
    camera.checkMouseRotation();
    camera.updateMatrices();

    // Send the camera matrix to the shaders:
    ddRenderIfaceGL.mvpMatrix = camera.vpMatrix;

    glClearColor(0.2f, 0.2f, 0.2f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // Call some DD functions to add stuff to the debug draw queues:
    drawGrid();
    drawMiscObjects();
    drawFrustum();
    drawText();

    // Drawing only really happens now (here's where RenderInterface gets called).
    dd::flush(getTimeMilliseconds());
}

static void sampleAppStart()
{
    if (!glfwInit())
    {
        std::cerr << "Failed to initialize GLFW!" << std::endl;
        return;
    }

    // Things we need for the window / GL render context:
    glfwWindowHint(GLFW_RESIZABLE, false);
    glfwWindowHint(GLFW_DEPTH_BITS, 32);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, true);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 2);

    GLFWwindow * window = glfwCreateWindow(windowWidth, windowHeight,
                                           "Debug Draw Sample - Core OpenGL",
                                           NULLPTR, NULLPTR);
    if (!window)
    {
        std::cerr << "Failed to create GLFW window!" << std::endl;
        return;
    }

    glfwMakeContextCurrent(window);

    if (!gl3wInit())
    {
        std::cerr << "Failed to initialize GL3W extension library!" << std::endl;
        return;
    }

    if (!gl3wIsSupported(3, 2))
    {
        std::cerr << "This sample application requires at least OpenGL version 3.2 to run!" << std::endl;
        return;
    }

    // From samples_common.hpp
    initInput(window);

    // Set up the Debug Draw:
    DDRenderInterfaceCoreGL ddRenderIfaceGL;
    dd::initialize(&ddRenderIfaceGL);

    // Loop until the user closes the window:
    while (!glfwWindowShouldClose(window))
    {
        sampleAppDraw(ddRenderIfaceGL);
        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    dd::shutdown();
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
