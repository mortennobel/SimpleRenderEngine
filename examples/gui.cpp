#include <iostream>
#include <vector>
#include <fstream>

#include "sre/Texture.hpp"
#include "sre/Renderer.hpp"
#include "sre/Profiler.hpp"
#include "sre/Material.hpp"

#include <glm/gtx/euler_angles.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "sre/SDLRenderer.hpp"

using namespace sre;

class GUIExample {
public:
    GUIExample()
    :r{},profiler{300,&r}
    {
        r.init();

        camera.lookAt({0,0,3},{0,0,0},{0,1,0});
        camera.setPerspectiveProjection(60,0.1,100);

        mesh = Mesh::create()
                .withCube()
                .build();
        worldLights.addLight(Light::create()
                                      .withPointLight({0, 0,10})
                                      .withColor({1,0,0})
                                      .withRange(50)
                                      .build());

        material = Shader::getStandard()->createMaterial();
        material->setSpecularity(20);

        // connect render callback
        r.frameRender = [&](){
            frameRender();
        };
        r.mouseEvent = [&](SDL_Event& e){
            if (e.type == SDL_MOUSEMOTION){
                rotation.x += glm::radians((float)e.motion.xrel);
                rotation.y += glm::radians((float)e.motion.yrel);
            }
        };
        // start render loop
        r.startEventLoop();
    }


    void frameRender(){
        bool showMouseCursor = r.isMouseCursorVisible();
        RenderPass rp = RenderPass::create()
                .withCamera(camera)
                .withWorldLights(&worldLights)
                .withClearColor(true,{clear_color.x,clear_color.y,clear_color.z,1.0f})
                .withGUI(showMouseCursor)
                .build();
        rp.draw(mesh, glm::eulerAngleY(rotation.x) * glm::eulerAngleX(rotation.y), material);

        if (!showMouseCursor){
            return;
        }

        bool open = true;

        // Show Label (with invisible window)
        ImGui::SetNextWindowPos(ImVec2(100,000));
        ImGui::Begin("#TestLabel",&open,ImVec2(500,100),0,ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoInputs);
        ImGui::Text("Hello, world!");
        ImGui::End();

        // Show Button (with invisible window)
        // Note window may disappear behind other windows
        ImGui::SetNextWindowPos(ImVec2(200,100));
        ImGui::Begin("#Button",&open,ImVec2(100,25),0,ImGuiWindowFlags_NoTitleBar|ImGuiWindowFlags_NoResize|ImGuiWindowFlags_NoMove|ImGuiWindowFlags_NoScrollbar);
        if (ImGui::Button("Click me")){
            std::cout << "Clicked"<<std::endl;
        }
        ImGui::End();
#ifndef EMSCRIPTEN
        bool fullscreen = r.isFullscreen();
        if (ImGui::Checkbox("Fullscreen",&fullscreen)){
            r.setFullscreen(fullscreen);
        }
        if (ImGui::Checkbox("Mouse cursor",&showMouseCursor)){
            r.setMouseCursorVisible(showMouseCursor);
        }
        bool showMouseLocked = r.isMouseCursorLocked();
        if (ImGui::Checkbox("Mouse locked",&showMouseLocked)){
            r.setMouseCursorLocked(showMouseLocked);
        }

#endif

        // 1. Show a simple window
        // Tip: if we don't call ImGui::Begin()/ImGui::End() the widgets appears in a window automatically called "Debug"
        {

            ImGui::Text("Hello, world!");
            ImGui::ColorEdit3("clearScreen color", (float*)&clear_color);

            ImGui::Checkbox("Another Window", &show_another_window);

            ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);


        }

        ImGui::ShowMetricsWindow();

        // 2. Show another simple window, this time using an explicit Begin/End pair
        if(show_another_window)
        {
            ImGui::SetNextWindowSize(ImVec2(200,100), ImGuiSetCond_FirstUseEver);
            ImGui::Begin("Another Window", &show_another_window);
            ImGui::Text("Hello");


            ImGui::End();
        }
        profiler.update();
        profiler.gui();
    }
private:
    SDLRenderer r;
    glm::vec2 rotation;
    Profiler profiler;
    bool show_another_window = false;
    ImVec4 clear_color = ImColor(114, 144, 154);
    std::shared_ptr<Mesh> mesh;
    std::shared_ptr<Material> material;
    Camera camera;
    WorldLights worldLights;
};



int main() {
    new GUIExample();

    return 0;
}

