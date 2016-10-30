//
// Created by morten on 31/07/16.
//

#include "SRE/Light.hpp"

namespace SRE{
    Light::Light()
    :lightType(LightType::Unused)
    {

    }

    Light::Light(LightType lightType, glm::vec3 position, glm::vec3 direction, glm::vec3 color, float range, float specularity)
            :Light(lightType, position, direction, color, range)
    {
    }

    Light::Light(LightType lightType, glm::vec3 position, glm::vec3 direction, glm::vec3 color, float range)
            : lightType(lightType), position(position), direction(direction), color(color), range(range) {

    }
}