#pragma once

/// Include GL header

#if defined(_WIN32)
#   define GLEW_STATIC
#   include <GL/glew.h>
extern "C"
{
	__declspec(dllexport) unsigned long NvOptimusEnablement = 0x00000001;
	__declspec(dllexport) int AmdPowerXpressRequestHighPerformance = 1;
}
#elif defined __linux__
#   include <GL/glew.h>
#else
#   include <OpenGL/gl3.h>
#endif