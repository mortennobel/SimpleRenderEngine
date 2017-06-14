//
// Created by Morten Nobel-Jørgensen on 12/06/2017.
//

#include "sre/WorldLights.hpp"

#include <algorithm>
#include <iostream>
#include <cassert>

using namespace std;

namespace sre {
    int WorldLights::addLight(const Light & light){
        lights.push_back(light);
        return lights.size()-1;
    }

    void WorldLights::removeLight(int index){
        assert(index < lights.size());
        lights.erase(lights.begin()+index);
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
        float maxAmbient = std::max(ambientLight.x, std::max(ambientLight.y,ambientLight.z));
        this->ambientLight = glm::vec4(ambientLight, maxAmbient);
    }
}