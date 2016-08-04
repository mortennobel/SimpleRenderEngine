//
// Created by morten on 31/07/16.
//

#include "Light.h"

namespace SRE{
    Light::Light(){

    }

    Light::Light(LightType lightType, const glm::vec3 &position, const glm::vec3 &direction, const glm::vec3 &color,
                 float range) : lightType(lightType), position(position), direction(direction), color(color), range(range) {}
}