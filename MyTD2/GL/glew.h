#pragma once

#ifdef __APPLE__
#include <OpenGL/gl3.h>
#include <OpenGL/gl3ext.h>
#else
#error "Local GL/glew.h shim is only configured for macOS."
#endif
