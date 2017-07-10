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

class PickingExample {
public:
    PickingExample(){
        r.init();
        camera = new Camera();
        camera->lookAt({0,0,6},{0,0,0},{0,1,0});
        camera->setPerspectiveProjection(60,0.1,100);
        for (int i=0;i<3;i++){
            mat[i] = Shader::getUnlit()->createMaterial();
            glm::vec4 color(0,0,0,1);
            color[i] = 1;
            mat[i]->setColor(color);
        }

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

        r.mouseEvent = [&](SDL_Event& e){
            if (e.type == SDL_MOUSEMOTION){
                mouseX = e.motion.x;
                mouseY = r.getWindowSize().y - e.motion.y;
            }
        };
        r.startEventLoop();
    }

    void render(Renderer * r){
        auto renderPass = r->createRenderPass()
                .withCamera(*camera)
                .withClearColor(true,{0, 0, 0, 1})
                .build();

        const float speed = .5f;
        int index = 0;
        for (int x=0;x<2;x++){
            for (int y=0;y<2;y++){
                if (index<3){
                    renderPass.draw(mesh[index], glm::translate(glm::vec3(-1.5+x*3,-1.5+y*3,0)), mat[index]);
                }
                index++;
            }
        }
        // read pixel values from defualt framebuffer (before gui is rendered)
        auto pixelValues = renderPass.readPixels(mouseX, mouseY);

        // render color using imgui
        ImVec4 v4 {pixelValues[0].x, pixelValues[0].y, pixelValues[0].z, pixelValues[0].w};
        ImGui::ValueColor("Selected color",v4);
    }
private:
    SDLRenderer r;
    Camera* camera;
    std::shared_ptr<Material> mat[3];
    std::shared_ptr<Mesh> mesh[3];
    int i=0;
    int mouseX;
    int mouseY;
};

int main() {
    new PickingExample();

    return 0;
}


