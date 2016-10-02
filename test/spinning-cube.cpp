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

#include <glm/glm.hpp>

#include <glm/gtx/euler_angles.hpp>
#include <glm/gtc/matrix_transform.hpp>

using namespace SRE;

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
    shader->setFloat("specularity",20);
    Mesh* mesh = Mesh::createCube();
    r.setLight(0, Light(LightType::Point,{0, 1,0},{0,0,0},{1,0,0},2));
    r.setLight(1, Light(LightType::Point,{1, 0,0},{0,0,0},{0,1,0},2));
    r.setLight(2, Light(LightType::Point,{0,-1,0},{0,0,0},{0,0,1},2));
    r.setLight(3, Light(LightType::Point,{-1,0,0},{0,0,0},{1,1,1},2));

    float duration = 10000;
    for (float i=0;i<duration ;i+=16){
        r.clearScreen({1,0,0,1});
        r.draw(mesh, glm::eulerAngleY(glm::radians(360 * i / duration)), shader);
        r.swapWindow();
        SDL_Delay(16);
    }

    // Close and destroy the window
    SDL_DestroyWindow(window);

    // Clean up
    SDL_Quit();

    return 0;
}

