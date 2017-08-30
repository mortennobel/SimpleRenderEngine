#include <iostream>
#include <vector>
#include <fstream>

#include "sre/Texture.hpp"
#include "sre/Renderer.hpp"
#include "sre/Material.hpp"
#include "sre/SDLRenderer.hpp"

#include <glm/gtx/euler_angles.hpp>
#include <glm/gtc/matrix_transform.hpp>



using namespace sre;

class RenderToTextureExample {
public:
    RenderToTextureExample(){
        r.init();

        camera.lookAt({0,0,3},{0,0,0},{0,1,0});
        camera.setPerspectiveProjection(60,0.1,100);

        texture = Texture::create().withRGBData(nullptr, 1024,1024).build();

        framebuffer = Framebuffer::create().withTexture(texture).build();

        materialOffscreen = Shader::getStandard()->createMaterial();
        material = Shader::getStandard()->createMaterial();
        material->setTexture(texture);

        mesh = Mesh::create().withCube().build();
        worldLights.addLight(Light::create().withPointLight({0,0,3}).withColor({1,1,1}).withRange(20).build());

        r.frameRender = [&](){
            render();
        };

        r.startEventLoop();
    }

    void render(){
        auto renderToTexturePass = RenderPass::create()                 // Create a renderpass which writes to the texture using a framebuffer
                .withCamera(camera)
                .withWorldLights(&worldLights)
                .withFramebuffer(framebuffer)
                .withClearColor(true, {0, 1, 1, 0})
                .build();

        renderToTexturePass.draw(mesh, glm::eulerAngleY(glm::radians((float)i)), materialOffscreen);

        auto renderPass = RenderPass::create()                          // Create a renderpass which writes to the screen.
                .withCamera(camera)
                .withWorldLights(&worldLights)
                .withClearColor(true, {1, 0, 0, 1})
                .build();

        renderPass.draw(mesh, glm::eulerAngleY(glm::radians((float)i)), material);
                                                                        // The offscreen texture is used in material
        i++;
    }
private:
    SDLRenderer r;
    Camera camera;
    WorldLights worldLights;
    std::shared_ptr<Mesh> mesh;
    std::shared_ptr<Material> materialOffscreen;
    std::shared_ptr<Material> material;
    std::shared_ptr<Texture> texture;
    std::shared_ptr<Framebuffer> framebuffer;
    int i=0;
};

int main() {
    new RenderToTextureExample();
    return 0;
}
