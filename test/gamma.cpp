#include <iostream>

#include "sre/Texture.hpp"
#include "sre/Renderer.hpp"
#include "sre/Material.hpp"

#include <glm/gtx/euler_angles.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <sre/SDLRenderer.hpp>
#include <sre/impl/GL.hpp>

using namespace sre;

class GammaTest{
public:
    GammaTest(){
        r.init();

        mesh = Mesh::create().withCube()
                .build();

        std::string vertexShaderSource =  R"(#version 140
in vec4 posxyzw;    // should automatically cast vec2 -> vec4 by appending (z = 0.0, w = 1.0)
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
                .withSourceString(vertexShaderSource,ShaderType::Vertex)
                .withSourceString(fragmentShaderSource, ShaderType::Fragment)
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
    new GammaTest();

    return 0;
}

