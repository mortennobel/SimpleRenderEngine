#include <iostream>

#include "sre/Texture.hpp"
#include "sre/Renderer.hpp"
#include "sre/Material.hpp"

#include <glm/gtx/euler_angles.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <sre/SDLRenderer.hpp>
#include <sre/impl/GL.hpp>
#include <sre/Inspector.hpp>

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
in vec4 posxyzw;
in vec4 vertex_color;
out vec4 vColor;

#pragma include "global_uniforms_incl.glsl"

void main(void) {
    gl_Position = g_projection * g_view * g_model * posxyzw;
    vColor = vertex_color;
}
)";
        std::string fragmentShaderSource = R"(#version 330
out vec4 fragColor;
in vec4 vColor;

void main(void)
{
    fragColor = vColor;
}
)";

        mat1 = Shader::getUnlit()->createMaterial({{"S_VERTEX_COLOR","1"}});

        r.frameRender = [&](){
            render();
        };
        r.mouseEvent = [&](SDL_Event& event){
            if (event.type == SDL_MOUSEBUTTONUP){
                if (event.button.button==SDL_BUTTON_RIGHT){
                    showInspector = true;
                }
            }
        };
        r.startEventLoop();
    }

    void render(){
        auto rp = RenderPass::create()
                .withCamera(camera)
                .withClearColor(true,{1,0,0,1})
                .build();

        rp.draw(mesh, glm::mat4(1), mat1);
        static Inspector inspector;
        inspector.update();
        if (showInspector){
            inspector.gui();
        }

    }
private:
    float time;
    SDLRenderer r;
    Camera camera;
    std::shared_ptr<Mesh> mesh;
    std::shared_ptr<Material> mat1;
    bool showInspector = false;
};

int main() {
    std::make_unique<CustomMeshLayoutExample>();
    return 0;
}

