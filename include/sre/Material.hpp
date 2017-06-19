#pragma once

#include "sre/Shader.hpp"
#include "sre/Texture.hpp"
#include "glm/glm.hpp"

#include <string>
#include <map>
#include <iostream>
#include <vector>

namespace sre {

    class Shader;
    class Texture;

    class DllExport Material {
    public:
        Material();
        Material(Shader* shader);
        ~Material();

        void bind();

        sre::Shader *getShader();

        void setShader(sre::Shader *shader);

        const std::string &getName();

        void setName(const std::string &name);

        // uniform parameters
        glm::vec4 getColor();

        bool setColor(const glm::vec4 &color);

        float getSpecularity();

        bool setSpecularity(float specularity);

        sre::Texture *getTexture();

        bool setTexture(sre::Texture *texture);

        bool set(std::string uniformName, glm::vec4 value);
        bool set(std::string uniformName, float value);
        bool set(std::string uniformName, sre::Texture *);

        template<typename T>
        inline T get(std::string uniformName);

        bool deleteUniform(std::string uniformName);

        std::vector<std::string> getUniformNames();

        UniformType getUniformType(std::string uniformName);
    private:
        std::string name;
        sre::Shader *shader;

        template<typename T>
        struct DllExport Uniform {
            int id;
            T value;
        };

        std::vector<Uniform<Texture*>> textureValues;
        std::vector<Uniform<glm::vec4>> vectorValues;
        std::vector<Uniform<float>> floatValues;
    };

    template<>
    inline sre::Texture* Material::get(std::string uniformName) {
        auto t = shader->getType(uniformName.c_str());
        if (t.type != UniformType::Texture && t.type != UniformType::TextureCube){
            return nullptr;
        }
        for (auto & tv : textureValues){
            if (tv.id == t.id){
                return tv.value;
            }
        }
        return nullptr;
    }

    template<>
    inline glm::vec4 Material::get(std::string uniformName)  {
        auto t = shader->getType(uniformName.c_str());
        if (t.type != UniformType::Vec4){
            return glm::vec4(0,0,0,0);
        }
        for (auto & tv : vectorValues){
            if (tv.id == t.id){
                return tv.value;
            }
        }
        return glm::vec4(0,0,0,0);
    }

    template<>
    inline float Material::get(std::string uniformName) {
        auto t = shader->getType(uniformName.c_str());
        if (t.type != UniformType::Vec4){
            return 0.0f;
        }
        for (auto & tv : floatValues){
            if (tv.id == t.id){
                return tv.value;
            }
        }
        return 0.0f;
    }
}