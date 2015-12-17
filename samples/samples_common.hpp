
// ================================================================================================
// -*- C++ -*-
// File: samples_common.hpp
// Author: Guilherme R. Lampert
// Created on: 15/12/15
// Brief: Common code shared by the Debug Draw samples.
//
// This software is in the public domain. Where that dedication is not recognized,
// you are granted a perpetual, irrevocable license to copy, distribute, and modify
// this file as you see fit.
// ================================================================================================

#ifndef DD_SAMPLES_COMMON_HPP
#define DD_SAMPLES_COMMON_HPP

// Shared dependencies:
#include <cassert>
#include <cmath>
#include <cstddef>
#include <cstdio>
#include <cstring>
#include <iostream>
#include <GLFW/glfw3.h>
#include <vectormath.h>

// Check for a couple C++11 goodies we'd like to use if available...
#if DEBUG_DRAW_CXX11_SUPPORTED
    #define OVERRIDE_METHOD override
    #define FINAL_CLASS     final
    #define NULLPTR         nullptr
#else // !C++11
    #define OVERRIDE_METHOD
    #define FINAL_CLASS
    #define NULLPTR         NULL
#endif // DEBUG_DRAW_CXX11_SUPPORTED

// App window dimensions; Not resizable.
static const int windowWidth  = 1024;
static const int windowHeight = 768;

// Angle in degrees to angle in radians for sin/cos/etc.
static inline float degToRad(const float ang)
{
    return ang * 3.1415926535897931f / 180.0f;
}

// Time in milliseconds since the application started.
static inline long long getTimeMilliseconds()
{
    const double seconds = glfwGetTime();
    return static_cast<long long>(seconds * 1000.0);
}

// GL error enum to printable string.
static inline const char * errorToString(const GLenum errorCode)
{
    switch (errorCode)
    {
    case GL_NO_ERROR          : return "GL_NO_ERROR";
    case GL_INVALID_ENUM      : return "GL_INVALID_ENUM";
    case GL_INVALID_VALUE     : return "GL_INVALID_VALUE";
    case GL_INVALID_OPERATION : return "GL_INVALID_OPERATION";
    case GL_OUT_OF_MEMORY     : return "GL_OUT_OF_MEMORY";
    case GL_STACK_UNDERFLOW   : return "GL_STACK_UNDERFLOW"; // Legacy; not used on GL3+
    case GL_STACK_OVERFLOW    : return "GL_STACK_OVERFLOW";  // Legacy; not used on GL3+
    default                   : return "Unknown GL error";
    } // switch (errorCode)
}

// ========================================================
// Key/Mouse input + A simple 3D camera:
// ========================================================

struct Keys
{
    // For the first-person camera controls.
    bool wDown;
    bool sDown;
    bool aDown;
    bool dDown;
    // Flags:
    bool showLabels; // True if object labels are drawn. Toggle with the space bar.
    bool showGrid;   // True if the ground grid is drawn. Toggle with the return key.
} keys;

struct Mouse
{
    enum { MaxDelta = 100 };
    int  deltaX;
    int  deltaY;
    int  lastPosX;
    int  lastPosY;
    bool leftButtonDown;
    bool rightButtonDown;
} mouse;

struct Camera
{
    //
    // Camera Axes:
    //
    //    (up)
    //    +Y   +Z (forward)
    //    |   /
    //    |  /
    //    | /
    //    + ------ +X (right)
    //  (eye)
    //
    Vector3 right;
    Vector3 up;
    Vector3 forward;
    Vector3 eye;
    Matrix4 viewMatrix;
    Matrix4 projMatrix;
    Matrix4 vpMatrix;

    enum MoveDir
    {
        Forward, // Move forward relative to the camera's space
        Back,    // Move backward relative to the camera's space
        Left,    // Move left relative to the camera's space
        Right    // Move right relative to the camera's space
    };

