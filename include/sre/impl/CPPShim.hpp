/*
 *  SimpleRenderEngine
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