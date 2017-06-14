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
        int addLight(const Light & light);
        void removeLight(int index);
        Light* getLight(int index);
        int lightCount();

        void setAmbientLight(const glm::vec3& light);
    private:
        glm::vec4 ambientLight;
        std::vector<Light> lights;

        friend class Shader;
    };
}