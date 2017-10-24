#include <iostream>

#include "sre/Texture.hpp"
#include "sre/Renderer.hpp"
#include "sre/Material.hpp"

#include <glm/gtx/euler_angles.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <sre/SDLRenderer.hpp>
#include <sre/impl/GL.hpp>

using namespace sre;

class CustomMeshLayoutExample{
public:
    CustomMeshLayoutExample(){
        r.init();

        std::vector<glm::vec4> positions({
                                                 {0, 1,0,1},
                                                 {0, 0,0,1},
                                                 {1, 0,0,1}
                                         });
        std::vector<glm::vec4> colors({
                                              {1, 0,0,1},
                                              {0, 1,0,1},
                                              {0, 0,1,1},

                                      });

        mesh = Mesh::create()
                .withAttribute("posxyzw",positions)
                .withAttribute("color",colors)
                .build();

        std::string vertexShaderSource =  R"(#version 140
in vec4 posxyzw;
in vec4 color;
out vec4 vColor;

uniform mat4 g_model;
uniform mat4 g_view;
uniform mat4 g_projection;

void main(void) {
    gl_Position = g_projection * g_view * g_model * posxyzw;
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
                .withSource(vertexShaderSource,fragmentShaderSource)
                .build()->createMaterial();

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

    }
private:
    float time;
    SDLRenderer r;
    Camera camera;
    std::shared_ptr<Mesh> mesh;
    std::shared_ptr<Material> mat1;
};

int main() {
    new CustomMeshLayoutExample();

    return 0;
}

