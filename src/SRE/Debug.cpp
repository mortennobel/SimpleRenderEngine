//
// Created by morten on 04/08/16.
//

#include "SRE/Debug.hpp"
#include "SRE/Mesh.hpp"
#include "SRE/Shader.hpp"
#include "SRE/SimpleRenderEngine.hpp"
#include <vector>

using namespace std;

namespace SRE {

    glm::vec4 Debug::color = glm::vec4(1,0,1,1);

    glm::vec4 Debug::getColor(){
        return color;
    }

    void Debug::setColor(glm::vec4 color){
        Debug::color = color;
    }

    void Debug::drawLine(glm::vec3 from, glm::vec3 to){
		vector<glm::vec3> verts;
		verts.push_back(from);
		verts.push_back(to);
		vector<glm::vec3> normals;
		vector<glm::vec2> uvs;

		Mesh mesh(verts, normals, uvs,MeshTopology::Lines);

        Shader* shader = Shader::getUnlit();
        shader->setVector("color", color);
        if (SimpleRenderEngine::instance != nullptr){
            SimpleRenderEngine::instance->draw(&mesh, glm::mat4(1),shader);
        }
    }

}