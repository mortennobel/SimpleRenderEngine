#pragma once

#include "glm/glm.hpp"
#include "LightType.hpp"
#include "SRE/impl/CPPShim.hpp"

#include "SRE/impl/Export.hpp"

namespace SRE {
    /**
     * Contains information about a Light source
     */
    struct DllExport Light {
        /// Defines the type of light source (note: ambient light is stored as a vec3 directly in SimpleRenderEngine)
        /// LightType::Point,
        /// LightType::Directional,
        /// LightType::Unused
        LightType lightType;
        // position in worldspace
        // only used for point lights
        glm::vec3 position;
        // direction towards the lightsource
        // only used for directional light
        glm::vec3 direction;
        // The color (or intensity) of the light
        // In some cases the light color may have values above 1.0
        glm::vec3 color;
        // The range of a point light (due to attenuation)
        float range;
        Light();
        Light(LightType lightType, glm::vec3 position, glm::vec3 direction, glm::vec3 color, float range);

        DEPRECATED("Use Light constructor without specularity. Specularity has been moved to a shader uniform")
        Light(LightType lightType, glm::vec3 position, glm::vec3 direction, glm::vec3 color, float range, float specularity);
    };
}