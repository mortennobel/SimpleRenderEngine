#pragma once

#include "glm/glm.hpp"

#ifdef _WIN32
#define DllExport __declspec(dllexport)
template struct __declspec(dllexport) glm::tvec4<float, glm::precision(0)>;
template struct __declspec(dllexport) glm::tvec3<float, glm::precision(0)>;
template struct __declspec(dllexport) glm::tvec2<float, glm::precision(0)>;
template struct __declspec(dllexport) glm::tmat4x4<float, glm::precision(0)>;
template struct __declspec(dllexport) glm::tmat3x3<float, glm::precision(0)>;
#else
#define DllExport 
#endif // _WIN32