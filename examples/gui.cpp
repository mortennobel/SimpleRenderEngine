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
#ifdef EMSCRIPTEN
#include "emscripten.h"
#endif

#include <glm/glm.hpp>

#include <glm/gtx/euler_angles.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <imgui.h>
#include "sre/imgui_sre.hpp"

using namespace sre;

SDL_Window *window;
float f = 1.0f;
bool quit = false;
float timeF = 0.0f;
bool show_another_window = false;
ImVec4 clear_color = ImColor(114, 144, 154);
Shader* shader;
Mesh* mesh;
Material* material;
Camera* camera;
WorldLights* worldLights;

void update();

int main() {
    std::cout << "GUI" << std::endl;

    SDL_Init(SDL_INIT_VIDEO);              // Initialize SDL2
#ifdef EMSCRIPTEN
    window = nullptr;
    SDL_Renderer *renderer = NULL;
    SDL_CreateWindowAndRenderer(640, 480, SDL_WINDOW_OPENGL, &window, &renderer);
#else
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 1);

    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);

    // Create an application window with the following settings:
    window = SDL_CreateWindow(
            "sre: GUI",                     // window title
            SDL_WINDOWPOS_UNDEFINED,              // initial x position
            SDL_WINDOWPOS_UNDEFINED,              // initial y position
            640,                                  // width, in pixels
            480,                                  // height, in pixels
            SDL_WINDOW_OPENGL|SDL_WINDOW_RESIZABLE// flags
    );
#endif



    // Check that the window was successfully made
    if (window == NULL) {
        // In the event that the window could not be made...
        printf("Could not create window: %s\n", SDL_GetError());
        return 1;
    }

    Renderer r{window};

    camera = new Camera();
    camera->lookAt({0,0,3},{0,0,0},{0,1,0});
    camera->setPerspectiveProjection(60,640,480,0.1,100);
    shader = Shader::getStandard();
    mesh = Mesh::create()
            .withCube()
            .build();

    worldLights = new WorldLights();
    worldLights->addLight(Light::create()
                                  .withPointLight({0, 0,10})
                                  .withColor({1,0,0})
                                  .withRange(50)
                                  .build());

    material = new Material(shader);
    material->setSpecularity(20);


#ifdef __EMSCRIPTEN__
    emscripten_set_main_loop(update, 0, 1);
#else
    while (!quit){
        update();
        SDL_Delay(16);
    }
#endif
    // Close and destroy the window
    SDL_DestroyWindow(window);

    // Clean up
    SDL_Quit();

    return 0;
}

void update(){
    Renderer &r = *Renderer::instance;
    SDL_Event e;

    //Handle events on queue
    while( SDL_PollEvent( &e ) != 0 )
    {
        ImGui_SRE_ProcessEvent(&e);
        if (e.type == SDL_QUIT)
            quit = true;
    }

    auto renderPass = r.createRenderPass()
            .withCamera(*camera)
            .withWorldLights(worldLights)
            .withClearColor(true,{clear_color.x,clear_color.y,clear_color.z,1.0f})
            .build();
    renderPass.draw(mesh, glm::eulerAngleY(timeF), material);

    bool open = true;

    // Show Label (with invisible window)
    ImGui::SetNextWindowPos(ImVec2(100,000));
    ImGui::Begin("#TestLabel",&open,ImVec2(500,100),0,ImGuiWindowFlags_NoTitleBar|ImGuiWindowFlags_NoResize|ImGuiWindowFlags_NoMove|ImGuiWindowFlags_NoScrollbar|ImGuiWindowFlags_NoSavedSettings|ImGuiWindowFlags_NoInputs);
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

    // 1. Show a simple window
    // Tip: if we don't call ImGui::Begin()/ImGui::End() the widgets appears in a window automatically called "Debug"
    {

        ImGui::Text("Hello, world!");
        ImGui::SliderFloat("rotationSpeed", &f, 0.0f, 1.0f);
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

    r.swapWindow();
    timeF += .016f*f;
}