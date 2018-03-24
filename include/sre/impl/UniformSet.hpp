/*
 *  SimpleRenderEngine (https://github.com/mortennobel/SimpleRenderEngine)
 *
 *  Created by Morten Nobel-Jørgensen ( http://www.nobel-joergensen.com/ )
 *  License: MIT
 */

#pragma once

#include "sre/Texture.hpp"
#include "sre/Color.hpp"
#include "glm/glm.hpp"
#include <string>
#include <map>

namespace sre {
    class UniformSet {
    public:
        void set(int id, glm::vec4 value);

        void set(int id, float value);

        void set(int id, std::shared_ptr<Texture> value);

        void set(int id, std::shared_ptr<std::vector<glm::mat3>> value);

        void set(int id, std::shared_ptr<std::vector<glm::mat4>> value);

        void set(int id, Color value);

        void clear();

        void bind();

        template<typename T>
        inline T get(int id);

    private:
        std::map<int,std::shared_ptr<sre::Texture>> textureValues;
        std::map<int,glm::vec4> vectorValues;
        std::map<int,std::shared_ptr<std::vector<glm::mat4>>> mat4Values;
        std::map<int,std::shared_ptr<std::vector<glm::mat3>>> mat3Values;
        std::map<int,float> floatValues;

        friend class Material;
    };

    template<>
    inline std::shared_ptr<sre::Texture> UniformSet::get(int id) {
        return textureValues[id];
    }

    template<>
    inline glm::vec4 UniformSet::get(int id)  {
        return vectorValues[id];
    }

    template<>
    inline Color UniformSet::get(int id)  {
        Color value;
        value.setFromLinear(vectorValues[id]);
        return value;
    }

    template<>
    inline float UniformSet::get(int id) {
        return floatValues[id];
    }
}