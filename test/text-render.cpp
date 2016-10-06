#include <iostream>
#include <vector>
#include <fstream>

#include "SRE/Texture.hpp"
#include "SRE/SimpleRenderEngine.hpp"
#include "SRE/Camera.hpp"
#include "SRE/Mesh.hpp"
#include "SRE/Shader.hpp"
#include "SRE/Text.hpp"
#define SDL_MAIN_HANDLED
#include "SDL.h"

#include <glm/glm.hpp>

#include <glm/gtx/euler_angles.hpp>
#include <glm/gtc/matrix_transform.hpp>

using namespace SRE;

int main() {
    SDL_Window *window;                    // Declare a pointer

    SDL_Init(SDL_INIT_VIDEO);              // Initialize SDL2

    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 1);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);

    window = SDL_CreateWindow(
            "Text render",                     // window title
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

    r.clearScreen({1,0,0,1});
    SimpleRenderEngine::instance->getCamera()->setWindowCoordinates();
    // Font shader - user correct texture
    Shader * fontShader = Shader::getFont();
    // create a text mesh (pivot point in lower left corner)
    Mesh* helloWorld = Text::createTextMesh("Hello world");

    SimpleRenderEngine::instance->draw(helloWorld, glm::mat4(1), fontShader);

    r.swapWindow();
    SDL_Delay(10000);

    // Close and destroy the window
    SDL_DestroyWindow(window);

    // Clean up
    SDL_Quit();

    return 0;
}

