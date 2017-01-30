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
        class DllExport LightBuilder {
        public:
            ~LightBuilder();
            LightBuilder& withPointLight(glm::vec3 position);
            LightBuilder& withDirectionalLight(glm::vec3 direction);
            // light color.
            LightBuilder& withColor(glm::vec3 color);
            // range only valid using point light
            LightBuilder& withRange(float range);
            Light build();
        private:
            LightBuilder();

            Light *light;
            friend class Light;
        };
        // Create light using builder pattern
        static LightBuilder create();
        // create light of type unused
        Light();

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
        // Range0 means no attenuation
        float range;
    private:

    };
}