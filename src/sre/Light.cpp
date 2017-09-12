/*
 *  SimpleRenderEngine (https://github.com/mortennobel/SimpleRenderEngine)
 *
 *  Created by Morten Nobel-Jørgensen ( http://www.nobel-joergnesen.com/ )
 *  License: MIT
 */

#include "sre/Light.hpp"

namespace sre{
    Light::Light()
    :lightType(LightType::Unused),
     position(glm::vec3(0,0,0)),
     direction(glm::vec3(0,0,0)),
     color(glm::vec3(1,1,1)),
     range(100.0f)
    {
    }

    Light::LightBuilder Light::create() {
        return Light::LightBuilder();
    }


    Light Light::LightBuilder::build() {
        Light res = *light;
        return res;
    }

    Light::LightBuilder& Light::LightBuilder::withPointLight(glm::vec3 position) {
        light->lightType = LightType::Point;
        light->position = position;
        return *this;
    }

    Light::LightBuilder& Light::LightBuilder::withDirectionalLight(glm::vec3 direction) {
        light->lightType = LightType::Directional;
        light->direction = glm::normalize(direction);
        return *this;
    }

    Light::LightBuilder& Light::LightBuilder::withColor(glm::vec3 color) {
        light->color = color;
        return *this;
    }

    Light::LightBuilder& Light::LightBuilder::withRange(float range) {
        light->range = range;
        return *this;
    }

    Light::LightBuilder::LightBuilder()
    :light{new Light()}
    {
    }

    Light::LightBuilder::~LightBuilder() {
        delete light;
    }

}