/*
 *  SimpleRenderEngine (https://github.com/mortennobel/SimpleRenderEngine)
 *
 *  Created by Morten Nobel-Jørgensen ( http://www.nobel-joergensen.com/ )
 *  License: MIT
 */

#include "sre/WorldLights.hpp"

#include <algorithm>
#include <iostream>
#include <cassert>

using namespace std;

namespace sre {
    WorldLights::WorldLights(){
        setAmbientLight(glm::vec3(0.2,0.2,0.2));
    }

    int WorldLights::addLight(const Light & light){
        lights.push_back(light);
        return (int)lights.size()-1;
    }

    void WorldLights::removeLight(int index){
        assert(index < lights.size());
        lights.erase(lights.cbegin()+index);
    }

    Light* WorldLights::getLight(int index){
        if (index >= lights.size()){
            return nullptr;
        }
        return &(lights[index]);
    }

    int WorldLights::lightCount(){
        return (int)lights.size();
    }

    void WorldLights::setAmbientLight(const glm::vec3& ambientLight){
        this->ambientLight = glm::vec4(ambientLight,0.0);
    }

    glm::vec3 WorldLights::getAmbientLight() {
        return glm::vec3(ambientLight);
    }

    void WorldLights::clear() {
        lights.clear();
    }
}