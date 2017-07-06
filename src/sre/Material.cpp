//
// Created by Morten Nobel-Jørgensen on 15/06/2017.
//
#include "sre/Material.hpp"
#include "sre/impl/GL.hpp"
#include <glm/gtc/type_ptr.hpp>
#include <sre/Renderer.hpp>

namespace sre{

    Material::Material(){
        if (! Renderer::instance ){
            throw std::runtime_error("Cannot instantiate sre::Material before sre::Renderer is created.");
        }

        setShader(sre::Shader::getStandard());
        name = "Undefined material";
    }

    Material::Material(std::shared_ptr<Shader> shader)
    :shader{nullptr}
    {
        setShader(shader);
        name = "Undefined material";
    }

    Material::~Material(){
    }

    void Material::bind(){
        unsigned int textureSlot = 0;
        for (auto t : textureValues) {
            glActiveTexture(GL_TEXTURE0 + textureSlot);
            glBindTexture(t.value->target, t.value->textureId);
            glUniform1i(t.id, textureSlot);
        }
        for (auto t : vectorValues) {
            glUniform4fv(t.id, 1, glm::value_ptr(t.value));
        }
        for (auto t : floatValues) {
            glUniform1f(t.id, t.value);
        }
    }

    std::shared_ptr<sre::Shader> Material::getShader()  {
        return shader;
    }

    void Material::setShader(std::shared_ptr<sre::Shader> shader) {
        Material::shader = shader;

        textureValues.clear();
        vectorValues.clear();
        floatValues.clear();

        for (auto & u : shader->uniforms){

            switch (u.type){
                case UniformType::Vec4:
                {
                    Uniform<glm::vec4> uniform;
                    uniform.id = u.id;
                    uniform.value = glm::vec4(1.0f,1.0f,1.0f,1.0f);
                    vectorValues.push_back(uniform);
                }
                break;
                case UniformType::Texture:
                {
                    Uniform<sre::Texture*> uniform;
                    uniform.id = u.id;
                    uniform.value = Texture::getWhiteTexture();
                    textureValues.push_back(uniform);
                }
                break;
                case UniformType::TextureCube:
                {
                    Uniform<sre::Texture*> uniform;
                    uniform.id = u.id;
                    uniform.value = Texture::getDefaultCubemapTexture();
                    textureValues.push_back(uniform);
                }
                break;
                case UniformType::Float:
                {
                    Uniform<float> uniform;
                    uniform.id = u.id;
                    uniform.value = 0.0f;
                    floatValues.push_back(uniform);
                }
                break;
                default:
                    std::cerr<<"Unsupported type"<<std::endl;
                    break;
            }
        }
    }

    glm::vec4 Material::getColor()   {
        return get<glm::vec4>("color");
    }

    bool Material::setColor(const glm::vec4 &color) {
        return set("color", color);
    }

    float Material::getSpecularity()   {
        return get<float>("specularity");
    }

    bool Material::setSpecularity(float specularity) {
        return set("specularity", specularity);
    }

    sre::Texture *Material::getTexture()  {
        return get<sre::Texture*>("tex");
    }

    bool Material::setTexture(sre::Texture *texture) {
        return set("tex",texture);
    }

    const std::string &Material::getName()  {
        return name;
    }

    void Material::setName(const std::string &name) {
        Material::name = name;
    }

    bool Material::set(std::string uniformName, glm::vec4 value){
        auto type = shader->getUniformType(uniformName);
        for (auto & v : vectorValues){
            if (v.id==type.id){
                v.value = value;
                return true;
            }
        }
        return false;
    }

    bool Material::set(std::string uniformName, float value){
        auto type = shader->getUniformType(uniformName);
        for (auto & v : floatValues){
            if (v.id==type.id){
                v.value = value;
                return true;
            }
        }
        return false;
    }

    bool Material::set(std::string uniformName, sre::Texture * value){
        auto type = shader->getUniformType(uniformName);
        for (auto & v : textureValues){
            if (v.id==type.id){
                v.value = value;
                return true;
            }
        }
        return false;
    }
}