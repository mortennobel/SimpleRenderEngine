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
#include <glm/gtc/random.hpp>

#include <glm/gtx/euler_angles.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <imgui.h>
#include <SRE/impl/GL.hpp>
#include "SRE/imgui_sre.hpp"
#ifdef EMSCRIPTEN
#include "emscripten.h"
#endif
using namespace SRE;

Mesh* createParticles(int size = 2500){
    std::vector<glm::vec3> positions;
    std::vector<glm::vec4> colors;
    std::vector<float> sizes;
    for (int i=0;i<size;i++){
        positions.push_back(glm::linearRand(glm::vec3(-1,-1,-1),glm::vec3(1,1,1)));
        colors.push_back(glm::linearRand(glm::vec4(0,0,0,0),glm::vec4(1,1,1,1)));
        sizes.push_back(glm::linearRand(0.0f,1.0f));
    }
    return Mesh::create ()
            .withVertexPositions(positions)
            .withColors(colors)
            .withParticleSize(sizes)
            .withMeshTopology(MeshTopology::Points)
            .build();
}

SDL_Window *window;
Shader *shader;
Mesh* particleMesh;
Mesh *mesh;
Shader* shaderParticles;
int i=0;
bool ortho = false;
bool quit = false;
void update();

int main() {
    std::cout << "Particle test" << std::endl;

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
            "An SDL2 window",                     // window title
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
    ImGui_SRE_Init(window);

    SimpleRenderEngine r{window};

    r.getCamera()->lookAt({0,0,3},{0,0,0},{0,1,0});
    r.getCamera()->setPerspectiveProjection(60,640,480,0.1,100);
    shader = Shader::getStandard();
    shader->set("specularity",20.0f);
    shader->set("tex",Texture::getWhiteTexture());
    shaderParticles = Shader::getStandardParticles();

    particleMesh = createParticles();
    mesh = Mesh::create()
            .withCube().build();
    r.setLight(0, Light::create().withPointLight({ 0, 1,0}).withColor({1,0,0}).withRange(2).build());
    r.setLight(1, Light::create().withPointLight({ 1, 0,0}).withColor({0,1,0}).withRange(2).build());
    r.setLight(2, Light::create().withPointLight({ 0,-1,0}).withColor({0,0,1}).withRange(2).build());
    r.setLight(3, Light::create().withPointLight({-1, 0,0}).withColor({1,1,1}).withRange(2).build());

#ifdef __EMSCRIPTEN__
    emscripten_set_main_loop(update, 0, 1);
#else
    while (!quit){

        update();
    }

    // Close and destroy the window
    SDL_DestroyWindow(window);

    // Clean up
    SDL_Quit();
#endif
    return 0;
}

void update(){
    SDL_Event e;
    //Handle events on queue
    while( SDL_PollEvent( &e ) != 0 )
    {
        ImGui_SRE_ProcessEvent(&e);
        if (e.type == SDL_QUIT)
            quit = true;
    }
    SimpleRenderEngine &r = *SimpleRenderEngine::instance;
    int w,h;
    SDL_GetWindowSize(window,&w,&h);
    r.getCamera()->setViewport(0,0,w,h);
    float aspect = w/(float)h;
    if (ortho){
        r.getCamera()->setOrthographicProjection(-2*aspect,2*aspect,-2,2,-2,100);
    } else {
        r.getCamera()->setPerspectiveProjection(60,w,h,0.1,100);
    }
    r.clearScreen({0,0,0.3,1});
    shader->set("tex", Texture::getWhiteTexture());
    r.draw(mesh, glm::eulerAngleY(-glm::radians((float)i))*glm::scale(glm::mat4(1),{0.3f,0.3f,0.3f}), shader);
    shaderParticles->set("tex", Texture::getSphereTexture());
    r.draw(particleMesh, glm::eulerAngleY(glm::radians((float)i)), shaderParticles);


    ImGui_SRE_NewFrame(window);

    // 1. Show a simple window
    // Tip: if we don't call ImGui::Begin()/ImGui::End() the widgets appears in a window automatically called "Debug"
    {

        ImGui::Text("Particle sprite");
        ImGui::Checkbox("Orthographic proj",&ortho);
        ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
        auto& renderStats = r.getRenderStats();

        float bytesToMB = 1.0f/(1024*1024);
        ImGui::Text("SRE draw-calls %i meshes %i (%.2fMB) textures %i (%.2fMB) shaders %i", renderStats.drawCalls,renderStats.meshCount, renderStats.meshBytes*bytesToMB, renderStats.textureCount, renderStats.textureBytes*bytesToMB, renderStats.shaderCount);
    }
    ImGui::Render();
    r.swapWindow();

    SDL_Delay(16);
    i++;
}