#pragma once

#include "glm/glm.hpp"

#ifdef _WIN32

#define DECLSPECIFIER __declspec(dllexport)
#define DllExport __declspec(dllexport)
#define EXPIMP_TEMPLATE

template struct __declspec(dllexport) glm::tvec4<float, glm::precision(0)>;
template struct __declspec(dllexport) glm::tvec3<float, glm::precision(0)>;
template struct __declspec(dllexport) glm::tvec2<float, glm::precision(0)>;
template struct __declspec(dllexport) glm::tmat4x4<float, glm::precision(0)>;
template struct __declspec(dllexport) glm::tmat3x3<float, glm::precision(0)>;

// https://support.microsoft.com/en-us/kb/168958
//disable warnings on 255 char debug symbols
#pragma warning (disable : 4786)
//disable warnings on extern before template instantiation
#pragma warning (disable : 4231)

#include <vector>

EXPIMP_TEMPLATE template class DECLSPECIFIER std::vector<int>;
EXPIMP_TEMPLATE template class DECLSPECIFIER std::vector<float>;
EXPIMP_TEMPLATE template class DECLSPECIFIER std::vector<uint16_t>;
EXPIMP_TEMPLATE template class DECLSPECIFIER std::vector<glm::vec2>;
EXPIMP_TEMPLATE template class DECLSPECIFIER std::vector<glm::vec3>;
EXPIMP_TEMPLATE template class DECLSPECIFIER std::vector<glm::vec4>;

#else
#define DllExport 
#endif // _WIN32