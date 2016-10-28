//
// Created by morten on 31/07/16.
//

#include "SRE/SimpleRenderEngine.hpp"
#include <cassert>
#include "SRE/Shader.hpp"
#include "SRE/Mesh.hpp"

#include "SRE/GL.hpp"
#include <iostream>
#include <algorithm>

namespace SRE {
    SimpleRenderEngine* SimpleRenderEngine::instance = nullptr;

    SimpleRenderEngine::SimpleRenderEngine(SDL_Window * window)
    :window{window}
    {
        if (instance != nullptr){
            std::cerr << "Multiple versions of SimpleRenderEngine initialized. Only a single instance is supported." << std::endl;
        }
        instance = this;
        camera = &defaultCamera;
        glcontext = SDL_GL_CreateContext(window);

#if defined(_WIN32)
		glewExperimental = GL_TRUE;
		GLenum err = glewInit();
		if (GLEW_OK != err)
		{
			/* Problem: glewInit failed, something is seriously wrong. */
			fprintf(stderr, "Error: %s\n", glewGetErrorString(err));
		}
#elif defined __LINUX__
		glewInit();
#endif

        std::cout << glGetString(GL_VERSION) << std::endl;
        std::cout << "OpenGL version "<<glGetString(GL_VERSION) << std::endl;
        std::cout << "SRE version "<<sre_version_major<<"."<<sre_version_minor << std::endl;


        // setup opengl context
        glEnable(GL_CULL_FACE);
    }

    SimpleRenderEngine::~SimpleRenderEngine() {
        SDL_GL_DeleteContext(glcontext);
        instance = nullptr;
    }

    void SimpleRenderEngine::setLight(int lightIndex, Light light) {
        assert(lightIndex >= 0);
        assert(lightIndex < maxSceneLights);
        sceneLights[lightIndex] = light;
    }

    Light SimpleRenderEngine::getLight(int lightIndex) {
        assert(lightIndex >= 0);
        assert(lightIndex < maxSceneLights);
        return sceneLights[lightIndex];
    }

    void SimpleRenderEngine::draw(Mesh *mesh, glm::mat4 modelTransform, Shader *shader) {
        if (camera == nullptr){
            std::cerr<<"Cannot render. Camera is null"<<std::endl;
            return;
        }

        shader->bind();
        shader->set("model", modelTransform);
        shader->set("view", camera->getViewTransform());
        shader->set("projection", camera->getProjectionTransform());
        auto normalMatrix = glm::transpose(glm::inverse((glm::mat3)(camera->getViewTransform()*modelTransform)));
        shader->set("normalMat", normalMatrix);
        shader->setLights(sceneLights, ambientLight, camera->getViewTransform());

        mesh->bind();

        glDrawArrays((GLenum) mesh->getMeshTopology(), 0, mesh->getVertexCount());
    }

    void SimpleRenderEngine::setCamera(Camera *camera) {
        this->camera = camera;
        camera->setViewport(camera->viewportX, camera->viewportY, camera->viewportWidth, camera->viewportHeight);
    }

    void SimpleRenderEngine::clearScreen(glm::vec4 color, bool clearColorBuffer, bool clearDepthBuffer) {
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

    void SimpleRenderEngine::swapWindow() {
        SDL_GL_SwapWindow(window);
    }

    Camera *SimpleRenderEngine::getCamera() {
        return camera;
    }

    Camera *SimpleRenderEngine::getDefaultCamera() {
        return &defaultCamera;
    }

    glm::vec3 SimpleRenderEngine::getAmbientLight() const {
        return glm::vec3(ambientLight);
    }

    void SimpleRenderEngine::setAmbientLight(const glm::vec3 &ambientLight) {
        float maxAmbient = std::max(ambientLight.x, std::max(ambientLight.y,ambientLight.z));
        SimpleRenderEngine::ambientLight = glm::vec4(ambientLight, maxAmbient);
    }
}
