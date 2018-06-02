#include <iostream>
#include <vector>
#include <fstream>

#include "sre/Texture.hpp"
#include "sre/Renderer.hpp"
#include "sre/Material.hpp"
#include "sre/SDLRenderer.hpp"

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/euler_angles.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/transform.hpp>


using namespace sre;

class ImGuiColorTest {
public:
    ImGuiColorTest(){
        r.init();

        camera.lookAt({0,0,3},{0,0,0},{0,1,0});
        camera.setPerspectiveProjection(60,0.1,100);


        material = Shader::getUnlit()->createMaterial();
        material->setColor(color);

        mesh = Mesh::create().withCube().build();
        worldLights.setAmbientLight({0.0,0.0,0.0});
        worldLights.addLight(Light::create().withPointLight({0, 3,0}).withColor({1,0,0}).withRange(20).build());
        worldLights.addLight(Light::create().withPointLight({3, 0,0}).withColor({0,1,0}).withRange(20).build());
        worldLights.addLight(Light::create().withPointLight({0,-3,0}).withColor({0,0,1}).withRange(20).build());
        worldLights.addLight(Light::create().withPointLight({-3,0,0}).withColor({1,1,1}).withRange(20).build());

        r.frameRender = [&](){
            render();
        };

        r.startEventLoop();
    }

    void render(){
        auto renderPass = RenderPass::create()
                .withCamera(camera)
                .withWorldLights(&worldLights)
                .withClearColor(true, {1, 0, 0, 1})
                .build();

        renderPass.draw(mesh, glm::eulerAngleY(glm::radians((float)i*0.1f))*glm::translate(glm::vec3{0,-1,0}), material);
        auto linearColor = color.toLinear();
        bool changed = ImGui::ColorEdit4("Color", &(linearColor.x));
        if (changed){
            color.setFromLinear(linearColor);
            material->setColor(color);
        }
        i++;
    }
private:
    SDLRenderer r;
    Camera camera;
    WorldLights worldLights;
    std::shared_ptr<Mesh> mesh;
    Color color = {1.0f,1.0f,1.0f,1.0f};

    std::shared_ptr<Material> material;
    int i=0;
};

int main() {
    std::make_unique<ImGuiColorTest>();
    return 0;
}
