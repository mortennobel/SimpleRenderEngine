/*
 *  SimpleRenderEngine (https://github.com/mortennobel/SimpleRenderEngine)
 *
 *  Created by Morten Nobel-Jørgensen ( http://www.nobel-joergnesen.com/ )
 *  License: MIT
 */

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
    class RenderPass;

    /**
     * Material encapsulates a shader and its states. Example using the standard shader (computing the color of the
     * surface using lights in the scene) a material would contain a color (glm::vec4), a texture (sre::Texture) and
     * specularity (float). The specularity determines that shininess of the material.
     */
    class DllExport Material {
    public:
        ~Material();

        std::shared_ptr<sre::Shader> getShader();

        void setShader(std::shared_ptr<sre::Shader> shader);

        const std::string &getName();

        void setName(const std::string &name);

        // uniform parameters
        glm::vec4 getColor();

        bool setColor(const glm::vec4 &color);

        float getSpecularity();

        bool setSpecularity(float specularity);

        std::shared_ptr<sre::Texture> getTexture();

        bool setTexture(std::shared_ptr<sre::Texture> texture);

        bool set(std::string uniformName, glm::vec4 value);
        bool set(std::string uniformName, float value);
        bool set(std::string uniformName, std::shared_ptr<sre::Texture>);

        template<typename T>
        inline T get(std::string uniformName);
    private:
        void bind();

        explicit Material(std::shared_ptr<sre::Shader> shader);
        std::string name;
        std::shared_ptr<sre::Shader> shader;

        template<typename T>
        struct DllExport Uniform {
            int id;
            T value;
        };

        std::vector<Uniform<std::shared_ptr<sre::Texture>>> textureValues;
        std::vector<Uniform<glm::vec4>> vectorValues;
        std::vector<Uniform<float>> floatValues;

        friend class Shader;
        friend class RenderPass;
    };

    template<>
    inline std::shared_ptr<sre::Texture> Material::get(std::string uniformName) {
        auto t = shader->getUniformType(uniformName.c_str());
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
        auto t = shader->getUniformType(uniformName.c_str());
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
        auto t = shader->getUniformType(uniformName.c_str());
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