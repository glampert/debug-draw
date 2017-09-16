
// ================================================================================================
// -*- C++ -*-
// File:   sample_gl_legacy.cpp
// Author: Guilherme R. Lampert
// Brief:  Debug Draw usage sample with legacy (AKA fixed function) OpenGL.
//
// This software is in the public domain. Where that dedication is not recognized,
// you are granted a perpetual, irrevocable license to copy, distribute, and modify
// this file as you see fit.
// ================================================================================================

#define DEBUG_DRAW_IMPLEMENTATION
#include "debug_draw.hpp"     // Debug Draw API. Notice that we need the DEBUG_DRAW_IMPLEMENTATION macro here!
#include "samples_common.hpp" // Common code shared by the samples (camera, input, etc).

using namespace ddSamplesCommon;

// ========================================================
// Debug Draw RenderInterface for legacy OpenGL:
// ========================================================

class DDRenderInterfaceLegacyGL final
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

        if (depthEnabled)
        {
            glEnable(GL_DEPTH_TEST);
        }
        else
        {
            glDisable(GL_DEPTH_TEST);
        }

        for (int i = 0; i < count; ++i, ++points)
        {
            const dd::DrawVertex & v = *points;

            // PoinSize cannot be called between Begin/End,
            // so we can't hoist them out of this loop
            glPointSize(v.point.size);

            glBegin(GL_POINTS);
            glColor3f(v.point.r, v.point.g, v.point.b);
            glVertex3f(v.point.x, v.point.y, v.point.z);
            glEnd();
        }

        checkGLError(__FILE__, __LINE__);
    }

    void drawLineList(const dd::DrawVertex * lines, int count, bool depthEnabled) override
    {
        assert(lines != nullptr);
        assert(count > 0 && count <= DEBUG_DRAW_VERTEX_BUFFER_SIZE);

        if (depthEnabled)
        {
            glEnable(GL_DEPTH_TEST);
        }
        else
        {
            glDisable(GL_DEPTH_TEST);
        }

        glBegin(GL_LINES);
        for (int i = 0; i < count; ++i, ++lines)
        {
            const dd::DrawVertex & v = *lines;
            glColor3f(v.line.r, v.line.g, v.line.b);
            glVertex3f(v.line.x, v.line.y, v.line.z);
        }
        glEnd();

        checkGLError(__FILE__, __LINE__);
    }

    void drawGlyphList(const dd::DrawVertex * glyphs, int count, dd::GlyphTextureHandle glyphTex) override
    {
        assert(glyphs != nullptr);
        assert(count > 0 && count <= DEBUG_DRAW_VERTEX_BUFFER_SIZE);

        // Legacy 2D draw settings:
        glMatrixMode(GL_PROJECTION);
        glLoadIdentity();
        glOrtho(0, WindowWidth, WindowHeight, 0, -99999, 99999);
        glMatrixMode(GL_MODELVIEW);
        glLoadIdentity();

        glDisable(GL_DEPTH_TEST);
        glDisable(GL_CULL_FACE);

        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

        if (glyphTex != nullptr)
        {
            glEnable(GL_TEXTURE_2D);
            glBindTexture(GL_TEXTURE_2D, handleToGL(glyphTex));
        }

        // DD will send the glyphs already triangulated to make
        // integration easier with modern OpenGL, which deprecated
        // GL_QUADS. But in this legacy GL sample we can still use it,
        // so two vertexes out of every six just get thrown away.
        glBegin(GL_QUADS);
        for (int i = 0; i < count; i += 6)
        {
            const dd::DrawVertex & t0_v0 = *glyphs++;
            const dd::DrawVertex & t0_v1 = *glyphs++;
            const dd::DrawVertex & t0_v2 = *glyphs++;

            /*const dd::DrawVertex & t1_v0 = */glyphs++;
            /*const dd::DrawVertex & t1_v1 = */glyphs++;
            const dd::DrawVertex & t1_v2 = *glyphs++;

            glColor3f(t0_v0.glyph.r, t0_v0.glyph.g, t0_v0.glyph.b);
            glTexCoord2f(t0_v0.glyph.u, t0_v0.glyph.v);
            glVertex2f(t0_v0.glyph.x, t0_v0.glyph.y);

            glColor3f(t0_v2.glyph.r, t0_v2.glyph.g, t0_v2.glyph.b);
            glTexCoord2f(t0_v2.glyph.u, t0_v2.glyph.v);
            glVertex2f(t0_v2.glyph.x, t0_v2.glyph.y);

            glColor3f(t1_v2.glyph.r, t1_v2.glyph.g, t1_v2.glyph.b);
            glTexCoord2f(t1_v2.glyph.u, t1_v2.glyph.v);
            glVertex2f(t1_v2.glyph.x, t1_v2.glyph.y);

            glColor3f(t0_v1.glyph.r, t0_v1.glyph.g, t0_v1.glyph.b);
            glTexCoord2f(t0_v1.glyph.u, t0_v1.glyph.v);
            glVertex2f(t0_v1.glyph.x, t0_v1.glyph.y);
        }
        glEnd();

        glDisable(GL_BLEND);
        if (glyphTex != nullptr)
        {
            glDisable(GL_TEXTURE_2D);
        }

        checkGLError(__FILE__, __LINE__);
    }

    dd::GlyphTextureHandle createGlyphTexture(int width, int height, const void * pixels) override
    {
        assert(width > 0 && height > 0);
        assert(pixels != nullptr);

        GLuint textureId = 0;
        glGenTextures(1, &textureId);
        glBindTexture(GL_TEXTURE_2D, textureId);

        //
        // I don't recall if there was a way of getting
        // similar behavior to GL_RED/GL_R8 in legacy OpenGL...
        // Tried GL_LUMINANCE/GL_ALPHA but it didn't work.
        //
        // Simplest way is to just expand the texture to RGBA manually.
        // Takes another memory allocation though, but this is a
        // sample, so performance is not paramount ;)
        //

        struct RGBA
        {
            std::uint8_t r, g, b, a;
        };

        RGBA * expanded = new RGBA[width * height];
        const std::uint8_t * p = static_cast<const std::uint8_t *>(pixels);

        // Expand graymap the RGBA:
        for (int i = 0; i < width * height; ++i)
        {
            expanded[i].r = 255;
            expanded[i].g = 255;
            expanded[i].b = 255;
            expanded[i].a = p[i];
        }

        glTexImage2D(GL_TEXTURE_2D, 0, 4, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, expanded);
        delete[] expanded;

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

        glBindTexture(GL_TEXTURE_2D, 0);
        checkGLError(__FILE__, __LINE__);

        return GLToHandle(textureId);
    }

    void destroyGlyphTexture(dd::GlyphTextureHandle glyphTex) override
    {
        if (glyphTex == nullptr)
        {
            return;
        }

        const GLuint textureId = handleToGL(glyphTex);
        glBindTexture(GL_TEXTURE_2D, 0);
        glDeleteTextures(1, &textureId);
    }

    // These two can also be implemented to perform GL render
    // state setup/cleanup, but we don't use them in this sample.
    //void beginDraw() override { }
    //void endDraw()   override { }

    //
    // Local methods:
    //

    DDRenderInterfaceLegacyGL()
    {
        std::printf("\n");
        std::printf("GL_VENDOR   : %s\n",   glGetString(GL_VENDOR));
        std::printf("GL_RENDERER : %s\n",   glGetString(GL_RENDERER));
        std::printf("GL_VERSION  : %s\n\n", glGetString(GL_VERSION));
        std::printf("DDRenderInterfaceLegacyGL initializing ...\n");

        // Default OpenGL states:
        glEnable(GL_CULL_FACE);
        glEnable(GL_DEPTH_TEST);
        glDisable(GL_BLEND);

        std::printf("DDRenderInterfaceLegacyGL ready!\n\n");
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
            errorF("%s(%d) : GL_ERROR=0x%X - %s", file, line, err, errorToString(err));
        }
    }
}; // class DDRenderInterfaceLegacyGL

