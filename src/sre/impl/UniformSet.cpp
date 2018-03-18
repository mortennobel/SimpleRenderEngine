/*
 *  SimpleRenderEngine (https://github.com/mortennobel/SimpleRenderEngine)
 *
 *  Created by Morten Nobel-Jørgensen ( http://www.nobel-joergensen.com/ )
 *  License: MIT
 */
#include <glm/gtc/type_ptr.hpp>
#include "sre/impl/UniformSet.hpp"

namespace sre {

    void UniformSet::bind(){
        unsigned int textureSlot = 0;
        for (const auto & t : textureValues) {

            glActiveTexture(GL_TEXTURE0 + textureSlot);
            glBindTexture(t.second->target, t.second->textureId);
            glUniform1i(t.first, textureSlot);
            textureSlot++;
        }
        for (auto& t : vectorValues) {
            glUniform4fv(t.first, 1, glm::value_ptr(t.second));
        }
        for (auto& t : floatValues) {
            glUniform1f(t.first, t.second);
        }
        for (auto& t : mat3Values) {
            if (t.second.get()) {
                glm::mat3& m3 = (*t.second)[0];
                glUniformMatrix3fv(t.first, static_cast<GLsizei>(t.second->size()), GL_FALSE, glm::value_ptr(m3));
            }
        }
        for (auto& t : mat4Values) {
            if (t.second.get()){
                glm::mat4& m4 = (*t.second)[0];
                glUniformMatrix4fv(t.first, static_cast<GLsizei>(t.second->size()), GL_FALSE, glm::value_ptr(m4));
            }
        }
    }

    void UniformSet::set(int id, glm::vec4 value){
        vectorValues[id] = value;
    }

    void UniformSet::set(int id, float value){
        floatValues[id] = value;
    }

    void UniformSet::set(int id, std::shared_ptr<Texture> value){
        textureValues[id] = value;
    }

    void UniformSet::set(int id, std::shared_ptr<std::vector<glm::mat3>> value){
        mat3Values[id] = value;
    }

    void UniformSet::set(int id, std::shared_ptr<std::vector<glm::mat4>> value){
        mat4Values[id] = value;
    }

    void UniformSet::set(int id, Color value){
        vectorValues[id] = value.toLinear();;
    }

    void UniformSet::clear(){
        textureValues.clear();
        vectorValues.clear();
        mat4Values.clear();
        mat3Values.clear();
        floatValues.clear();
    }
}