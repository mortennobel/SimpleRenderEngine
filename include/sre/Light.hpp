/*
 *  SimpleRenderEngine
 *
 *  Created by Morten Nobel-Jørgensen ( http://www.nobel-joergnesen.com/ )
 *  License: MIT
 */

#pragma once

#include "glm/glm.hpp"
#include "LightType.hpp"
#include "sre/impl/CPPShim.hpp"

#include "sre/impl/Export.hpp"

namespace sre {
    /**
     * Contains information about a Light source
     */
	class DllExport Light {
	public:
        class DllExport LightBuilder {
        public:
            ~LightBuilder();
            LightBuilder& withPointLight(glm::vec3 position);           // Using point light with a worldspace position
            LightBuilder& withDirectionalLight(glm::vec3 direction);    // Using directional light with a worldspace direction
            LightBuilder& withColor(glm::vec3 color);                   // light color - note intensity can extend 1.0
            LightBuilder& withRange(float range);                       // range only valid using point light
            Light build();
        private:
            LightBuilder();

            Light *light;
            friend class Light;
        };
        static LightBuilder create();       // Create light using builder pattern

        Light();                            // create light of type unused

        LightType lightType;                // Defines the type of light source (note: ambient light is stored as a vec3 directly in SimpleRenderEngine)
                                            // LightType::Point,
                                            // LightType::Directional,
                                            // LightType::Unused

        glm::vec3 position;                 // position in worldspace
                                            // only used for point lights

        glm::vec3 direction;                // direction towards the lightsource
                                            // only used for directional light

        glm::vec3 color;                    // The color (or intensity) of the light
                                            // In some cases the light color may have values above 1.0

        float range;                        // The range of a point light (due to attenuation)
                                            // Range == 0 means no attenuation
    };
}