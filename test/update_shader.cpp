#include <iostream>

#include "sre/Texture.hpp"
#include "sre/Renderer.hpp"
#include "sre/Material.hpp"
#include "sre/Inspector.hpp"

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/euler_angles.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/transform.hpp>
#include <sre/SDLRenderer.hpp>
#include <sre/Resource.hpp>
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
                .withAttribute("vertex_color",colors)
                .build();

        std::string vertexShaderSource =  R"(#version 330
in vec4 position;
in vec4 vertex_color;
out vec4 vColor;

uniform mat4 customTransform4[2];
uniform float customTransformIndex;

#pragma include "global_uniforms_incl.glsl"

void main(void) {
    int id = int(customTransformIndex);
    gl_Position = g_projection * g_view * g_model * customTransform4[id]*vec4(position);
    vColor = vertex_color;
}
)";
        std::string fragmentShaderSource = R"(#version 330
out vec4 fragColor;
in vec4 vColor;
uniform vec4 extra;

void main(void)
{
    fragColor = vColor + extra;
}
)";
        Resource::set("mat4-vert.glsl", vertexShaderSource);
        Resource::set("mat4-frag.glsl", fragmentShaderSource);

        mat1 = Shader::create()
                .withSourceResource("mat4-vert.glsl",ShaderType::Vertex)
                .withSourceResource("mat4-frag.glsl", ShaderType::Fragment)
                .build()->createMaterial();

        mats4 = std::make_shared<std::vector<glm::mat4>>();
        for (int i=0;i<2;i++){
            mats4->emplace_back(1);
        }
        mat1->set("customTransform4", mats4);
        mat1->set("customTransformIndex", 1);
        mat1->set("extra", glm::vec4(1,0,-1,1));

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



        rp.draw(mesh, glm::mat4(1), mat1);

        if (ImGui::Button("Update shader")){
            updateShader();
        }

        static Inspector inspector;
        inspector.update();
        inspector.gui();

    }

    void updateShader(){
        auto shader = mat1->getShader();
        auto shaderBuilder = shader->update();
        std::string vertexShaderSource =  R"(#version 330
in vec4 position;
in vec4 vertex_color;
out vec4 vColor;

#pragma include "global_uniforms_incl.glsl"

void main(void) {
    gl_Position = g_projection * g_view * g_model * vec4(position);
    vColor = vertex_color;
}
)";
        std::string fragmentShaderSource = R"(#version 330
out vec4 fragColor;
in vec4 vColor;
uniform vec4 extra;

void main(void)
{
    fragColor = vColor + extra;
}
)";
        Resource::set("mat4-vert.glsl", vertexShaderSource);
        Resource::set("mat4-frag.glsl", fragmentShaderSource);
        shaderBuilder.withSourceResource("mat4-vert.glsl",ShaderType::Vertex);
        shaderBuilder.withSourceResource("mat4-frag.glsl", ShaderType::Fragment);
        shaderBuilder.build();
    }
private:
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
    std::make_unique<CustomMeshLayoutExample>();
    return 0;
}

