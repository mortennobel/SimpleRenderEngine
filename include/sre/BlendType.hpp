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