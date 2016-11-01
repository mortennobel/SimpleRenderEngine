#pragma once


#include "SRE/impl/Export.hpp"

namespace SRE {
    /**
     * Defines the types of Light. Note ambient light is simply represented using a vec3 in the SimpleRenderEngine class.
     */
    enum class LightType {
        Point,
        Directional,
        Unused
    };
}