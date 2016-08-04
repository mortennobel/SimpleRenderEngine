//
// Created by morten on 31/07/16.
//

#include "Renderer.h"
#include <cassert>
#include "Shader.h"
#include "Mesh.h"

#if defined(_WIN32)
#   define GLEW_STATIC
#   include <GL/glew.h>
#else
#   include <OpenGL/gl3.h>
#endif
#include <iostream>

namespace SRE {
    Renderer* Renderer::instance = nullptr;

    Renderer::Renderer(SDL_Window *window)
    :window{window}
    {
        instance = this;
        camera = &defaultCamera;
        glcontext = SDL_GL_CreateContext(window);
        std::cout << glGetString(GL_VERSION)<<std::endl;
        std::cout << "OpenGL version "<<glGetString(GL_VERSION)<<std::endl;
        std::cout << "SRE version "<<sre_version_major<<"."<<sre_version_minor<<std::endl;

        // setup opengl context
        glEnable(GL_CULL_FACE);
    }

    Renderer::~Renderer() {
        SDL_GL_DeleteContext(glcontext);
    }

    void Renderer::setLight(int lightIndex, Light light) {
        assert(lightIndex >= 0);
        assert(lightIndex < maxSceneLights);
        sceneLights[lightIndex] = light;
    }

    Light Renderer::getLight(int lightIndex) {
        assert(lightIndex >= 0);
        assert(lightIndex < maxSceneLights);
        return sceneLights[lightIndex];
    }

    void Renderer::render(Mesh *mesh, glm::mat4 modelTransform, Shader *shader) {
        if (camera == nullptr){
            std::cerr<<"Cannot render. Camera is null"<<std::endl;
            return;
        }

        shader->bind();
        shader->setMatrix("model", modelTransform);
        shader->setMatrix("view", camera->getViewTransform());
        shader->setMatrix("projection", camera->getProjectionTransform());
        auto normalMatrix = glm::transpose(glm::inverse((glm::mat3)(camera->getViewTransform()*modelTransform)));
        shader->setMatrix("normalMat", normalMatrix);
        shader->setLights(sceneLights);

        mesh->bind();

        glDrawArrays((GLenum) mesh->getMeshTopology(), 0, mesh->getVertexCount());
    }

    void Renderer::setCamera(Camera *camera) {
        this->camera = camera;
        camera->setViewport(camera->viewportX, camera->viewportY, camera->viewportWidth, camera->viewportHeight);
    }

    void Renderer::clearScreen(glm::vec4 color, bool clearColorBuffer, bool clearDepthBuffer) {
        glClearColor(color.r, color.g, color.b, color.a);
        GLbitfield clear = 0;
        if (clearColorBuffer){
            clear |= GL_COLOR_BUFFER_BIT;
        }
        if (clearDepthBuffer){
            clear |= GL_DEPTH_BUFFER_BIT;
        }
        glClear(clear);
    }

    void Renderer::swapWindow() {
        SDL_GL_SwapWindow(window);
    }


    Camera *Renderer::getCamera() {
        return camera;
    }

    Camera *Renderer::getDefaultCamera() {
        return &defaultCamera;
    }
}