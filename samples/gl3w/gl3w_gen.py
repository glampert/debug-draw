#!/usr/bin/env python

#   This file is part of gl3w, hosted at https://github.com/skaslev/gl3w
#
#   This is free and unencumbered software released into the public domain.
#
#   Anyone is free to copy, modify, publish, use, compile, sell, or
#   distribute this software, either in source code form or as a compiled
#   binary, for any purpose, commercial or non-commercial, and by any
#   means.
#
#   In jurisdictions that recognize copyright laws, the author or authors
#   of this software dedicate any and all copyright interest in the
#   software to the public domain. We make this dedication for the benefit
#   of the public at large and to the detriment of our heirs and
#   successors. We intend this dedication to be an overt act of
#   relinquishment in perpetuity of all present and future rights to this
#   software under copyright law.
#
#   THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
#   EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
#   MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
#   IN NO EVENT SHALL THE AUTHORS BE LIABLE FOR ANY CLAIM, DAMAGES OR
#   OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
#   ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
#   OTHER DEALINGS IN THE SOFTWARE.

# Allow Python 2.6+ to use the print() function
from __future__ import print_function

import re
import os

# Try to import Python 3 library urllib.request
# and if it fails, fall back to Python 2 urllib2
try:
    import urllib.request as urllib2
except ImportError:
    import urllib2

# UNLICENSE copyright header
UNLICENSE = br'''/*

    This file was generated with gl3w_gen.py, part of gl3w
    (hosted at https://github.com/skaslev/gl3w)

    This is free and unencumbered software released into the public domain.

    Anyone is free to copy, modify, publish, use, compile, sell, or
    distribute this software, either in source code form or as a compiled
    binary, for any purpose, commercial or non-commercial, and by any
    means.

    In jurisdictions that recognize copyright laws, the author or authors
    of this software dedicate any and all copyright interest in the
    software to the public domain. We make this dedication for the benefit
    of the public at large and to the detriment of our heirs and
    successors. We intend this dedication to be an overt act of
    relinquishment in perpetuity of all present and future rights to this
    software under copyright law.

    THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
    EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
    MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
    IN NO EVENT SHALL THE AUTHORS BE LIABLE FOR ANY CLAIM, DAMAGES OR
    OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
    ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
    OTHER DEALINGS IN THE SOFTWARE.

*/

'''

# Create directories
if not os.path.exists('include/GL'):
    os.makedirs('include/GL')
if not os.path.exists('src'):
    os.makedirs('src')

# Download glcorearb.h
if not os.path.exists('include/GL/glcorearb.h'):
    print('Downloading glcorearb.h to include/GL...')
    web = urllib2.urlopen('https://www.opengl.org/registry/api/GL/glcorearb.h')
    with open('include/GL/glcorearb.h', 'wb') as f:
        f.writelines(web.readlines())
else:
    print('Reusing glcorearb.h from include/GL...')

# Parse function names from glcorearb.h
print('Parsing glcorearb.h header...')
procs = []
p = re.compile(r'GLAPI.*APIENTRY\s+(\w+)')
with open('include/GL/glcorearb.h', 'r') as f:
    for line in f:
        m = p.match(line)
        if m:
            procs.append(m.group(1))
procs.sort()

def proc_t(proc):
    return { 'p': proc,
             'p_s': 'gl3w' + proc[2:],
             'p_t': 'PFN' + proc.upper() + 'PROC' }

# Generate gl3w.h
print('Generating gl3w.h in include/GL...')
with open('include/GL/gl3w.h', 'wb') as f:
    f.write(UNLICENSE)
    f.write(br'''#ifndef __gl3w_h_
#define __gl3w_h_

#include <GL/glcorearb.h>

#ifndef __gl_h_
#define __gl_h_
#endif

typedef void (*GL3WglProc)(void);

/* gl3w API: */
int gl3wInit(void);
void gl3wShutdown(void);
int gl3wIsSupported(int major, int minor);
GL3WglProc gl3wGetProcAddress(const char *proc);

/* OpenGL functions: */
''')
    for proc in procs:
        f.write('extern {0[p_t]: <52} {0[p_s]};\n'.format(proc_t(proc)).encode("utf-8"))
    f.write(b'\n')
    for proc in procs:
        f.write('#define {0[p]: <51} {0[p_s]}\n'.format(proc_t(proc)).encode("utf-8"))
    f.write(br'''
#endif // __gl3w_h_
''')

