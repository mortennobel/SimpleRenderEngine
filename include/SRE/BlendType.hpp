#pragma once

#include "SRE/Export.hpp"

namespace SRE {
    /**
     * Enum which defines blending types. Currently support types of blending is:
     *  - BlendType::Disabled - no blending
     *  - BlendType::AlphaBlending - src minus one alpha
     *  - BlendType::AdditiveBlending - one alpha, one
     */
    enum class BlendType {
        Disabled,
        AlphaBlending,
        AdditiveBlending
    };
}