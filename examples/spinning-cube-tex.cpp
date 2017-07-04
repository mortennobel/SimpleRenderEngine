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

#include <glm/gtx/euler_angles.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <sre/SDLRenderer.hpp>

using namespace sre;

class SpinningCubeTexExample {
public:
    SpinningCubeTexExample(){
        r.init();
        camera = new Camera();
        camera->lookAt({0,0,3},{0,0,0},{0,1,0});
        camera->setPerspectiveProjection(60,0.1,100);
        shader = Shader::getUnlit();
        mat = new Material(shader);
        mat->setTexture(Texture::create().withFile("examples-data/test.jpg").withGenerateMipmaps(true).build());
        mesh = Mesh::create()
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
        renderPass.draw(mesh, glm::eulerAngleY(glm::radians( i * speed)), mat);
        i++;
    }
private:
    SDLRenderer r;
    Camera* camera;
    Shader* shader;
    Material* mat;

    Mesh* mesh;
    int i=0;

};

int main() {
    new SpinningCubeTexExample();

    return 0;
}

