/*
 *  SimpleRenderEngine (https://github.com/mortennobel/SimpleRenderEngine)
 *
 *  Created by Morten Nobel-Jørgensen ( http://www.nobel-joergnesen.com/ )
 *  License: MIT
 */

#pragma once

#if _MSC_VER
#define DEPRECATED(X) __declspec(deprecated(X))
#else
#define DEPRECATED(X) [[deprecated(X)]]
#endif

// platform independent way to pack structs
// https://stackoverflow.com/a/3312896
// usage PACK(
// struct myStruct
// {
// 	int a;
// 	int b;
// });
#ifdef _MSC_VER
#define PACK( __Declaration__ ) __pragma( pack(push, 1) ) __Declaration__ __pragma( pack(pop) )
#else
#define PACK( __Declaration__ ) __Declaration__ __attribute__((__packed__))
#endif