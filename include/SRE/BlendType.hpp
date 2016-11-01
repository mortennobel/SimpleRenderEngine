#pragma once

#include "SRE/impl/Export.hpp"

namespace SRE {
    /**
     * Enum which defines blending types. Currently support types of blending is:
     *  - BlendType::Disabled - No blending
     *  - BlendType::AlphaBlending - Blends the surface with background based on alpha value (src alpha ,one minus src alpha)
     *  - BlendType::AdditiveBlending - Add light based on the alpha value (src alpha,one)
     */
    enum class BlendType {
        Disabled,
        AlphaBlending,
        AdditiveBlending
    };
}