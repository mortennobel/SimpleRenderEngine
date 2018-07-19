/*
 *  SimpleRenderEngine (https://github.com/mortennobel/SimpleRenderEngine)
 *
 *  Created by Morten Nobel-Jørgensen ( http://www.nobel-joergensen.com/ )
 *  License: MIT
 */

#pragma once

/// Include GL header

#if defined(EMSCRIPTEN)
#   include <GLES3/gl3.h>
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
void checkGLError(const char* title = nullptr);

bool getMaximumOpenGLSupport(int * major, int * minor);

bool hasExtension(const std::string& extensionName);
std::vector<std::string> listExtension();

bool has_sRGB();