    Camera()
    {
        right      = Vector3(1.0f, 0.0f, 0.0f);
        up         = Vector3(0.0f, 1.0f, 0.0f);
        forward    = Vector3(0.0f, 0.0f, 1.0f);
        eye        = Vector3(0.0f, 0.0f, 0.0f);
        viewMatrix = Matrix4::identity();
        vpMatrix   = Matrix4::identity();

        const float fovY   = degToRad(60.0f);
        const float aspect = static_cast<float>(windowWidth) / static_cast<float>(windowHeight);
        projMatrix = Matrix4::perspective(fovY, aspect, 0.1f, 1000.0f);
    }

    void pitch(const float angle)
    {
        // Pitches camera by 'angle' radians.
        forward = rotateAroundAxis(forward, right, angle); // Calculate new forward.
        up      = cross(forward, right);                   // Calculate new camera up vector.
    }

    void rotate(const float angle)
    {
        // Rotates around world Y-axis by the given angle (in radians).
        const float sinAng = std::sin(angle);
        const float cosAng = std::cos(angle);
        float xxx, zzz;

        // Rotate forward vector:
        xxx = forward[0];
        zzz = forward[2];
        forward[0] = xxx *  cosAng + zzz * sinAng;
        forward[2] = xxx * -sinAng + zzz * cosAng;

        // Rotate up vector:
        xxx = up[0];
        zzz = up[2];
        up[0] = xxx *  cosAng + zzz * sinAng;
        up[2] = xxx * -sinAng + zzz * cosAng;

        // Rotate right vector:
        xxx = right[0];
        zzz = right[2];
        right[0] = xxx *  cosAng + zzz * sinAng;
        right[2] = xxx * -sinAng + zzz * cosAng;
    }

    void move(const MoveDir dir, const float amount)
    {
        switch (dir)
        {
        case Camera::Forward : eye += forward * amount; break;
        case Camera::Back    : eye -= forward * amount; break;
        case Camera::Left    : eye += right   * amount; break;
        case Camera::Right   : eye -= right   * amount; break;
        }
    }

    void checkKeyboardMovement()
    {
        static const float moveSpeed = 3.0f * (1.0f / 60.0f);

        if (keys.aDown) { move(Camera::Left,    moveSpeed); }
        if (keys.dDown) { move(Camera::Right,   moveSpeed); }
        if (keys.wDown) { move(Camera::Forward, moveSpeed); }
        if (keys.sDown) { move(Camera::Back,    moveSpeed); }
    }

    void checkMouseRotation()
    {
        static const float maxAngle    = 89.5f; // Max degrees of rotation
        static const float rotateSpeed = 5.0f * (1.0f / 60.0f);
        static float pitchAmt;

        if (!mouse.leftButtonDown)
        {
            return;
        }

        // Rotate left/right:
        float amt = static_cast<float>(mouse.deltaX) * rotateSpeed;
        rotate(degToRad(-amt));

        // Calculate amount to rotate up/down:
        amt = static_cast<float>(mouse.deltaY) * rotateSpeed;

        // Clamp pitch amount:
        if ((pitchAmt + amt) <= -maxAngle)
        {
            amt = -maxAngle - pitchAmt;
            pitchAmt = -maxAngle;
        }
        else if ((pitchAmt + amt) >= maxAngle)
        {
            amt = maxAngle - pitchAmt;
            pitchAmt = maxAngle;
        }
        else
        {
            pitchAmt += amt;
        }

        pitch(degToRad(-amt));
    }

    void updateMatrices()
    {
        viewMatrix = Matrix4::lookAt(Point3(eye), getTarget(), up);
        vpMatrix   = projMatrix * viewMatrix; // Vectormath lib uses column-major OGL style, so multiply P*V*M
    }

    Point3 getTarget() const
    {
        return Point3(eye[0] + forward[0], eye[1] + forward[1], eye[2] + forward[2]);
    }

