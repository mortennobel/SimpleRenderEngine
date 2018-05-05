#include <iostream>
#include <vector>
#include <fstream>

#include "sre/Texture.hpp"
#include "sre/Renderer.hpp"
#include "sre/Material.hpp"

#include <glm/gtx/transform.hpp>
#include <glm/gtx/euler_angles.hpp>
#include <sre/SDLRenderer.hpp>
#include <glm/gtx/string_cast.hpp>
#include <sre/Inspector.hpp>

using namespace sre;
using namespace std;

class PickingExample {
public:
    PickingExample(){
        r.init();

        camera.lookAt({0,0,6},{0,0,0},{0,1,0});
        camera.setPerspectiveProjection(60,0.1,100);
        for (int i=0;i<4;i++){
            mat[i] = Shader::getUnlit()->createMaterial();
            if (i==3){
                mat[i]->setColor({1,1,0,1});
            } else {
                Color color(0,0,0,1);
                color[i] = 1;
                mat[i]->setColor(color);
            }
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
        mesh[3] = Mesh::create()
                .withTorus()
                .build();

        r.frameRender = [&](){
            render();
        };

        r.mouseEvent = [&](SDL_Event& e){
            if (e.type == SDL_MOUSEMOTION){
                auto r = Renderer::instance;
                glm::vec2 pos{e.motion.x, r->getWindowSize().y - e.motion.y};

                // convert to pixel coordinates
                pos /= r->getWindowSize();
                pos *= r->getDrawableSize();

                mouseX = static_cast<int>(pos.x);
                mouseY = static_cast<int>(pos.y);
            }
            if (e.button.button==SDL_BUTTON_RIGHT){
                showInspector = true;
            }
        };
        r.startEventLoop();
    }

    void render(){
        // Render scene to framebuffer (without gui)
        auto sceneRenderPass = RenderPass::create()
                .withCamera(camera)
                .withClearColor(true,{0, 0, 0, 1})
                .withGUI(false)
                .build();
        const float speed = .5f;
        int index = 0;
        for (int x=0;x<2;x++){
            for (int y=0;y<2;y++){
                sceneRenderPass.draw(mesh[index], glm::translate(glm::vec3(-1.5+x*3,-1.5+y*3,0)), mat[index]);
                index++;
            }
        }
        sceneRenderPass.finish();

        auto pixelValues = sceneRenderPass.readPixels(mouseX, mouseY);           // read pixel values from framebuffer

        // render gui to framebuffer
        auto guiRenderPass = RenderPass::create()
                .withClearColor(false)
                .withGUI(true)
                .build();

        drawTopTextAndColor(pixelValue);
        static Inspector inspector;
        inspector.update();
        if (showInspector){
            inspector.gui();
        }

        guiRenderPass.finish();

        pixelValue = pixelValues[0];
    }

    void drawTopTextAndColor(sre::Color color){
        auto size = Renderer::instance->getWindowSize();
        ImVec2 imSize(size.x, 50.0f);
        ImVec2 imPos(0, 0);
        ImGui::SetNextWindowSize(imSize);                                   // imgui window size should have same width as SDL window size
        ImGui::SetNextWindowPos(imPos);
        ImGui::Begin("",nullptr,ImGuiWindowFlags_NoInputs|ImGuiWindowFlags_NoMove|ImGuiWindowFlags_NoTitleBar|ImGuiWindowFlags_NoScrollbar);

        ImGui::ColorEdit4("Selected color",&color.r, ImGuiColorEditFlags_NoInputs);
        ImGui::TextWrapped("Mouse pos %i %i",mouseX, mouseY);
        ImGui::End();
    }
private:
    SDLRenderer r;
    Camera camera;
    std::shared_ptr<Material> mat[4];
    std::shared_ptr<Mesh> mesh[4];
    Color pixelValue;
    int i=0;
    int mouseX;
    int mouseY;
    bool showInspector = false;
};

int main() {
    std::make_unique<PickingExample>();
    return 0;
}


