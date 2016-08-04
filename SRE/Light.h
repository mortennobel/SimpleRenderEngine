#pragma once

#include "glm/glm.hpp"

namespace SRE {
    enum class LightType {
        Point,
        Directional
    };

    struct Light {
        LightType light;
        glm::vec3 position;
        glm::vec3 direction;
        glm::vec3 color;
        float range;

        Light();
        Light(LightType light, const glm::vec3 &position, const glm::vec3 &direction, const glm::vec3 &color,
              float range);
    };
}