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

        std::shared_ptr<sre::Texture> getTexture();

        bool setTexture(std::shared_ptr<sre::Texture> texture);

        float getSpecularity();

        bool setSpecularity(float specularity);

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