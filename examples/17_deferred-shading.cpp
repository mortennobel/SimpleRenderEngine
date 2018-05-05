#include <iostream>

#include "sre/Texture.hpp"
#include "sre/Renderer.hpp"
#include "sre/Material.hpp"

#include <glm/gtx/euler_angles.hpp>
#include <glm/gtx/transform.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <sre/SDLRenderer.hpp>
#include <sre/impl/GL.hpp>
#include <sre/Inspector.hpp>
#include <sre/ModelImporter.hpp>

using namespace sre;

class Deferred{
public:
    Deferred(){
        r.init();

        mesh = sre::ModelImporter::importObj("examples_data/sponza/", "sponza.obj", materials);

        worldLights.setAmbientLight(glm::vec3{0.02f});
        lightDirection = glm::normalize(glm::vec3{1,1,1});
        worldLights.addLight(Light::create().withDirectionalLight(lightDirection).withColor(Color(1,1,1),7).build());

        camera.setPerspectiveProjection(fieldOfViewY,near,far);
        camera.lookAt(eye,at,{0,1,0});

        r.frameRender = [&](){
            render();
        };
        r.mouseEvent = [&](SDL_Event& event){
            if (event.type == SDL_MOUSEMOTION){
                float mouseSpeed = 1/50.0f;
                rotateY = event.motion.x*mouseSpeed;
                rotateX = event.motion.y*mouseSpeed;
                glm::vec4 atRel = glm::rotate(rotateY, glm::vec3(0,1,0))*glm::rotate(rotateX, glm::vec3(1,0,0))*glm::vec4(0,0,1,0);
                at = eye+glm::vec3(atRel);
                camera.lookAt(eye,at,{0,1,0});
            }
            if (event.button.button==SDL_BUTTON_RIGHT){
                showInspector = true;
            }
        };
        r.startEventLoop();
    }

    void render(){
        // render pass - render world with shadow lookup
        auto rp = RenderPass::create()
                .withCamera(camera)
                .withClearColor(true,{0,0,0,1})
                .withWorldLights(&worldLights)
                .build();

        rp.draw(mesh, glm::mat4(1),materials);


        static Inspector inspector;
        inspector.update();
        if (showInspector){
            inspector.gui();
        }
    }
private:
    float fieldOfViewY = 45;
    float near = 0.1;
    float far = 100;
    unsigned int shadowMapSize = 1024;
    glm::vec3 eye = {0,1.8,0};
    glm::vec3 at = {0,1.8,1};
    glm::vec3 lightDirection;
    SDLRenderer r;
    Camera shadowmapCamera;
    Camera camera;
    std::vector<std::shared_ptr<Material>> materials;
    std::shared_ptr<Mesh> mesh;
    float rotateX = 0;
    float rotateY = 0;
    int texture = 0;
    bool showInspector = false;
    WorldLights worldLights;
};

int main() {
    Deferred deferredExample;

    return 0;
}

