/*
 *  SimpleRenderEngine (https://github.com/mortennobel/SimpleRenderEngine)
 *
 *  Created by Morten Nobel-Jørgensen ( http://www.nobel-joergnesen.com/ )
 *  License: MIT
 */

#pragma once


#include "sre/impl/Export.hpp"

namespace sre {
    /**
     * Defines the types of Light. Note ambient light is simply represented using a vec3 in the SimpleRenderEngine class.
     */
    enum class LightType {
        Point,
        Directional,
        Unused
    };
}