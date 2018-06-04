/*
 *  SimpleRenderEngine (https://github.com/mortennobel/SimpleRenderEngine)
 *
 *  Created by Morten Nobel-Jørgensen ( http://www.nobel-joergensen.com/ )
 *  License: MIT
 */

#pragma once

#include "glm/glm.hpp"

#ifdef _WIN32

#define DECLSPECIFIER __declspec(dllexport)
#define DllExport __declspec(dllexport)
#define EXPIMP_TEMPLATE

// https://support.microsoft.com/en-us/kb/168958
//disable warnings on 255 char debug symbols
#pragma warning (disable : 4786)
//disable warnings on extern before template instantiation
#pragma warning (disable : 4231)

#include <vector>


#else
#define DllExport 
#endif // _WIN32