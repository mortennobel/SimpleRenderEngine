//
// Created by morten on 31/07/16.
//

#include "Light.hpp"

namespace SRE{
    Light::Light(){

    }

    Light::Light(LightType lightType, glm::vec3 position, glm::vec3 direction, glm::vec3 color, float range, float specularity)
            : lightType(lightType), position(position), direction(direction), color(color), range(range), specularity{specularity} {

    }
}