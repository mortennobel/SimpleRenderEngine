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

class RenderDepth {
public:
    RenderDepth(){
        r.init();

        camera.lookAt({0,0,3},{0,0,0},{0,1,0});
        camera.setPerspectiveProjection(60,1.5f,3.5f);

        depthTexture = Texture::create().withDepth(1024,1024, Texture::DepthPrecision::I16).build();
        texture = Texture::create().withRGBData(nullptr, 1024,1024).build();
        framebuffer = Framebuffer::create()
                .withColorTexture(texture)
                .withName("Render to color texture")
                .build();
        framebufferDepth = Framebuffer::create()
                .withName("Render to depth texture")
                .withDepthTexture(depthTexture)
                .build();

        materialOffscreen = Shader::getStandardBlinnPhong()->createMaterial();
        materialOffscreen->setSpecularity({1,1,1,120});
        material = Shader::getStandardBlinnPhong()->createMaterial();
        material->setTexture(texture);

        mesh = Mesh::create().withCube().build();
        worldLights.addLight(Light::create().withPointLight({0,0,3}).withColor({1,1,1}).withRange(20).build());

        r.frameRender = [&](){
            render();
        };

        r.startEventLoop();
    }

    void render(){
        static bool useDepthTex = false;
        auto renderToTexturePass = RenderPass::create()                 // Create a renderpass which writes to the texture using a framebuffer
                .withCamera(camera)
                .withWorldLights(&worldLights)
                .withFramebuffer(useDepthTex?framebufferDepth:framebuffer)
                .withClearColor(true, {0, 1, 1, 0})
                .withGUI(false)
                .build();

        renderToTexturePass.draw(mesh, glm::eulerAngleY(glm::radians((float)i)), materialOffscreen);

        auto renderPass = RenderPass::create()                          // Create a renderpass which writes to the screen.
                .withCamera(camera)
                .withWorldLights(&worldLights)
                .withClearColor(true, {1, 0, 0, 1})
                .withGUI(true)
                .build();

        static bool useBlit = false;

        ImGui::Checkbox("Use blit",&useBlit);
        ImGui::Checkbox("Use depth tex",&useDepthTex);
        if (useBlit){
            renderPass.blit(useDepthTex?depthTexture:texture);
        } else {
            material->setTexture(useDepthTex?depthTexture:texture);
            renderPass.draw(mesh, glm::eulerAngleY(glm::radians((float)i)), material);
        }

        // The offscreen texture is used in material
        static Inspector prof;
        prof.update();
        prof.gui(true);

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
    std::shared_ptr<Texture> depthTexture;
    std::shared_ptr<Framebuffer> framebuffer;
    std::shared_ptr<Framebuffer> framebufferDepth;
    int i=0;
};

int main() {
    RenderDepth renderDepth;
    return 0;
}
