#include <iostream>
#include <vector>
#include <fstream>

#include "sre/Texture.hpp"
#include "sre/Renderer.hpp"
#include "sre/Camera.hpp"
#include "sre/Mesh.hpp"
#include "sre/Material.hpp"
#include "sre/Shader.hpp"
#include "sre/SDLRenderer.hpp"
#define SDL_MAIN_HANDLED
#include "SDL.h"

#include <glm/glm.hpp>

#include <glm/gtx/euler_angles.hpp>
#include <glm/gtc/matrix_transform.hpp>

#ifdef EMSCRIPTEN
#include "emscripten.h"
#endif
#include <glm/glm.hpp>

#include <glm/gtx/euler_angles.hpp>
#include <glm/gtc/matrix_transform.hpp>


using namespace sre;

class PolygonOffsetExample {
public:
    PolygonOffsetExample(){
        r.init();

        
        camera.lookAt({0,0,3},{0,0,0},{0,1,0});
        camera.setPerspectiveProjection(60,0.001,10000000);
        
        material = Shader::create().withSourceStandard().build()->createMaterial();
        material->setColor({1.0f,1.0f,1.0f,1.0f});
        material->setSpecularity(20.0f);

        material2 = Shader::create().withSourceStandard().withOffset(factor,offset).build()->createMaterial();
        material2->setColor({1.0f,0.0f,0.0f,1.0f});
        material2->setSpecularity(20.0f);

        mesh = Mesh::create().withQuad(100.9999999f).build();
        mesh2 = Mesh::create().withQuad(100.000001f).build();
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
        
        glm::mat4 rot = glm::eulerAngleY(glm::radians((float)i))*glm::eulerAngleX(glm::radians((float)sin((float)i*0.01f)*30.8f));
        camera.lookAt((glm::vec3)(rot*glm::vec4(0,0,3,1)), {0,0,0}, {0,1,0});
        auto renderPass = RenderPass::create()
                .withCamera(camera)
                .withWorldLights(&worldLights)
                .withClearColor(true, {1, 0, 0, 1})
                .build();
        renderPass.draw(mesh, glm::mat4(1), material);
        renderPass.draw(mesh2, glm::mat4(1), material2);

        bool changed = ImGui::SliderFloat2("Factor/Offset",&factor,0,3);
        if (changed){
            material2 = Shader::create().withSourceStandard().withOffset(factor,offset).build()->createMaterial();
            material2->setColor({1.0f,0.0f,0.0f,1.0f});
            material2->setSpecularity(20.0f);
        }

        ImGui::Checkbox("Rotate",&rotate);

        if (rotate) i++;
    }
private:
    SDLRenderer r;
    Camera camera;
    WorldLights worldLights;
    std::shared_ptr<Mesh> mesh;
    std::shared_ptr<Mesh> mesh2;
    std::shared_ptr<Material> material;
    std::shared_ptr<Material> material2;
    float factor=0;
    float offset=0;
    int i=0;
    bool rotate = true;
};

int main() {
    new PolygonOffsetExample();
    return 0;
}
