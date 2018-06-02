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
#include <sre/Skybox.hpp>
#include <sre/Inspector.hpp>

using namespace sre;

class SkyboxExample {
public:
    SkyboxExample(){
        r.init();

        camera.lookAt({0,0,3},{0,0,0},{0,1,0});
        camera.setPerspectiveProjection(60,0.1,100);

        material = Shader::getStandardPBR()->createMaterial();
        material->setColor({1.0f,1.0f,1.0f,1.0f});
        material->setMetallicRoughness({.5f,.5f});

        mesh = Mesh::create().withCube().build();
        worldLights.setAmbientLight({0.0,0.0,0.0});
        worldLights.addLight(Light::create().withPointLight({0, 3,0}).withColor({1,0,0}).withRange(20).build());
        worldLights.addLight(Light::create().withPointLight({3, 0,0}).withColor({0,1,0}).withRange(20).build());
        worldLights.addLight(Light::create().withPointLight({0,-3,0}).withColor({0,0,1}).withRange(20).build());
        worldLights.addLight(Light::create().withPointLight({-3,0,0}).withColor({1,1,1}).withRange(20).build());

        auto tex = Texture::create()
                .withFileCubemap("examples_data/cube-posx.png", Texture::CubemapSide::PositiveX)
                .withFileCubemap("examples_data/cube-negx.png", Texture::CubemapSide::NegativeX)
                .withFileCubemap("examples_data/cube-posy.png", Texture::CubemapSide::PositiveY)
                .withFileCubemap("examples_data/cube-negy.png", Texture::CubemapSide::NegativeY)
                .withFileCubemap("examples_data/cube-posz.png", Texture::CubemapSide::PositiveZ)
                .withFileCubemap("examples_data/cube-negz.png", Texture::CubemapSide::NegativeZ)
                .withWrapUV(Texture::Wrap::ClampToEdge)
                .build();

        skybox = Skybox::create();

        auto skyboxMaterial = Shader::getSkybox()->createMaterial();
        skyboxMaterial->setTexture(tex);
        skybox->setMaterial(skyboxMaterial);

        r.frameRender = [&](){
            render();
        };
        r.mouseEvent = [&](SDL_Event& event){
            if (event.type == SDL_MOUSEMOTION){
                float mouseSpeed = 1/50.0f;
                float rotateY = event.motion.x*mouseSpeed;
                float rotateX = (event.motion.y/(float)Renderer::instance->getWindowSize().y) * 3.14f-3.14f/2;
                auto rot = glm::rotate(rotateY, glm::vec3(0,1,0))*glm::rotate(rotateX, glm::vec3(1,0,0));
                auto rotatedPos = rot * glm::vec4(0,0,3,1);
                camera.lookAt(glm::vec3(rotatedPos),{0,0,0},{0,1,0});
            }
            if (event.type == SDL_MOUSEBUTTONUP){
                if (event.button.button==SDL_BUTTON_RIGHT){
                    showGui = true;
                }
            }
        };

        r.startEventLoop();
    }

    void render(){
        auto renderPass = RenderPass::create()
                .withCamera(camera)
                .withWorldLights(&worldLights)
                .withSkybox(skybox)
                .withName("Frame")
                .build();

        renderPass.draw(mesh, glm::mat4(1), material);
        renderPass.draw(mesh, glm::translate(glm::vec3(2,2,2)) * glm::scale(glm::vec3(0.1)), material);
        renderPass.draw(mesh, glm::translate(glm::vec3(-2,-2,-2)) * glm::scale(glm::vec3(0.1)), material);
        static Inspector inspector;
        inspector.update();
        if (showGui) inspector.gui();
    }
private:
    SDLRenderer r;
    Camera camera;
    WorldLights worldLights;
    std::shared_ptr<Mesh> mesh;
    std::shared_ptr<Material> material;
    std::shared_ptr<Skybox> skybox;
    bool showGui = false;
};

int main() {
    std::make_unique<SkyboxExample>();
    return 0;
}
