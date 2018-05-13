#include <iostream>
#include <vector>
#include <fstream>

#include "sre/Texture.hpp"
#include "sre/Renderer.hpp"
#include "sre/Material.hpp"
#include "sre/SDLRenderer.hpp"


#include <glm/gtx/euler_angles.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/transform.hpp>
#include <sre/Inspector.hpp>


using namespace sre;

class SpinningCubeExample {
public:
    SpinningCubeExample(){
        r.init();

        camera.lookAt({0,0,3},{0,0,0},{0,1,0});
        camera.setPerspectiveProjection(60,0.1,100);

        materialPhong = Shader::getStandardBlinnPhong()->createMaterial();
        materialPhong->setColor({1.0f,1.0f,1.0f,1.0f});
        materialPhong->setSpecularity(Color(.5,.5,.5,180.0f));

        mesh = Mesh::create().withCube().build();
        worldLights.setAmbientLight({0.0,0.0,0.0});
        worldLights.addLight(Light::create().withPointLight({0, 3,0}).withColor({1,0,0}).withRange(20).build());
        worldLights.addLight(Light::create().withPointLight({3, 0,0}).withColor({0,1,0}).withRange(20).build());
        worldLights.addLight(Light::create().withPointLight({0,-3,0}).withColor({0,0,1}).withRange(20).build());
        worldLights.addLight(Light::create().withPointLight({-3,0,0}).withColor({1,1,1}).withRange(20).build());

        r.frameRender = [&](){
            render();
        };
        r.mouseEvent = [&](SDL_Event& event){
            if (event.button.button==SDL_BUTTON_RIGHT){
                showInspector = true;
            }
        };
        r.startEventLoop();
    }

    void render(){
        auto renderPass = RenderPass::create()
                .withCamera(camera)
                .withWorldLights(&worldLights)
                .withClearColor(true, {1, 0, 0, 1})
                .build();
        renderPass.draw(mesh, glm::eulerAngleY(glm::radians((float)i*0.1f)), materialPhong);
        i++;
        static Inspector inspector;
        inspector.update();
        if (showInspector){
            inspector.gui();
        }
    }
private:
    SDLRenderer r;
    Camera camera;
    WorldLights worldLights;
    std::shared_ptr<Mesh> mesh;
    std::shared_ptr<Material> materialPhong;
    int i=0;
    bool showInspector = false;
};

int main() {
     
    std::make_unique<SpinningCubeExample>();
    return 0;
}
