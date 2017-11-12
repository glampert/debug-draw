
// ================================================================================================
// -*- C++ -*-
// File:   samples_common.hpp
// Author: Guilherme R. Lampert
// Brief:  Common code shared by the Debug Draw samples.
//
// This software is in the public domain. Where that dedication is not recognized,
// you are granted a perpetual, irrevocable license to copy, distribute, and modify
// this file as you see fit.
// ================================================================================================

#ifndef DD_SAMPLES_COMMON_HPP
#define DD_SAMPLES_COMMON_HPP

// #define DD_SAMPLES_NOGL to exclude all OpenGL/GLFW
// related code (e.g.: for the D3D Win32 samples)

#include <cassert>
#include <cmath>
#include <cstdarg>
#include <cstddef>
#include <cstdint>
#include <cstdio>
#include <cstring>

#include <chrono>
#include <condition_variable>
#include <functional>
#include <mutex>
#include <queue>
#include <thread>
#include <vectormath.h>

#ifndef DD_SAMPLES_NOGL
    #include <GLFW/glfw3.h>
#endif // DD_SAMPLES_NOGL

namespace ddSamplesCommon
{

// App window dimensions; Not resizable.
static const int WindowWidth  = 1024;
static const int WindowHeight = 768;

// Angle in degrees to angle in radians for sin/cos/etc.
static inline float degToRad(const float degrees)
{
    return (degrees * 3.1415926535897931f / 180.0f);
}

#ifndef DD_SAMPLES_NOGL

// Time in milliseconds since the application started.
static inline std::int64_t getTimeMilliseconds()
{
    const double seconds = glfwGetTime();
    return static_cast<std::int64_t>(seconds * 1000.0);
}

// Time in seconds since the application started.
static inline double getTimeSeconds()
{
    return glfwGetTime();
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

#else // DD_SAMPLES_NOGL defined

using TimePoint = std::chrono::time_point<std::chrono::high_resolution_clock>;
static TimePoint startupTime = std::chrono::high_resolution_clock::now();

static inline std::int64_t getTimeMilliseconds()
{
    const auto currentTime = std::chrono::high_resolution_clock::now();
    return std::chrono::duration_cast<std::chrono::milliseconds>(currentTime - startupTime).count();
}

static inline double getTimeSeconds()
{
    return getTimeMilliseconds() * 0.001f;
}

#endif // DD_SAMPLES_NOGL

// Prints error to standard error stream.
static inline void errorF(const char * format, ...)
{
    va_list args;
    va_start(args, format);
    std::vfprintf(stderr, format, args);
    va_end(args);

    // Default newline and flush (like std::endl)
    std::fputc('\n', stderr);
    std::fflush(stderr);
}

// Prints some of the compile-time build settings to stdout.
static inline void printDDBuildConfig()
{
    std::printf("\n");

    #ifdef DEBUG_DRAW_CXX11_SUPPORTED
    std::printf("DEBUG_DRAW_CXX11_SUPPORTED    = %u\n", DEBUG_DRAW_CXX11_SUPPORTED);
    #endif // DEBUG_DRAW_CXX11_SUPPORTED

    #ifdef DEBUG_DRAW_PER_THREAD_CONTEXT
    std::printf("DEBUG_DRAW_PER_THREAD_CONTEXT = %u\n", /*DEBUG_DRAW_PER_THREAD_CONTEXT*/1);
    #endif // DEBUG_DRAW_PER_THREAD_CONTEXT

    #ifdef DEBUG_DRAW_EXPLICIT_CONTEXT
    std::printf("DEBUG_DRAW_EXPLICIT_CONTEXT   = %u\n", /*DEBUG_DRAW_EXPLICIT_CONTEXT*/1);
    #endif // DEBUG_DRAW_EXPLICIT_CONTEXT

    std::printf("DEBUG_DRAW_USE_STD_MATH       = %u\n", DEBUG_DRAW_USE_STD_MATH);
    std::printf("DEBUG_DRAW_MAX_STRINGS        = %u\n", DEBUG_DRAW_MAX_STRINGS);
    std::printf("DEBUG_DRAW_MAX_POINTS         = %u\n", DEBUG_DRAW_MAX_POINTS);
    std::printf("DEBUG_DRAW_MAX_LINES          = %u\n", DEBUG_DRAW_MAX_LINES);
    std::printf("DEBUG_DRAW_VERTEX_BUFFER_SIZE = %u\n", DEBUG_DRAW_VERTEX_BUFFER_SIZE);
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

struct Time
{
    float seconds;
    std::int64_t milliseconds;
} deltaTime;

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

    // Frustum planes for clipping:
    enum { A, B, C, D };
    Vector4 planes[6];

    // Tunable values:
    float movementSpeed = 3.0f;
    float lookSpeed     = 6.0f;

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
        const float aspect = static_cast<float>(WindowWidth) / static_cast<float>(WindowHeight);
        projMatrix = Matrix4::perspective(fovY, aspect, 0.1f, 1000.0f);

        for (int i = 0; i < 6; ++i)
        {
            planes[i] = Vector4(0.0f);
        }
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
        const float moveSpeed = movementSpeed * deltaTime.seconds;
        if (keys.aDown) { move(Camera::Left,    moveSpeed); }
        if (keys.dDown) { move(Camera::Right,   moveSpeed); }
        if (keys.wDown) { move(Camera::Forward, moveSpeed); }
        if (keys.sDown) { move(Camera::Back,    moveSpeed); }
    }

    void checkMouseRotation()
    {
        static const float maxAngle = 89.5f; // Max degrees of rotation
        static float pitchAmt;

        if (!mouse.leftButtonDown)
        {
            return;
        }

        const float rotateSpeed = lookSpeed * deltaTime.seconds;

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

        // Compute and normalize the 6 frustum planes:
        const float * const m = toFloatPtr(vpMatrix);
        planes[0][A] = m[3]  - m[0];
        planes[0][B] = m[7]  - m[4];
        planes[0][C] = m[11] - m[8];
        planes[0][D] = m[15] - m[12];
        planes[0] = normalize(planes[0]);
        planes[1][A] = m[3]  + m[0];
        planes[1][B] = m[7]  + m[4];
        planes[1][C] = m[11] + m[8];
        planes[1][D] = m[15] + m[12];
        planes[1] = normalize(planes[1]);
        planes[2][A] = m[3]  + m[1];
        planes[2][B] = m[7]  + m[5];
        planes[2][C] = m[11] + m[9];
        planes[2][D] = m[15] + m[13];
        planes[2] = normalize(planes[2]);
        planes[3][A] = m[3]  - m[1];
        planes[3][B] = m[7]  - m[5];
        planes[3][C] = m[11] - m[9];
        planes[3][D] = m[15] - m[13];
        planes[3] = normalize(planes[3]);
        planes[4][A] = m[3]  - m[2];
        planes[4][B] = m[7]  - m[6];
        planes[4][C] = m[11] - m[10];
        planes[4][D] = m[15] - m[14];
        planes[4] = normalize(planes[4]);
        planes[5][A] = m[3]  + m[2];
        planes[5][B] = m[7]  + m[6];
        planes[5][C] = m[11] + m[10];
        planes[5][D] = m[15] + m[14];
        planes[5] = normalize(planes[5]);
    }

    Point3 getTarget() const
    {
        return Point3(eye[0] + forward[0], eye[1] + forward[1], eye[2] + forward[2]);
    }

    bool isPointInsideFrustum(const float x, const float y, const float z) const
    {
        for (int i = 0; i < 6; ++i)
        {
            if ((planes[i][A] * x + planes[i][B] * y + planes[i][C] * z + planes[i][D]) <= 0.0f)
            {
                return false;
            }
        }
        return true;
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

#ifndef DD_SAMPLES_NOGL

// ========================================================
// Input callbacks for GLFW:
// ========================================================

static void mousePositionCallback(GLFWwindow * window, const double xPos, const double yPos)
{
    (void)window; // Unused.

    int mx = static_cast<int>(xPos);
    int my = static_cast<int>(yPos);

    // Clamp to window bounds:
    if      (mx > WindowWidth)  { mx = WindowWidth;  }
    else if (mx < 0)            { mx = 0;            }
    if      (my > WindowHeight) { my = WindowHeight; }
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

#endif // DD_SAMPLES_NOGL

// ========================================================
// MainThreadChecker - test if the calling thread is main()
// ========================================================

struct MainThreadChecker
{
    const std::thread::id mainThreadId;

    MainThreadChecker()
        : mainThreadId(std::this_thread::get_id())
    { }

    bool operator()() const
    {
        return std::this_thread::get_id() == mainThreadId;
    }
} isMainThread;

// ========================================================
// class JobQueue
// ========================================================

class JobQueue final
{
public:
    typedef std::function<void()> Job;

    // Wait for the worker thread to exit.
    ~JobQueue()
    {
        if (worker.joinable())
        {
            waitAll();
            mutex.lock();
            terminating = true;
            condition.notify_one();
            mutex.unlock();
            worker.join();
        }
    }

    // Launch the worker thread.
    void launch()
    {
        assert(!worker.joinable()); // Not already launched!
        worker = std::thread(&JobQueue::queueLoop, this);
    }

    // Add a new job to the thread's queue.
    void pushJob(Job job)
    {
        std::lock_guard<std::mutex> lock(mutex);
        queue.push(std::move(job));
        condition.notify_one();
    }

    // Wait until all work items have been completed.
    void waitAll()
    {
        std::unique_lock<std::mutex> lock(mutex);
        condition.wait(lock, [this]() { return queue.empty(); });
    }

private:
    void queueLoop()
    {
        for (;;)
        {
            Job job;
            {
                std::unique_lock<std::mutex> lock(mutex);
                condition.wait(lock, [this] { return !queue.empty() || terminating; });
                if (terminating)
                {
                    break;
                }
                job = queue.front();
            }

            job();

            {
                std::lock_guard<std::mutex> lock(mutex);
                queue.pop();
                condition.notify_one();
            }
        }
    }

    bool terminating = false;

    std::thread worker;
    std::queue<Job> queue;

    std::mutex mutex;
    std::condition_variable condition;
};

} // namespace ddSamplesCommon

#endif // DD_SAMPLES_COMMON_HPP

