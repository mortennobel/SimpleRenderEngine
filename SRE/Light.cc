//
// Created by morten on 31/07/16.
//

#include "Light.h"

namespace SRE{
    Light::Light(){

    }

    Light::Light(LightType light, const glm::vec3 &position, const glm::vec3 &direction, const glm::vec3 &color,
                 float range) : light(light), position(position), direction(direction), color(color), range(range) {}
}