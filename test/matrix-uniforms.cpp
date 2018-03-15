#include <iostream>

#include "sre/Texture.hpp"
#include "sre/Renderer.hpp"
#include "sre/Material.hpp"
#include "sre/Inspector.hpp"

#include <glm/gtx/euler_angles.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/transform.hpp>
#include <sre/SDLRenderer.hpp>
#include <sre/impl/GL.hpp>

using namespace sre;

class CustomMeshLayoutExample{
public:
    CustomMeshLayoutExample(){
        r.init();

        std::vector<glm::vec3> positions({
                                                  {0, 1,0},
                                                  {0, 0,0},
                                                  {1, 0,0}
                                          });
        std::vector<glm::vec4> colors({
                                              {1, 0,0,1},
                                              {0, 1,0,1},
                                              {0, 0,1,1},

                                      });

        mesh = Mesh::create()
                .withPositions(positions)
                .withAttribute("color",colors)
                .build();

        std::string vertexShaderSource =  R"(#version 140
in vec4 position;
in vec4 color;
out vec4 vColor;

uniform mat4 customTransform4[2];
uniform float customTransformIndex;
uniform mat4 g_model;
uniform mat4 g_view;
uniform mat4 g_projection;

void main(void) {
    int id = int(customTransformIndex);
    gl_Position = g_projection * g_view * g_model * customTransform4[id]*vec4(position);
    vColor = color;
}
)";
        std::string fragmentShaderSource = R"(#version 140
out vec4 fragColor;
in vec4 vColor;

void main(void)
{
    fragColor = vColor;
}
)";

        mat1 = Shader::create()
                .withSourceString(vertexShaderSource,ShaderType::Vertex)
                .withSourceString(fragmentShaderSource, ShaderType::Fragment)
                .build()->createMaterial();

        std::string info;
        bool isValid = mat1->getShader()->validateMesh(mesh.get(), info);
        std::cout << "Mesh is valid: "<<isValid<<"\ninfo: "<<info<<std::endl;

        mats4 = std::make_shared<std::vector<glm::mat4>>();
        for (int i=0;i<2;i++){
            mats4->emplace_back(1);
        }

        r.frameRender = [&](){
            render();
        };
        r.startEventLoop();
    }

    void render(){
        auto rp = RenderPass::create()
                .withCamera(camera)
                .withClearColor(true,{1,0,0,1})
                .build();

        ImGui::DragInt("Id ",&id, 1,0,1);
        ImGui::DragFloat3("Offset ",&offset[id].x,0.1f);
        ImGui::DragFloat("Rotate ",&rotate[id],0.1f);

        // update matrix array
        for (int i=0;i<2;i++){
            (*mats4)[i] = glm::translate(offset[i]) * glm::rotate(rotate[i],glm::vec3(0,0,1));
        }
        // update uniforms
        bool setIndex = mat1->set("customTransformIndex",(float)id);
        bool setMatrix = mat1->set("customTransform4",mats4);

        rp.draw(mesh, glm::mat4(1), mat1);

        static Inspector inspector;
        inspector.update();
        inspector.gui();

    }
private:
    float time;
    SDLRenderer r;
    Camera camera;
    std::shared_ptr<Mesh> mesh;
    std::shared_ptr<Material> mat1;
    std::shared_ptr<std::vector<glm::mat4>> mats4;
    int id = 0;
    glm::vec3 offset[2] = {{0,0,0},{0,0,0}};
    float rotate[2] = {0,0};

};

int main() {
    new CustomMeshLayoutExample();

    return 0;
}

