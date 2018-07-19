/*
 *  SimpleRenderEngine (https://github.com/mortennobel/SimpleRenderEngine)
 *
 *  Created by Morten Nobel-Jørgensen ( http://www.nobel-joergensen.com/ )
 *  License: MIT
 */

#pragma once

#include "sre/Shader.hpp"
#include "sre/Texture.hpp"
#include "glm/glm.hpp"
#include "sre/Color.hpp"
#include "sre/impl/UniformSet.hpp"

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
        Color getColor();

        bool setColor(const Color &color);

        std::shared_ptr<sre::Texture> getTexture();

        bool setTexture(std::shared_ptr<sre::Texture> texture);

        Color getSpecularity();

        bool setSpecularity(Color specularity); // {specular intensity (rgb), Specular exponent (a)}.
                                                // Specular intensity should be between 0.0 and 1.0
                                                // Alpha value stores the specular exponent must be above 0.0. Large values gives smaller highlights


        glm::vec2 getMetallicRoughness();       // The metalness of the material. A value of 1.0 means the material is
        bool setMetallicRoughness(glm::vec2 metallicRoughness);       // a metal. A value of 0.0 means the material is a dielectric. Values in
                                                // between are for blending between metals and dielectrics such as dirty
                                                // metallic surfaces. This value is linear. If a metallicRoughnessTexture
                                                // is specified, this value is multiplied with the metallic texel values.
                                                // (Source https://github.com/KhronosGroup/glTF/tree/master/specification/2.0#reference-pbrmetallicroughness)
                                                // The roughness of the material. A value of 1.0 means the material is
                                                // completely rough. A value of 0.0 means the material is completely smooth.
                                                //  This value is linear. If a metallicRoughnessTexture is specified,
                                                // this value is multiplied with the roughness texel values.
                                                // (Source https://github.com/KhronosGroup/glTF/tree/master/specification/2.0#reference-pbrmetallicroughness)

        std::shared_ptr<sre::Texture> getMetallicRoughnessTexture();
        bool setMetallicRoughnessTexture(std::shared_ptr<sre::Texture> texture);
                                                // The metallic-roughness texture. The metalness values are sampled from
                                                // the B channel. The roughness values are sampled from the G channel.
                                                // These values are linear. If other channels are present (R or A), they
                                                // are ignored for metallic-roughness calculations.
                                                // (Source https://github.com/KhronosGroup/glTF/tree/master/specification/2.0#reference-pbrmetallicroughness)

        bool set(std::string uniformName, glm::vec4 value);
        bool set(std::string uniformName, float value);
        bool set(std::string uniformName, glm::mat4 value);
        bool set(std::string uniformName, std::shared_ptr<Texture> value);
        bool set(std::string uniformName, std::shared_ptr<std::vector<glm::mat3>> value);
        bool set(std::string uniformName, std::shared_ptr<std::vector<glm::mat4>> value);
        bool set(std::string uniformName, Color value);

        template<typename T>
        inline T get(std::string uniformName);
    private:
        void bind();

        explicit Material(std::shared_ptr<sre::Shader> shader);
        std::shared_ptr<std::vector<Uniform>> uniforms;
        std::string name;
        std::shared_ptr<sre::Shader> shader;

        UniformSet uniformMap;

        friend class Shader;
        friend class RenderPass;
        friend class Inspector;
    };

    template<>
    inline std::shared_ptr<sre::Texture> Material::get(std::string uniformName) {
        auto t = shader->getUniform(uniformName);
        if (t.type != UniformType::Texture && t.type != UniformType::TextureCube){
            return nullptr;
        }
        for (auto & tv : uniformMap.textureValues){
            auto res = uniformMap.textureValues.find(t.id);
            if (res != uniformMap.textureValues.end()){
                return res->second;
            }
        }
        return nullptr;
    }

    template<>
    inline glm::vec4 Material::get(std::string uniformName)  {
        auto t = shader->getUniform(uniformName);
        if (t.type == UniformType::Vec4){
            auto res = uniformMap.vectorValues.find(t.id);
            if (res != uniformMap.vectorValues.end()){
                return res->second;
            }
        }
        return glm::vec4(0,0,0,0);
    }

    template<>
    inline glm::mat4 Material::get(std::string uniformName)  {
        auto t = shader->getUniform(uniformName);
        if (t.type == UniformType::Mat4){
            auto res = uniformMap.mat4Values.find(t.id);
            if (res != uniformMap.mat4Values.end()){
                return res->second;
            }
        }
        return glm::mat4(1);
    }

    template<>
    inline Color Material::get(std::string uniformName)  {
        auto t = shader->getUniform(uniformName);
        if (t.type == UniformType::Vec4){
            auto res = uniformMap.vectorValues.find(t.id);
            if (res != uniformMap.vectorValues.end()){
                Color value;
                value.setFromLinear(res->second);
                return value;
            }
        }
        return {0,0,0,0};
    }

    template<>
    inline float Material::get(std::string uniformName) {
        auto t = shader->getUniform(uniformName);
        if (t.type == UniformType::Float) {
            auto res = uniformMap.floatValues.find(t.id);
            if (res != uniformMap.floatValues.end()){
                return res->second;
            }
        }
        return 0.0f;
    }

    template<>
    inline std::shared_ptr<std::vector<glm::mat3>> Material::get(std::string uniformName) {
        auto t = shader->getUniform(uniformName);
        if (t.type == UniformType::Mat3Array) {
            auto res = uniformMap.mat3sValues.find(t.id);
            if (res != uniformMap.mat3sValues.end()){
                return res->second;
            }
        }
        return {};
    }

    template<>
    inline std::shared_ptr<std::vector<glm::mat4>> Material::get(std::string uniformName) {
        auto t = shader->getUniform(uniformName);
        if (t.type == UniformType::Mat4Array) {
            auto res = uniformMap.mat4sValues.find(t.id);
            if (res != uniformMap.mat4sValues.end()){
                return res->second;
            }
        }
        return {};
    }

}