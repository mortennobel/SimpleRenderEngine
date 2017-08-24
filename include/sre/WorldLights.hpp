/*
 *  SimpleRenderEngine
 *
 *  Created by Morten Nobel-Jørgensen ( http://www.nobel-joergnesen.com/ )
 *  License: MIT
 */

#pragma once

#include "Light.hpp"
#include <vector>

#include "sre/impl/Export.hpp"

namespace sre {
    class DllExport WorldLights {
    public:
        WorldLights();                                      // Create world light
        int addLight(const Light & light);                  // Add light
        void removeLight(int index);                        // Remove light by index
        Light* getLight(int index);                         // Get light at position
        int lightCount();                                   // The number of lights
        void setAmbientLight(const glm::vec3& light);       // Set ambient light
        glm::vec3 getAmbientLight();                        // Get ambient light
    private:
        glm::vec3 ambientLight;
        std::vector<Light> lights;

        friend class Shader;
    };
}