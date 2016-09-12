#pragma once

#ifdef _WIN32
#define DllExport __declspec(dllexport)
#else
#define DllExport 
#endif // _WIN32