    static Vector3 rotateAroundAxis(const Vector3 & vec, const Vector3 & axis, const float angle)
    {
        const float sinAng = std::sin(angle);
        const float cosAng = std::cos(angle);
        const float oneMinusCosAng = (1.0f - cosAng);

        const float aX = axis[0];
        const float aY = axis[1];
        const float aZ = axis[2];

        float x = (aX * aX * oneMinusCosAng + cosAng)      * vec[0] +
                  (aX * aY * oneMinusCosAng + aZ * sinAng) * vec[1] +
                  (aX * aZ * oneMinusCosAng - aY * sinAng) * vec[2];

        float y = (aX * aY * oneMinusCosAng - aZ * sinAng) * vec[0] +
                  (aY * aY * oneMinusCosAng + cosAng)      * vec[1] +
                  (aY * aZ * oneMinusCosAng + aX * sinAng) * vec[2];

        float z = (aX * aZ * oneMinusCosAng + aY * sinAng) * vec[0] +
                  (aY * aZ * oneMinusCosAng - aX * sinAng) * vec[1] +
                  (aZ * aZ * oneMinusCosAng + cosAng)      * vec[2];

        return Vector3(x, y, z);
    }
} camera;

// ========================================================
// Input callbacks for GLFW:
// ========================================================

static void mousePositionCallback(GLFWwindow * window, const double xPos, const double yPos)
{
    (void)window; // Unused.

    int mx = static_cast<int>(xPos);
    int my = static_cast<int>(yPos);

    // Clamp to window bounds:
    if      (mx > windowWidth)  { mx = windowWidth;  }
    else if (mx < 0)            { mx = 0;            }
    if      (my > windowHeight) { my = windowHeight; }
    else if (my < 0)            { my = 0;            }

    mouse.deltaX = mx - mouse.lastPosX;
    mouse.deltaY = my - mouse.lastPosY;
    mouse.lastPosX = mx;
    mouse.lastPosY = my;

    // Clamp between -/+ max delta:
    if      (mouse.deltaX >  Mouse::MaxDelta) { mouse.deltaX =  Mouse::MaxDelta; }
    else if (mouse.deltaX < -Mouse::MaxDelta) { mouse.deltaX = -Mouse::MaxDelta; }
    if      (mouse.deltaY >  Mouse::MaxDelta) { mouse.deltaY =  Mouse::MaxDelta; }
    else if (mouse.deltaY < -Mouse::MaxDelta) { mouse.deltaY = -Mouse::MaxDelta; }
}

static void mouseButtonCallback(GLFWwindow * window, const int button, const int action, const int mods)
{
    // Unused.
    (void)window;
    (void)mods;

    if (button == GLFW_MOUSE_BUTTON_LEFT)
    {
        mouse.leftButtonDown = (action != GLFW_RELEASE);
    }
    else if (button == GLFW_MOUSE_BUTTON_RIGHT)
    {
        mouse.rightButtonDown = (action != GLFW_RELEASE);
    }
}

static void keyCallback(GLFWwindow * window, const int key, const int scancode, const int action, const int mods)
{
    // Unused.
    (void)window;
    (void)scancode;
    (void)mods;

    if      (key == GLFW_KEY_A || key == GLFW_KEY_LEFT)  { keys.aDown = (action != GLFW_RELEASE); }
    else if (key == GLFW_KEY_D || key == GLFW_KEY_RIGHT) { keys.dDown = (action != GLFW_RELEASE); }
    else if (key == GLFW_KEY_W || key == GLFW_KEY_UP)    { keys.wDown = (action != GLFW_RELEASE); }
    else if (key == GLFW_KEY_S || key == GLFW_KEY_DOWN)  { keys.sDown = (action != GLFW_RELEASE); }
    // Toggleable flags:
    else if (key == GLFW_KEY_SPACE && action == GLFW_PRESS)
    {
        keys.showLabels = !keys.showLabels;
    }
    else if (key == GLFW_KEY_ENTER && action == GLFW_PRESS)
    {
        keys.showGrid = !keys.showGrid;
    }
}

static void initInput(GLFWwindow * window)
{
    std::memset(&keys,  0, sizeof(keys));
    std::memset(&mouse, 0, sizeof(mouse));

    // GLFW input callbacks:
    glfwSetCursorPosCallback(window,   &mousePositionCallback);
    glfwSetMouseButtonCallback(window, &mouseButtonCallback);
    glfwSetKeyCallback(window,         &keyCallback);
}

#endif // DD_SAMPLES_COMMON_HPP
