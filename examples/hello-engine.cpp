#include <iostream>
#include <vector>
#include <fstream>

#include "sre/Texture.hpp"
#include "sre/Renderer.hpp"
#include "sre/Camera.hpp"
#include "sre/Mesh.hpp"
#include "sre/Shader.hpp"
#define SDL_MAIN_HANDLED
#include "SDL.h"

#include <glm/glm.hpp>

#include <glm/gtx/euler_angles.hpp>
#include <glm/gtc/matrix_transform.hpp>

using namespace sre;

int main() {
    SDL_Window *window;
    SDL_Init(SDL_INIT_VIDEO);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 1);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
    window = SDL_CreateWindow("Hello Engine",SDL_WINDOWPOS_UNDEFINED,SDL_WINDOWPOS_UNDEFINED,640,480,SDL_WINDOW_OPENGL);
    Renderer r{window};
    RenderPass rp = r.createRenderPass().build();
    rp.drawLines({{0,0,0},{1,1,1}});
    r.swapWindow();
    SDL_Delay(10000);
    SDL_DestroyWindow(window);
    SDL_Quit();

    return 0;
}