// ========================================================
// GLFW / window management / sample drawing:
// ========================================================

static void drawGrid()
{
    if (!keys.showGrid)
    {
        return;
    }
    dd::xzSquareGrid(-50.0f, 50.0f, -1.0f, 1.7f, dd::colors::Green); // Grid from -50 to +50 in both X & Z
}

static void drawLabel(ddVec3_In pos, const char * name)
{
    if (!keys.showLabels)
    {
        return;
    }

    // Only draw labels inside the camera frustum.
    if (camera.isPointInsideFrustum(pos[0], pos[1], pos[2]))
    {
        const ddVec3 textColor = { 0.8f, 0.8f, 1.0f };
        dd::projectedText(name, pos, textColor, toFloatPtr(camera.vpMatrix),
                          0, 0, WindowWidth, WindowHeight, 0.5f);
    }
}

static void drawMiscObjects()
{
    // Start a row of objects at this position:
    ddVec3 origin = { -15.0f, 0.0f, 0.0f };

    // Box with a point at it's center:
    drawLabel(origin, "box");
    dd::box(origin, dd::colors::Blue, 1.5f, 1.5f, 1.5f);
    dd::point(origin, dd::colors::White, 15.0f);
    origin[0] += 3.0f;

    // Sphere with a point at its center
    drawLabel(origin, "sphere");
    dd::sphere(origin, dd::colors::Red, 1.0f);
    dd::point(origin, dd::colors::White, 15.0f);
    origin[0] += 4.0f;

    // Two cones, one open and one closed:
    const ddVec3 condeDir = { 0.0f, 2.5f, 0.0f };
    origin[1] -= 1.0f;

    drawLabel(origin, "cone (open)");
    dd::cone(origin, condeDir, dd::colors::Yellow, 1.0f, 2.0f);
    dd::point(origin, dd::colors::White, 15.0f);
    origin[0] += 4.0f;

    drawLabel(origin, "cone (closed)");
    dd::cone(origin, condeDir, dd::colors::Cyan, 0.0f, 1.0f);
    dd::point(origin, dd::colors::White, 15.0f);
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
    dd::aabb(bbMins, bbMaxs, dd::colors::Orange);
    dd::point(bbCenter, dd::colors::White, 15.0f);

    // Move along the Z for another row:
    origin[0] = -15.0f;
    origin[2] += 5.0f;

    // A big arrow pointing up:
    const ddVec3 arrowFrom = { origin[0], origin[1], origin[2] };
    const ddVec3 arrowTo   = { origin[0], origin[1] + 5.0f, origin[2] };
    drawLabel(arrowFrom, "arrow");
    dd::arrow(arrowFrom, arrowTo, dd::colors::Magenta, 1.0f);
    dd::point(arrowFrom, dd::colors::White, 15.0f);
    dd::point(arrowTo, dd::colors::White, 15.0f);
    origin[0] += 4.0f;

    // Plane with normal vector:
    const ddVec3 planeNormal = { 0.0f, 1.0f, 0.0f };
    drawLabel(origin, "plane");
    dd::plane(origin, planeNormal, dd::colors::Yellow, dd::colors::Blue, 1.5f, 1.0f);
    dd::point(origin, dd::colors::White, 15.0f);
    origin[0] += 4.0f;

    // Circle on the Y plane:
    drawLabel(origin, "circle");
    dd::circle(origin, planeNormal, dd::colors::Orange, 1.5f, 15.0f);
    dd::point(origin, dd::colors::White, 15.0f);
    origin[0] += 3.2f;

    // Tangent basis vectors:
    const ddVec3 normal    = { 0.0f, 1.0f, 0.0f };
    const ddVec3 tangent   = { 1.0f, 0.0f, 0.0f };
    const ddVec3 bitangent = { 0.0f, 0.0f, 1.0f };
    origin[1] += 0.1f;
    drawLabel(origin, "tangent basis");
    dd::tangentBasis(origin, normal, tangent, bitangent, 2.5f);
    dd::point(origin, dd::colors::White, 15.0f);

    // And a set of intersecting axes:
    origin[0] += 4.0f;
    origin[1] += 1.0f;
    drawLabel(origin, "cross");
    dd::cross(origin, 2.0f);
    dd::point(origin, dd::colors::White, 15.0f);
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
    dd::point(origin, dd::colors::White, 15.0f);

    // A set of arrows at the camera's origin/eye:
    const Matrix4 transform = Matrix4::translation(Vector3(-8.0f, 0.5f, 14.0f)) * Matrix4::rotationZ(degToRad(60.0f));
    dd::axisTriad(toFloatPtr(transform), 0.3f, 2.0f);
}

