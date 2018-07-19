/*
 *  SimpleRenderEngine (https://github.com/mortennobel/SimpleRenderEngine)
 *
 *  Created by Morten Nobel-Jørgensen ( http://www.nobel-joergensen.com/ )
 *  License: MIT
 */

#include "sre/Material.hpp"
#include "sre/impl/GL.hpp"
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/color_space.hpp>
#include "sre/Renderer.hpp"
#include "sre/Log.hpp"


namespace sre {

    Material::Material(std::shared_ptr<Shader> shader)
    :shader{nullptr}
    {
        setShader(std::move(shader));
        name = "Undefined material";
    }

    Material::~Material(){
    }

    void Material::bind(){
        if (shader->uniforms != uniforms){
            setShader(shader);
        }
        uniformMap.bind();
    }

    std::shared_ptr<sre::Shader> Material::getShader()  {
        return shader;
    }

    void Material::setShader(std::shared_ptr<sre::Shader> shader) {
        Material::shader = shader;

        UniformSet oldUniformMap = uniformMap;
        uniformMap.clear();

        for (auto & u : *(shader->uniforms)){
            switch (u.type){
                case UniformType::Vec4:
                {
                    uniformMap.set(u.id, glm::vec4(1.0f,1.0f,1.0f,1.0f));
                }
                break;
                case UniformType::Texture:
                {
                    uniformMap.set(u.id, Texture::getWhiteTexture());
                }
                break;
                case UniformType::TextureCube:
                {
                    uniformMap.set(u.id, Texture::getDefaultCubemapTexture());
                }
                break;
                case UniformType::Float:
                {
                    uniformMap.set(u.id, 0.0f);
                }
                break;
                case UniformType::Mat3Array:
                {
                    uniformMap.set(u.id, std::shared_ptr<std::vector<glm::mat3>>());
                }
                break;
                case UniformType::Mat4:
                case UniformType::Mat4Array:
                {
                    if (u.arraySize>1){
                        uniformMap.set(u.id, std::shared_ptr<std::vector<glm::mat4>>());
                    } else {
                        uniformMap.set(u.id, glm::mat4(1));
                    }
                }
                break;
                default:
                    LOG_ERROR("'%s' Unsupported uniform type: %i. Only Vec4, Texture, TextureCube and Float is supported.", u.name.c_str(), (int)u.type);
                    break;
            }
        }
        if (uniforms) {
            // copy old uniform values
            for (auto &oldUniform : *uniforms) {
                for (auto &u : *(shader->uniforms)) {

                    if (u.type == oldUniform.type &&
                        u.arraySize == oldUniform.arraySize &&
                        u.name == oldUniform.name) {
                        switch (u.type) {
                            case UniformType::Vec4: {
                                uniformMap.set(u.id, oldUniformMap.get<glm::vec4>(oldUniform.id));
                            }
                                break;
                            case UniformType::Texture:
                            case UniformType::TextureCube: {
                                uniformMap.set(u.id, oldUniformMap.get<std::shared_ptr<Texture>>(oldUniform.id));
                            }
                                break;
                            case UniformType::Float: {
                                uniformMap.set(u.id, oldUniformMap.get<float>(oldUniform.id));
                            }
                                break;
                            case UniformType::Mat3Array: {
                                std::shared_ptr<std::vector<glm::mat3>> val = oldUniformMap.get<std::shared_ptr<std::vector<glm::mat3>>>(
                                        oldUniform.id);
                                uniformMap.set(u.id, val);
                            }
                                break;
                            case UniformType::Mat4:
                            case UniformType::Mat4Array: {
                                if (u.arraySize > 1) {
                                    std::shared_ptr<std::vector<glm::mat4>> val = oldUniformMap.get<std::shared_ptr<std::vector<glm::mat4>>>(
                                            oldUniform.id);
                                    uniformMap.set(u.id, val);
                                } else {
                                    uniformMap.set(u.id, oldUniformMap.get<glm::mat4>(oldUniform.id));
                                }
                            }
                                break;
                            default:
                                LOG_ERROR(
                                        "'%s' Unsupported uniform type: %i. Only Vec4, Texture, TextureCube and Float is supported.",
                                        u.name.c_str(), (int) u.type);
                                break;
                                break;
                        }
                    }
                }
            }
        }
        uniforms = shader->uniforms;
    }

    Color Material::getColor()   {
        return get<Color>("color");
    }

    bool Material::setColor(const Color &color) {
        return set("color", color);
    }

    Color Material::getSpecularity()   {
        return get<Color>("specularity");
    }

    bool Material::setSpecularity(Color specularity) {
        return set("specularity", specularity);
    }

    std::shared_ptr<sre::Texture> Material::getTexture()  {
        return get<std::shared_ptr<sre::Texture>>("tex");
    }

    bool Material::setTexture(std::shared_ptr<sre::Texture> texture) {
        return set("tex",texture);
    }

    const std::string &Material::getName()  {
        return name;
    }

    void Material::setName(const std::string &name) {
        Material::name = name;
    }

    bool Material::set(std::string uniformName, glm::vec4 value){
        auto type = shader->getUniform(uniformName);
        uniformMap.set(type.id, value);
        return true;
    }

    bool Material::set(std::string uniformName, glm::mat4 value){
        auto type = shader->getUniform(uniformName);
        uniformMap.set(type.id, value);
        return true;
    }


    bool Material::set(std::string uniformName, std::shared_ptr<std::vector<glm::mat3>> value){
        auto type = shader->getUniform(uniformName);
        uniformMap.set(type.id, value);
        return true;
    }

    bool Material::set(std::string uniformName, std::shared_ptr<std::vector<glm::mat4>> value){
        auto type = shader->getUniform(uniformName);
        uniformMap.set(type.id, value);
        return true;
    }

    bool Material::set(std::string uniformName, Color value){
        auto type = shader->getUniform(uniformName);
        uniformMap.set(type.id, value);
        return true;
    }

    bool Material::set(std::string uniformName, float value){
        auto type = shader->getUniform(uniformName);
        uniformMap.set(type.id, value);
        return true;
    }

    bool Material::set(std::string uniformName, std::shared_ptr<sre::Texture> value){
        auto type = shader->getUniform(uniformName);
        uniformMap.set(type.id, value);
        return true;
    }

    std::shared_ptr<sre::Texture> Material::getMetallicRoughnessTexture() {
        return get<std::shared_ptr<sre::Texture>>("mrTex");
    }

    bool Material::setMetallicRoughnessTexture(std::shared_ptr<sre::Texture> texture) {
        return set("mrTex",texture);

    }

    glm::vec2 Material::getMetallicRoughness() {
        return (glm::vec2)get<glm::vec4>("metallicRoughness");
    }

    bool Material::setMetallicRoughness(glm::vec2 metallicRoughness) {
        return set("metallicRoughness",glm::vec4(metallicRoughness,0,0));
    }
}