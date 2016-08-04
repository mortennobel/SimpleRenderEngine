#pragma once

#include "glm/glm.hpp"
#include "LightType.h"

namespace SRE {
    struct Light {
        LightType lightType;
        glm::vec3 position;
        glm::vec3 direction;
        glm::vec3 color;
        float range;

        Light();
        Light(LightType lightType, const glm::vec3 &position, const glm::vec3 &direction, const glm::vec3 &color,
              float range);
    };
}