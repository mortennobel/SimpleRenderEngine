#include <iostream>
#include <vector>
#include <fstream>

#include "sre/Texture.hpp"
#include "sre/Renderer.hpp"
#include "sre/Material.hpp"
#include "sre/SDLRenderer.hpp"
#include "sre/impl/GL.hpp"


#include <glm/gtx/euler_angles.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <sre/Inspector.hpp>


using namespace sre;

class MultiCamExample {
public:
    MultiCamExample(){
        r.init();

        camera.lookAt(eye,at,up);
        camera.setPerspectiveProjection(60,0.1,100);

        camera2.lookAt({0,0,3},{0,0,0},{0,1,0});
        camera2.setPerspectiveProjection(60,0.1,100);
        camera2.setViewport({0,0.8},{0.2,0.2});

        material = Shader::getStandardPhong()->createMaterial();
        material->setColor({1.0f,1.0f,1.0f,1.0f});
        material->setSpecularity(20.0f);

        mesh = Mesh::create().withCube().build();
        worldLights.setAmbientLight({0.5,0.5,0.5});
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
                .withGUI(false)
                .build();

        renderPass.draw(mesh, glm::eulerAngleY(glm::radians((float)i)), material);

        renderPass = RenderPass::create()
                .withCamera(camera2)
                .withWorldLights(&worldLights)
                .withClearColor(true, {1, 1, 0, 1})
                .withGUI(true)
                .build();

        renderPass.draw(mesh, glm::eulerAngleY(glm::radians((float)i)), material);

        static bool lookAt = true;
        ImGui::Checkbox("LookAt",&lookAt);
        if (lookAt){
            ImGui::DragFloat3("eye",&eye.x);
            ImGui::DragFloat3("at",&at.x);
            ImGui::DragFloat3("up",&up.x);
            camera.lookAt(eye, at, up);
        } else {
            ImGui::DragFloat3("position",&position.x);
            ImGui::DragFloat3("rotation",&rotation.x);
            camera.setPositionAndRotation(position, rotation);
        }
        auto pos = camera.getPosition();
        auto rot = camera.getRotationEuler();
        ImGui::LabelText("GetPos","%f %f %f", pos.x, pos.y, pos.z);
        ImGui::LabelText("GetRot","%f %f %f", rot.x, rot.y, rot.z);

        // static Inspector prof;
        // prof.update();
        // prof.gui(true);

        i++;
    }
private:
    glm::vec3 eye{0,0,3};
    glm::vec3 at{0,0,0};
    glm::vec3 up{0,1,0};
    glm::vec3 position{0,0,3};
    glm::vec3 rotation{0,0,0};
    SDLRenderer r;
    Camera camera;
    Camera camera2;
    WorldLights worldLights;
    std::shared_ptr<Mesh> mesh;
    std::shared_ptr<Material> material;
    int i=0;
};

int main() {
    new MultiCamExample();

    return 0;
}
