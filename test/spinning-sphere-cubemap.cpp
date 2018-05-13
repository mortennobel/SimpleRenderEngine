#include <iostream>
#include <vector>
#include <fstream>

#include "sre/Texture.hpp"
#include "sre/Renderer.hpp"
#include "sre/Camera.hpp"
#include "sre/Mesh.hpp"
#include "sre/Material.hpp"
#include "sre/Shader.hpp"
#include <imgui.h>
#include "sre/imgui_sre.hpp"

#include <glm/glm.hpp>

#include <glm/gtx/euler_angles.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <sre/SDLRenderer.hpp>

using namespace sre;

class SpinningSphereCubemapExample{
public:
    SpinningSphereCubemapExample(){
        r.init();

        camera.lookAt(eye,at, up);
        camera.setPerspectiveProjection(60,0.1f,100);
        const char* vertexShaderStr = R"(#version 330
in vec3 position;
in vec3 normal;
in vec2 uv;
out vec3 vNormal;

#pragma include "global_uniforms_incl.glsl"

void main(void) {
    gl_Position = g_projection * g_view * g_model * vec4(position,1.0);
    vNormal = normal;
}
)";
        const char* fragmentShaderStr = R"(#version 330
out vec4 fragColor;
in vec3 vNormal;

uniform samplerCube tex;

void main(void)
{
    fragColor = texture(tex, vNormal);
}
)";
        auto shader = Shader::create()
                .withSourceString(vertexShaderStr, ShaderType::Vertex)
                .withSourceString(fragmentShaderStr, ShaderType::Fragment)
                .build();
        material = shader->createMaterial();
        tex = Texture::create()
                .withFileCubemap("test_data/cube-posx.png", Texture::CubemapSide::PositiveX)
                .withFileCubemap("test_data/cube-negx.png", Texture::CubemapSide::NegativeX)
                .withFileCubemap("test_data/cube-posy.png", Texture::CubemapSide::PositiveY)
                .withFileCubemap("test_data/cube-negy.png", Texture::CubemapSide::NegativeY)
                .withFileCubemap("test_data/cube-posz.png", Texture::CubemapSide::PositiveZ)
                .withFileCubemap("test_data/cube-negz.png", Texture::CubemapSide::NegativeZ)
                .build();

        material->setTexture(tex);

        mesh = Mesh::create().withSphere().build();
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

        rp.draw(mesh, glm::eulerAngleY(time), material);
        time += 0.016f;

        ImGui::DragFloat3("CameraPos",&eye.x);
        if (animatedCamera){
            eye = {
                    sin(time*-0.2)*5.0f,
                    sin(time*-0.4)*0.5f,
                    cos(time*-0.2)*5.0f,
            };
        }
        camera.lookAt(eye,at, up);

    }

    void drawCross(RenderPass& rp,glm::vec3 p, float size = 0.3f, glm::vec4 color = {1,0,0,1}){
        Color col;
        col.setFromLinear(color);
        rp.drawLines({p-glm::vec3{size,0,0}, p+glm::vec3{size,0,0}},col);
        rp.drawLines({p-glm::vec3{0,size,0}, p+glm::vec3{0,size,0}},col);
        rp.drawLines({p-glm::vec3{0,0,size}, p+glm::vec3{0,0,size}},col);
    }

    void drawLight(RenderPass rp, Light& l, float size){
        if (l.lightType == LightType::Point || l.lightType == LightType::Directional){

            drawCross(rp,l.position, size, {0,0,0,1});
        }
        if (l.lightType == LightType::Directional){
            rp.drawLines({l.position, l.position - l.direction*size*2.0f},{1,1,0,1});
        }
    }
private:
    SDLRenderer r;
    Camera camera;
    std::shared_ptr<Material> material;
    std::shared_ptr<Texture> tex;
    std::shared_ptr<Mesh> mesh;
    bool animatedCamera = true;
    glm::vec3 eye{0,0,5};
    glm::vec3 at{0,0,0};
    glm::vec3 up{0,1,0};
    float time = 0;
};


int main() {
    std::make_unique<SpinningSphereCubemapExample>();
    return 0;
}

