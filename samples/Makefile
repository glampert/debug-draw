
#------------------------------------------------
# Brief: Makefile for the Debug Draw samples.
#
# Remarks:
# - Tested only on Mac OSX.
# - GLFW must be installed in the system.
#------------------------------------------------

# Select the proper OpenGL library for Mac (-framework OpenGL)
# or use a default (-lGL) that should work on most Unix-like systems.
# This script also assumes GLFW is installed and available in the system path.
UNAME = $(shell uname)
ifeq ($(UNAME), Darwin)
  OPENGL_LIB = -framework CoreFoundation -framework OpenGL
else
  OPENGL_LIB = -lGL
endif

# Should be installed and visible in the system path!
GLFW_LIB = -lGLFW3

# Define 'VERBOSE' to get the full console output.
# Otherwise we print a shorter message for each rule.
ifndef VERBOSE
  QUIET          = @
  ECHO_COMPILING = @echo "-> Building OpenGL samples ..."
  ECHO_CLEANING  = @echo "-> Cleaning ..."
endif

#------------------------------------------------
# Macros / source files:
#------------------------------------------------

LIB_FILES = $(OPENGL_LIB) $(GLFW_LIB)
INC_DIRS  = -I. -I.. -Ivectormath -Igl3w/include
CXXFLAGS  = $(INC_DIRS) -Wall -Wextra -Weffc++ -Winit-self -Wunused -Wshadow

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
	$(QUIET) $(CXX) $(CXXFLAGS) $(LIB_FILES) $(SRC_FILES_NULL_SAMP)      -o $(BIN_TARGET_NULL_SAMP)
	$(QUIET) $(CXX) $(CXXFLAGS) $(LIB_FILES) $(SRC_FILES_GL_LEGACY_SAMP) -o $(BIN_TARGET_GL_LEGACY_SAMP)
	$(QUIET) $(CXX) $(CXXFLAGS) $(LIB_FILES) $(SRC_FILES_GL_CORE_SAMP)   -o $(BIN_TARGET_GL_CORE_SAMP)

clean:
	$(ECHO_CLEANING)
	$(QUIET) rm -f $(BIN_TARGET_NULL_SAMP)
	$(QUIET) rm -f $(BIN_TARGET_GL_LEGACY_SAMP)
	$(QUIET) rm -f $(BIN_TARGET_GL_CORE_SAMP)

