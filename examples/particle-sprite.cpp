#include <iostream>
#include <vector>
#include <fstream>

#include "SRE/Texture.hpp"
#include "SRE/SimpleRenderEngine.hpp"
#include "SRE/Camera.hpp"
#include "SRE/Mesh.hpp"
#include "SRE/ParticleMesh.hpp"
#include "SRE/Shader.hpp"
#define SDL_MAIN_HANDLED
#include "SDL.h"

#include <glm/glm.hpp>
#include <glm/gtc/random.hpp>

#include <glm/gtx/euler_angles.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <imgui.h>
#include "imgui_sre.hpp"

#include <imgui.h>
#include "imgui_sre.hpp"

using namespace SRE;

ParticleMesh* createParticles(){
    std::vector<glm::vec3> positions;
    std::vector<glm::vec4> colors;
    std::vector<glm::vec2> uvCenter;
    std::vector<float> empty;
    std::vector<float> sizes;

    positions.push_back({0,0,0});
    sizes.push_back(10.0f);

    return new ParticleMesh(positions,colors,uvCenter,empty,empty,sizes);
}

void updateParticlesAnimation(float time, glm::vec2& pos,float& size, float& rotation){
    int frame = ((int)(time*10))%16;
    int frameX = 3-frame%4;
    int frameY = frame/4;
    pos = glm::vec2(frameX * 0.25f, frameY * 0.25f);
    size = 0.25f;
    rotation = 0;
}

void updateParticles(ParticleMesh* particleMesh, glm::vec2 uv, float uvSize, float rotation, float size){
    std::vector<glm::vec3> positions;
    std::vector<glm::vec4> colors;
    std::vector<glm::vec2> uvCenter;
    std::vector<float> uvSizes;
    std::vector<float> uvRotations;
    std::vector<float> sizes;

    positions.push_back({0,0,0});
    uvCenter.push_back(uv);
    uvSizes.push_back(uvSize);
    uvRotations.push_back(rotation);
    sizes.push_back(size);

    particleMesh->update(positions, colors, uvCenter,uvSizes, uvRotations,sizes);
}

int main() {
    std::cout << "Spinning cube" << std::endl;
    SDL_Window *window;                    // Declare a pointer

    SDL_Init(SDL_INIT_VIDEO);              // Initialize SDL2

    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 1);

    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);

    // Create an application window with the following settings:
    window = SDL_CreateWindow(
            "An SDL2 window",                      // window title
            SDL_WINDOWPOS_UNDEFINED,               // initial x position
            SDL_WINDOWPOS_UNDEFINED,               // initial y position
            640,                                   // width, in pixels
            480,                                   // height, in pixels
            SDL_WINDOW_OPENGL|SDL_WINDOW_RESIZABLE // flags
    );

    // Check that the window was successfully made
    if (window == NULL) {
        // In the event that the window could not be made...
        printf("Could not create window: %s\n", SDL_GetError());
        return 1;
    }

    ImGui_SRE_Init(window);

    SimpleRenderEngine r{window};

    r.getCamera()->lookAt({0,0,3},{0,0,0},{0,1,0});
    r.getCamera()->setPerspectiveProjection(60,640,480,0.1,100);
    Shader* shaderParticles = Shader::getStandardParticles();
    shaderParticles->set("tex", Texture::createFromFile("examples-data/t_explosionsheet.png", true));
    ParticleMesh* particleMesh = createParticles();


    bool quit = false;
    glm::vec2 spriteUV = glm::vec2(0, 0);
    bool spriteAnimation = false;
    bool ortho = false;
    float uvSize = 1.0;
    float uvRotation = 0.0;
    float size = 10.0f;
    float time = 0;
    while (!quit){
        SDL_Event e;
        //Handle events on queue
        while( SDL_PollEvent( &e ) != 0 )
        {
            ImGui_SRE_ProcessEvent(&e);
            if (e.type == SDL_QUIT)
                quit = true;
        }
        int w,h;
        SDL_GetWindowSize(window,&w,&h);
        r.getCamera()->setViewport(0,0,w,h);
        if (ortho) {
            r.getCamera()->setOrthographicProjection(-4,4,-4,4,-4,100);
        } else {
            r.getCamera()->setPerspectiveProjection(60,w,h,0.1,100);
        }
        r.clearScreen({0,0,0.0,1});


        ImGui_SRE_NewFrame(window);

        // 1. Show a simple window
        // Tip: if we don't call ImGui::Begin()/ImGui::End() the widgets appears in a window automatically called "Debug"
        {

            ImGui::Text("Particle sprite");
            ImGui::Checkbox("Orthographic proj",&ortho);
            ImGui::Checkbox("Sprite animation",&spriteAnimation);
            if (spriteAnimation) {
                updateParticlesAnimation(time, spriteUV,uvSize, uvRotation);
            }
            ImGui::SliderFloat("size", &size, 0.0f, 100.0f);
            ImGui::DragFloat2("uv", &(spriteUV.x), 0.1f);
            ImGui::SliderFloat("uv", &uvSize, 0.0f, 1.0f);
            ImGui::SliderFloat("uvRotation", &uvRotation, 0.0f, 2*3.1415f);

            ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
        }
        updateParticles(particleMesh, spriteUV, uvSize, uvRotation, size);
        r.draw(particleMesh, glm::mat4(1), shaderParticles);

        ImGui::Render();

        r.swapWindow();

        SDL_Delay(16);
        time+=0.016f;
    }

    // Close and destroy the window
    SDL_DestroyWindow(window);

    // Clean up
    SDL_Quit();

    return 0;
}