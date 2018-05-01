#include <iostream>

#include "sre/Texture.hpp"
#include "sre/Renderer.hpp"
#include "sre/Material.hpp"

#include <glm/gtx/euler_angles.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <sre/SDLRenderer.hpp>
#include <sre/impl/GL.hpp>
#include <sre/Inspector.hpp>

using namespace sre;

class Spheres{
public:
    Spheres(){
        r.init();

        camera.lookAt({0,0,3},{0,0,0},{0,1,0});
        camera.setPerspectiveProjection(60,0.1f,100);

        mesh = Mesh::create()
                .withSphere()
                .build();

        worldLights.addLight(Light::create()
                 .withDirectionalLight(glm::normalize(glm::vec3(1,1,1)))
                 .build());

        mat1 = Shader::getStandardBlinnPhong()->createMaterial();
        mat1->setColor({1,1,1,1});
        mat1->setSpecularity(Color(1,1,1,50));

        mat2 = Shader::getUnlit()->createMaterial();
        mat2->setColor({1,1,0,1});
        mat2->setSpecularity(Color(0,0,0,0));

        r.frameUpdate = [&](float deltaTime){
            update(deltaTime);
        };
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

    void update(float deltaTime){
        time += deltaTime;
    }

    void render(){
        auto rp = RenderPass::create()
                .withCamera(camera)
                .withWorldLights(&worldLights)
                .withClearColor(true,{1,0,0,1})
                .build();

        rp.draw(mesh, pos1, mat1);

        rp.draw(mesh, pos2, mat2);
        
        static sre::Inspector inspector;
        inspector.update();
        if (showInspector){
            inspector.gui();
        }
    }
private:
    float time;
    SDLRenderer r;
    Camera camera;
    std::shared_ptr<Mesh> mesh;
    WorldLights worldLights;
    std::shared_ptr<Material> mat1;
    std::shared_ptr<Material> mat2;
    bool showInspector = false;
    glm::mat4 pos1 = glm::translate(glm::mat4(1), {-1,0,0});
    glm::mat4 pos2 = glm::translate(glm::mat4(1), {1,0,0});
};

int main() {
    Spheres spheres;
    return 0;
}

