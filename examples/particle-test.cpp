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

using namespace SRE;

ParticleMesh* createParticles(int size = 2500){
    std::vector<glm::vec3> positions;
    std::vector<glm::vec4> colors;
    std::vector<glm::vec2> uvs;
    std::vector<float> scaleAndRotate;
    std::vector<float> sizes;
    for (int i=0;i<size;i++){
        positions.push_back(glm::linearRand(glm::vec3(-1,-1,-1),glm::vec3(1,1,1)));
        colors.push_back(glm::linearRand(glm::vec4(0,0,0,0),glm::vec4(1,1,1,1)));
        sizes.push_back(glm::linearRand(0.0f,500.0f));
    }
    return new ParticleMesh(positions,colors,uvs,scaleAndRotate,scaleAndRotate,sizes);
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
            "An SDL2 window",                  // window title
            SDL_WINDOWPOS_UNDEFINED,           // initial x position
            SDL_WINDOWPOS_UNDEFINED,           // initial y position
            640,                               // width, in pixels
            480,                               // height, in pixels
            SDL_WINDOW_OPENGL                  // flags - see below
    );

    // Check that the window was successfully made
    if (window == NULL) {
        // In the event that the window could not be made...
        printf("Could not create window: %s\n", SDL_GetError());
        return 1;
    }

    SimpleRenderEngine r{window};

    r.getCamera()->lookAt({0,0,3},{0,0,0},{0,1,0});
    r.getCamera()->setPerspectiveProjection(60,640,480,0.1,100);
    Shader* shader = Shader::getStandard();
    shader->set("specularity",20.0f);
    Shader* shaderParticles = Shader::getStandardParticles();

    ParticleMesh* particleMesh = createParticles();
    Mesh* mesh = Mesh::createCube();
    r.setLight(0, Light(LightType::Point,{0, 1,0},{0,0,0},{1,0,0},2));
    r.setLight(1, Light(LightType::Point,{1, 0,0},{0,0,0},{0,1,0},2));
    r.setLight(2, Light(LightType::Point,{0,-1,0},{0,0,0},{0,0,1},2));
    r.setLight(3, Light(LightType::Point,{-1,0,0},{0,0,0},{1,1,1},2));

    float duration = 5000;
    for (float i=0;i<duration ;i+=16){
        r.clearScreen({0,0,0.3,1});
        r.draw(mesh, glm::eulerAngleY(-glm::radians(360 * i / duration))*glm::scale(glm::mat4(1),{0.3f,0.3f,0.3f}), shader);
        r.draw(particleMesh, glm::eulerAngleY(glm::radians(360 * i / duration)), shaderParticles);

        r.swapWindow();

        SDL_Delay(16);
    }

    // Close and destroy the window
    SDL_DestroyWindow(window);

    // Clean up
    SDL_Quit();

    return 0;
}