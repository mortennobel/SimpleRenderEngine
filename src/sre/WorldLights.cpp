/*
 *  SimpleRenderEngine
 *
 *  Created by Morten Nobel-Jørgensen ( http://www.nobel-joergnesen.com/ )
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
        return lights.size()-1;
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
        return lights.size();
    }

    void WorldLights::setAmbientLight(const glm::vec3& ambientLight){
        this->ambientLight = ambientLight;
    }

    glm::vec3 WorldLights::getAmbientLight() {
        return ambientLight;
    }
}