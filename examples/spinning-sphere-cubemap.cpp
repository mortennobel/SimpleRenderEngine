#include <iostream>
#include <vector>
#include <fstream>

#include "SRE/Texture.hpp"
#include "SRE/Renderer.hpp"
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

    Renderer r{window};
    ImGui_SRE_Init(window);
    glm::vec3 eye{0,0,5};
    glm::vec3 at{0,0,0};
    glm::vec3 up{0,1,0};
    r.getCamera()->lookAt(eye,at, up);
    r.getCamera()->setPerspectiveProjection(60,640,480,0.1f,100);
    const char* vertexShaderStr = R"(#version 140
in vec4 position;
in vec3 normal;
in vec2 uv;
out vec3 vNormal;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;
uniform mat3 normalMat;

void main(void) {
    gl_Position = projection * view * model * position;
    vNormal = normal;
}
)";
    const char* fragmentShaderStr = R"(#version 140
out vec4 fragColor;
in vec3 vNormal;

uniform samplerCube tex;

void main(void)
{
    fragColor = texture(tex, vNormal);
}
)";
    Shader* shader = Shader::create().withSource(vertexShaderStr, fragmentShaderStr).build();

    auto tex = Texture::create()
            .withFileCubemap("examples-data/cube-posx.png", Texture::TextureCubemapSide::PositiveX)
            .withFileCubemap("examples-data/cube-negx.png", Texture::TextureCubemapSide::NegativeX)
            .withFileCubemap("examples-data/cube-posy.png", Texture::TextureCubemapSide::PositiveY)
            .withFileCubemap("examples-data/cube-negy.png", Texture::TextureCubemapSide::NegativeY)
            .withFileCubemap("examples-data/cube-posz.png", Texture::TextureCubemapSide::PositiveZ)
            .withFileCubemap("examples-data/cube-negz.png", Texture::TextureCubemapSide::NegativeZ)
            .build();

    shader->set("tex", tex);

    Mesh* mesh = Mesh::create().withSphere().build();


    bool animatedCamera = true;
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

        r.draw(mesh, glm::eulerAngleY(time), shader);
        time += 0.016f;

        ImGui_SRE_NewFrame(window);

        ImGui::DragFloat3("CameraPos",&eye.x);
        if (animatedCamera){
            eye = {
                sin(time*-0.2)*5.0f,
                        sin(time*-0.4)*0.5f,
                        cos(time*-0.2)*5.0f,
            };
        }
        r.getCamera()->lookAt(eye,at, up);

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

