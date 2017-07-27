/*
 *  SimpleRenderEngine
 *
 *  Created by Morten Nobel-Jørgensen ( http://www.nobel-joergnesen.com/ )
 *  License: MIT
 */

#pragma once

/// Include GL header

#if defined(EMSCRIPTEN)
#   include <GLES2/gl2.h>
#elif defined(_WIN32)
#   define GLEW_STATIC
#   include <GL/glew.h>
#elif defined __linux__
#   include <GL/glew.h>
#else
#   include  <OpenGL/gl3.h>
#endif
#include <string>
#include <vector>

// For internal debugging of gl errors
inline void checkGLError();

inline bool hasExtension(std::string extensionName);
inline std::vector<std::string> listExtension();

inline bool has_sRGB();

#include "GL.inl"