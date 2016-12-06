#include <iostream>
#include <vector>
#include <fstream>

#include "SRE/Texture.hpp"
#include "SRE/SimpleRenderEngine.hpp"
#include "SRE/Camera.hpp"
#include "SRE/Mesh.hpp"
#include "SRE/Shader.hpp"
#define SDL_MAIN_HANDLED
#include "SDL.h"
#include <imgui.h>
#include "SRE/imgui_sre.hpp"

#include <glm/glm.hpp>

#include <glm/gtx/euler_angles.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <SRE/Debug.hpp>

using namespace SRE;

void drawCross(glm::vec3 p, float size = 0.3f){
    SRE::Debug::drawLine(p-glm::vec3{size,0,0}, p+glm::vec3{size,0,0});
    SRE::Debug::drawLine(p-glm::vec3{0,size,0}, p+glm::vec3{0,size,0});
    SRE::Debug::drawLine(p-glm::vec3{0,0,size}, p+glm::vec3{0,0,size});
}

void drawLight(Light& l, float size){
    if (l.lightType == LightType::Point || l.lightType == LightType::Directional){
        SRE::Debug::setColor({0,0,0,1});
        drawCross(l.position, size);
    }
    if (l.lightType == LightType::Directional){
        SRE::Debug::setColor({1,1,0,1});
        SRE::Debug::drawLine(l.position, l.position - l.direction*size*2.0f);
    }
}

int main() {
    std::cout << "Spinning sphere"<<std::endl;
    SDL_Window *window;                    // Declare a pointer

    SDL_Init(SDL_INIT_VIDEO);              // Initialize SDL2

    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 1);

    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK,SDL_GL_CONTEXT_PROFILE_CORE);

    // Create an application window with the following settings:
    window = SDL_CreateWindow(
            "An SDL2 window",                  // window title
            SDL_WINDOWPOS_UNDEFINED,           // initial x position
            SDL_WINDOWPOS_UNDEFINED,           // initial y position
            640,                               // width, in pixels
            480,                               // height, in pixels
            SDL_WINDOW_OPENGL                  // flags
    );

    // Check that the window was successfully made
    if (window == NULL) {
        // In the event that the window could not be made...
        printf("Could not create window: %s\n", SDL_GetError());
        return 1;
    }

    SimpleRenderEngine r{window};
    ImGui_SRE_Init(window);
    glm::vec3 eye{0,0,5};
    glm::vec3 at{0,0,0};
    glm::vec3 up{0,1,0};
    r.getCamera()->lookAt(eye,at, up);
    r.getCamera()->setPerspectiveProjection(60,640,480,0.1f,100);
    Shader* shader = Shader::getStandard();
    float specularity = 20;
    glm::vec4 color {1,1,1,1};
    shader->set("specularity", 20.0f);
    shader->set("color", color);
    Mesh* mesh = Mesh::createSphere();

    Light lights[] = {
            Light(LightType::Point,{0, 2,1},{0,0,0},{1,0,0},10),
            Light(LightType::Unused,{2, 0,1},{0,0,0},{0,1,0},10),
            Light(LightType::Unused,{0,-2,1},{0,0,0},{0,0,1},10),
            Light(LightType::Unused,{-2,0,1},{0,0,0},{1,1,1},10),
    };

    bool debugLight = true;
    bool animatedLight = true;
    bool animatedCamera = true;
    float debugLightSize = 0.2;
    bool quit = false;
    SDL_Event e;
    float time = 0;
    while (!quit){
        //Handle events on queue
        while( SDL_PollEvent( &e ) != 0 )
        {
            ImGui_SRE_ProcessEvent(&e);
            if (e.type == SDL_QUIT)
                quit = true;
        }
        r.clearScreen({1,0,0,1});
        drawCross({2,2,2});
        drawCross({-2,-2,-2});
        r.draw(mesh, glm::eulerAngleY(time), shader);
        time += 0.016f;

        ImGui_SRE_NewFrame(window);
        std::string labels[] = {
                "Light 1",
                "Light 2",
                "Light 3",
                "Light 4"
        };

        ImGui::DragFloat3("Camera",&eye.x);
        ImGui::Checkbox("AnimatedLight",&animatedLight);
        ImGui::Checkbox("AnimatedCamera",&animatedCamera);
        if (animatedCamera){
            eye = {
                sin(time*-0.2)*5.0f,
                        sin(time*-0.4)*0.5f,
                        cos(time*-0.2)*5.0f,
            };
        }
        r.getCamera()->lookAt(eye,at, up);
        if (animatedLight){
            lights[0].position = {
                    sin(time)*1.5f,
                    sin(time*2)*0.5f,
                    cos(time)*1.5f,
            };
        }
        ImGui::Checkbox("DebugLight",&debugLight);
        if (debugLight){
            ImGui::DragFloat("DebugLightSize", &debugLightSize,0.1f,0,3);
        }
        // Show Label (with invisible window)
        for (int i=0;i<4;i++){
            if (debugLight){
                drawLight(lights[i],debugLightSize);
            }
            if (ImGui::TreeNode(labels[i].c_str())){
                auto lightType = (int)lights[i].lightType;
                ImGui::RadioButton("Point", &lightType, 0); ImGui::SameLine();
                ImGui::RadioButton("Directional", &lightType, 1); ImGui::SameLine();
                ImGui::RadioButton("Unused", &lightType, 2);
                lights[i].lightType = (LightType)lightType;

                ImGui::ColorEdit3("Color", &(lights[i].color.x));
                ImGui::DragFloat3("Position", &(lights[i].position.x));
                ImGui::DragFloat3("Direction", &(lights[i].direction.x));
                ImGui::DragFloat("Range", &(lights[i].range),1,0,30);


                ImGui::TreePop();
            }
            r.setLight(i, lights[i]);
        }

        if (ImGui::TreeNode("Material")){
            ImGui::DragFloat("Specularity", &specularity,1,0,200);
            shader->set("specularity", specularity);
            ImGui::ColorEdit3("Color", &(color.x));
            shader->set("color", color);
            ImGui::TreePop();
        }

        ImGui::Render();

        r.swapWindow();
        SDL_Delay(16);
    }

    // Close and destroy the window
    SDL_DestroyWindow(window);

    // Clean up
    SDL_Quit();

    return 0;
}

