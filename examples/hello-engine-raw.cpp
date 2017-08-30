#include "sre/Renderer.hpp"

#define SDL_MAIN_HANDLED
#include "SDL.h"

using namespace sre;
using namespace std;

int main() {
    SDL_Window *window;
    SDL_Init(SDL_INIT_VIDEO);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 1);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
    window = SDL_CreateWindow("Hello Engine",SDL_WINDOWPOS_UNDEFINED,SDL_WINDOWPOS_UNDEFINED,640,480,SDL_WINDOW_OPENGL);
    Renderer r{window};
    RenderPass rp = RenderPass::create().build();
    rp.drawLines({{0,0,0},{1,1,1}});
    r.swapWindow();
    SDL_Delay(10000);
    SDL_DestroyWindow(window);
    SDL_Quit();

    return 0;
}