# Generate gl3w.cpp
print('Generating gl3w.cpp in src...')
with open('src/gl3w.cpp', 'wb') as f:
    f.write(UNLICENSE)
    f.write(br'''#include <GL/gl3w.h>

/* --------------------------------------------------------------------------------------------- */

#ifdef _WIN32

/* ------------------------------------
 * Windows
 * ------------------------------------ */

#define WIN32_LEAN_AND_MEAN 1
#include <windows.h>

static HMODULE gl3w_libgl = NULL;

static int gl3w_open_libgl(void)
{
    if (!gl3w_libgl)
    {
        gl3w_libgl = LoadLibraryA("opengl32.dll");
    }

    return gl3w_libgl != NULL;
}

static void gl3w_close_libgl(void)
{
    if (gl3w_libgl)
    {
        FreeLibrary(gl3w_libgl);
        gl3w_libgl = NULL;
    }
}

static GL3WglProc gl3w_fn(const char *proc)
{
    GL3WglProc res;
    res = (GL3WglProc) wglGetProcAddress(proc);

    if (!res)
    {
        res = (GL3WglProc) GetProcAddress(gl3w_libgl, proc);
    }

    return res;
}

#elif defined(__APPLE__) || defined(__APPLE_CC__)

/* ------------------------------------
 * Mac OS
 * ------------------------------------ */

#include <Carbon/Carbon.h>

static CFBundleRef gl3w_cfBundle = NULL;
static CFURLRef gl3w_cfBundleURL = NULL;

static int gl3w_open_libgl(void)
{
    if (gl3w_cfBundle)
    {
        return 1; /* Already init */
    }

    gl3w_cfBundleURL = CFURLCreateWithFileSystemPath(
                            kCFAllocatorDefault,
                            CFSTR("/System/Library/Frameworks/OpenGL.framework"),
                            kCFURLPOSIXPathStyle, true);
    if (!gl3w_cfBundleURL)
    {
        return 0;
    }

    gl3w_cfBundle = CFBundleCreate(kCFAllocatorDefault, gl3w_cfBundleURL);
    if (!gl3w_cfBundle)
    {
        CFRelease(gl3w_cfBundleURL);
        return 0;
    }

    return 1;
}

static void gl3w_close_libgl(void)
{
    if (gl3w_cfBundle)
    {
        CFRelease(gl3w_cfBundle);
        gl3w_cfBundle = NULL;
    }
    if (gl3w_cfBundleURL)
    {
        CFRelease(gl3w_cfBundleURL);
        gl3w_cfBundleURL = NULL;
    }
}

static GL3WglProc gl3w_fn(const char *proc)
{
    GL3WglProc res;
    CFStringRef procName;

    procName = CFStringCreateWithCString(kCFAllocatorDefault, proc, kCFStringEncodingASCII);
    res = (GL3WglProc) CFBundleGetFunctionPointerForName(gl3w_cfBundle, procName);
    CFRelease(procName);

    return res;
}

#else

/* ------------------------------------
 * GLX
 * ------------------------------------ */

#include <dlfcn.h>
#include <GL/glx.h>

static void *gl3w_libgl = NULL;

static int gl3w_open_libgl(void)
{
    if (!gl3w_libgl)
    {
        gl3w_libgl = dlopen("libGL.so.1", RTLD_LAZY | RTLD_GLOBAL);
    }

    return gl3w_libgl != NULL;
}

static void gl3w_close_libgl(void)
{
    if (gl3w_libgl)
    {
        dlclose(gl3w_libgl);
        gl3w_libgl = NULL;
    }
}

static GL3WglProc gl3w_fn(const char *proc)
{
    GL3WglProc res;
    res = (GL3WglProc) glXGetProcAddress((const GLubyte *) proc);

    if (!res)
    {
        res = (GL3WglProc) dlsym(gl3w_libgl, proc);
    }

    return res;
}

#endif

/* ------------------------------------
 * GL3W API
 * ------------------------------------ */

static struct {
	int major, minor;
} gl3w_version;

static void gl3w_load_all_functions(void);

static int gl3w_parse_version(void)
{
    if (!glGetIntegerv)
    {
        return 0;
    }

    glGetIntegerv(GL_MAJOR_VERSION, &gl3w_version.major);
    glGetIntegerv(GL_MINOR_VERSION, &gl3w_version.minor);

    if (gl3w_version.major < 3)
    {
        return 0;
    }

    return 1;
}

int gl3wInit(void)
{
    if (!gl3w_open_libgl())
    {
        return 0;
    }

    gl3w_load_all_functions();
    return gl3w_parse_version();
}

void gl3wShutdown(void)
{
    gl3w_close_libgl();
}

int gl3wIsSupported(int major, int minor)
{
    if (major < 3)
    {
        return 0;
    }
    if (gl3w_version.major == major)
    {
        return gl3w_version.minor >= minor;
    }
    return gl3w_version.major >= major;
}

GL3WglProc gl3wGetProcAddress(const char *proc)
{
    if (!proc)
    {
        return NULL;
    }
    return gl3w_fn(proc);
}

/* --------------------------------------------------------------------------------------------- */

''')
    for proc in procs:
        f.write('{0[p_t]: <52} {0[p_s]};\n'.format(proc_t(proc)).encode("utf-8"))
    f.write(br'''
/* --------------------------------------------------------------------------------------------- */

static void gl3w_load_all_functions(void)
{
''')
    for proc in procs:
        f.write('\t{0[p_s]: <47} = ( {0[p_t]: <52} ) gl3w_fn("{0[p]}");\n'.format(proc_t(proc)).encode("utf-8"))
    f.write(b'}\n')
