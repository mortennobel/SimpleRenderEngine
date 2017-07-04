#pragma once

/// Simple header which wraps C++14 features to MSVC


#if _MSC_VER
#define DEPRECATED(X) __declspec(deprecated(X))
#else
#define DEPRECATED(X) [[deprecated(X)]]
#endif