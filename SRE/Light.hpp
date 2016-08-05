#pragma once

#include "glm/glm.hpp"
#include "LightType.hpp"

namespace SRE {
    struct Light {
        LightType lightType;
        // position in worldspace
        glm::vec3 position;
        // direction towards the lightsource
        glm::vec3 direction;
        glm::vec3 color;
        float range;
        // 0 = no specular
        float specularity;

        Light();
        Light(LightType lightType, glm::vec3 position, glm::vec3 direction, glm::vec3 color, float range, float specularity);
    };
}