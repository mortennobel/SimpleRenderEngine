#include <iostream>
#include <vector>
#include <fstream>

#include "sre/Texture.hpp"
#include "sre/Renderer.hpp"
#include "sre/Camera.hpp"
#include "sre/Mesh.hpp"
#include "sre/Material.hpp"
#include "sre/Shader.hpp"
#define SDL_MAIN_HANDLED
#include "SDL.h"

#include <glm/glm.hpp>
#include <glm/gtx/transform.hpp>
#include <glm/gtx/euler_angles.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <sre/SDLRenderer.hpp>

using namespace sre;

class SpinningPrimitivesTexExample {
public:
    SpinningPrimitivesTexExample(){
        r.init();
        camera = new Camera();
        camera->lookAt({0,0,6},{0,0,0},{0,1,0});
        camera->setPerspectiveProjection(60,0.1,100);
        mat = Shader::getUnlit()->createMaterial();
        mat->setTexture(Texture::create().withFile("examples/data/test.png").withGenerateMipmaps(true).build());
        mesh[0] = Mesh::create()
                .withQuad()
                .build();
        mesh[1] = Mesh::create()
                .withSphere()
                .build();
        mesh[2] = Mesh::create()
                .withCube()
                .build();


        r.frameRender = [&](Renderer* r){
            render(r);
        };
        r.startEventLoop();
    }

    void render(Renderer * r){
        auto renderPass = r->createRenderPass()
                .withCamera(*camera)
                .withClearColor(true,{1, 0, 0, 1})
                .build();

        const float speed = .5f;
        int index = 0;
        for (int x=0;x<2;x++){
            for (int y=0;y<2;y++){
                if (index<3){
                    renderPass.draw(mesh[index], glm::translate(glm::vec3(-1.5+x*3,-1.5+y*3,0))*glm::eulerAngleY(glm::radians( i * speed)), mat);
                }
                index++;
            }
        }
        i++;
    }
private:
    SDLRenderer r;
    Camera* camera;
    std::shared_ptr<Material> mat;
    std::shared_ptr<Mesh> mesh[3];
    int i=0;
};

int main() {
    new SpinningPrimitivesTexExample();

    return 0;
}

