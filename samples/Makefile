
#------------------------------------------------
# Brief: Makefile for the Debug Draw samples.
#
# Remarks:
# - Tested only on Mac OSX; Should work on Linux.
# - GLFW must be installed on the system.
#   (http://www.glfw.org)
#------------------------------------------------

# Select the proper OpenGL library for Mac (-framework OpenGL)
# or use a default (-lGL) that should work on most Unix-like systems.
# This script also assumes GLFW is installed and available in the system path.
UNAME = $(shell uname -s)
ifeq ($(UNAME), Darwin)
  OPENGL_LIB = -framework CoreFoundation -framework OpenGL
else
  OPENGL_LIB = -lGL
endif

# GLFW Should be installed and visible in the system path!
ifeq ($(UNAME), Darwin)
  CXXFLAGS = -Weffc++
  GLFW_LIB = -L/usr/local/lib -lglfw3
endif
ifeq ($(UNAME), Linux)
  CXXFLAGS = `pkg-config --cflags glfw3`
  GLFW_LIB = `pkg-config --static --libs glfw3`
endif

# Define 'VERBOSE' to get the full console output.
# Otherwise we print a shorter message for each rule.
ifndef VERBOSE
  QUIET          = @
  ECHO_COMPILING = @echo "-> Building Debug Draw samples ..."
  ECHO_CLEANING  = @echo "-> Cleaning ..."
endif

#------------------------------------------------
# Macros / source files:
#------------------------------------------------

LIBRARIES = $(OPENGL_LIB) $(GLFW_LIB)
INC_DIRS  = -I. -I.. -Ivectormath -Igl3w/include
CXXFLAGS += $(INC_DIRS) -O3 -Wall -Wextra -Winit-self -Wunused -Wshadow
CFLAGS    = $(CXXFLAGS)

SRC_FILES_GL_LEGACY_SAMP  = sample_gl_legacy.cpp
BIN_TARGET_GL_LEGACY_SAMP = sample_gl_legacy

SRC_FILES_GL_CORE_SAMP  = gl3w/src/gl3w.cpp sample_gl_core.cpp
BIN_TARGET_GL_CORE_SAMP = sample_gl_core

SRC_FILES_NULL_SAMP  = sample_null_renderer.cpp
BIN_TARGET_NULL_SAMP = sample_null_renderer

#------------------------------------------------
# Build rules:
#------------------------------------------------

all:
	$(ECHO_COMPILING)
	$(QUIET) $(CXX) $(CXXFLAGS) $(SRC_FILES_NULL_SAMP)      -o $(BIN_TARGET_NULL_SAMP)
	$(QUIET) $(CXX) $(CXXFLAGS) $(SRC_FILES_GL_LEGACY_SAMP) -o $(BIN_TARGET_GL_LEGACY_SAMP) $(LIBRARIES)
	$(QUIET) $(CXX) $(CXXFLAGS) $(SRC_FILES_GL_CORE_SAMP)   -o $(BIN_TARGET_GL_CORE_SAMP)   $(LIBRARIES)

clean:
	$(ECHO_CLEANING)
	$(QUIET) rm -f $(BIN_TARGET_NULL_SAMP)
	$(QUIET) rm -f $(BIN_TARGET_GL_LEGACY_SAMP)
	$(QUIET) rm -f $(BIN_TARGET_GL_CORE_SAMP)