static void drawText()
{
    // HUD text:
    const ddVec3 textColor = { 1.0f,  1.0f,  1.0f };
    const ddVec3 textPos2D = { 10.0f, 15.0f, 0.0f };
    dd::screenText("Welcome to the legacy OpenGL Debug Draw demo.\n\n"
                   "[SPACE]  to toggle labels on/off\n"
                   "[RETURN] to toggle grid on/off",
                   textPos2D, textColor, 0.55f);
}

static void sampleAppDraw()
{
    // Camera input update (the 'camera' object is declared in samples_common.hpp):
    camera.checkKeyboardMovement();
    camera.checkMouseRotation();
    camera.updateMatrices();

    glMatrixMode(GL_PROJECTION);
    glLoadMatrixf(toFloatPtr(camera.projMatrix));

    glMatrixMode(GL_MODELVIEW);
    glLoadMatrixf(toFloatPtr(camera.viewMatrix));

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
    printDDBuildConfig();

    if (!glfwInit())
    {
        errorF("Failed to initialize GLFW!");
        return;
    }

    // Things we need for the window / GL render context:
    glfwWindowHint(GLFW_RESIZABLE, false);
    glfwWindowHint(GLFW_DEPTH_BITS, 32);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 2);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);

    GLFWwindow * window = glfwCreateWindow(WindowWidth, WindowHeight,
                                           "Debug Draw Sample - Legacy OpenGL",
                                           nullptr, nullptr);
    if (!window)
    {
        errorF("Failed to create GLFW window!");
        return;
    }

    glfwMakeContextCurrent(window);

    // From samples_common.hpp
    initInput(window);

    // Set up the Debug Draw:
    DDRenderInterfaceLegacyGL ddRenderIfaceGL;
    dd::initialize(&ddRenderIfaceGL);

    // Loop until the user closes the window:
    while (!glfwWindowShouldClose(window))
    {
        const double t0s = glfwGetTime();

        sampleAppDraw();
        glfwSwapBuffers(window);
        glfwPollEvents();

        const double t1s = glfwGetTime();

        deltaTime.seconds      = static_cast<float>(t1s - t0s);
        deltaTime.milliseconds = static_cast<std::int64_t>(deltaTime.seconds * 1000.0);
    }

    dd::shutdown();
}

// ========================================================
// main():
// ========================================================

int main()
{
    sampleAppStart();
    glfwTerminate();
}

