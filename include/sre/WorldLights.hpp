//
// Created by Morten Nobel-Jørgensen on 12/06/2017.
//

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
    private:
        glm::vec3 ambientLight;
        std::vector<Light> lights;

        friend class Shader;
    };
}