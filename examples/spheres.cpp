#include <iostream>

#include "sre/Texture.hpp"
#include "sre/Renderer.hpp"
#include "sre/Material.hpp"


#include <glm/gtx/euler_angles.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <sre/SDLRenderer.hpp>

using namespace sre;

class SpheresExample{
public:
    SpheresExample(){
        r.init();
        camera = new Camera();
        camera->lookAt({0,0,3},{0,0,0},{0,1,0});
        camera->setPerspectiveProjection(60,0.1f,100);

        shader = Shader::getStandard();
        mesh = Mesh::create()
                .withSphere()
                .build();


        worldLights.addLight(Light::create()
                                     .withDirectionalLight(glm::normalize(glm::vec3(1,1,1)))
                                     .build());

        mat1 = new Material(shader);
        mat1->setColor({1,1,1,1});
        mat1->setSpecularity(50);

        mat2 = new Material(shader);
        mat2->setColor({1,0,0,1});
        mat2->setSpecularity(0);

        r.frameUpdate = [&](float deltaTime){
            update(deltaTime);
        };
        r.frameRender = [&](Renderer* r){
            render(r);
        };
        r.startEventLoop();
    }

    void update(float deltaTime){
        time += deltaTime;
    }

    void render(Renderer* r){
        auto rp = r->createRenderPass()
                .withCamera(*camera)
                .withWorldLights(&worldLights)
                .withClearColor(true,{1,0,0,1})
                .build();

        rp.draw(mesh, pos1, mat1);
        rp.draw(mesh, pos2, mat2);

    }
private:
    float time;
    SDLRenderer r;
    Camera* camera;
    std::shared_ptr<Shader> shader;
    std::shared_ptr<Mesh> mesh;
    WorldLights worldLights;
    Material* mat1;
    Material* mat2;
    glm::mat4 pos1 = glm::translate(glm::mat4(1), {-1,0,0});
    glm::mat4 pos2 = glm::translate(glm::mat4(1), {1,0,0});
};

int main() {
    new SpheresExample();

    return 0;
}

