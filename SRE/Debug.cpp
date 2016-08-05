//
// Created by morten on 04/08/16.
//

#include "Debug.hpp"
#include "Mesh.hpp"
#include "Shader.hpp"
#include "SimpleRenderEngine.hpp"

namespace SRE {

    static glm::vec4 Debug::color = glm::vec4(1,0,1,1);

    static glm::vec4 Debug::getColor(){
        return color;
    }

    static void setColor(glm::vec4 color){
        Debug::color = color;
    }

    static void drawLine(glm::vec3 from, glm::vec3 to){
        Mesh mesh;
        mesh.updateMesh({from, to}, {{0},{0}},{{0},{0}}, MeshTopology::Lines);

        Shader* shader = Shader::createUnlitColor();
        shader->setVector("color", color);
        if (SimpleRenderEngine::instance != nullptr){
            SimpleRenderEngine::instance->draw(mesh, glm::mat4(1),shader);
        }
    }

    static void drawBox(glm::vec3 position, glm::vec3 size){

    }

    static void drawWireBox(glm::vec3 position, glm::vec3 size){

    }

    static void drawSphere(glm::vec3 position, float size){

    }

    static void drawWireSphere(glm::vec3 position, float size){

    }
}