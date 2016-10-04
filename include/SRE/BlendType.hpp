#pragma once

#include "SRE/Export.hpp"

namespace SRE {
    /**
     * Enum which defines blending types. Currently support types of blending is:
     *  - BlendType::Disabled - no blending
     *  - BlendType::AlphaBlending - src minus one alpha
     */
    enum class BlendType {
        Disabled,
        AlphaBlending
    };
}