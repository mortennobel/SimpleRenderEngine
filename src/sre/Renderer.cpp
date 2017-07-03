//
// Created by morten on 31/07/16.
//

#if defined(_WIN32) // force high performance graphics card see https://github.com/grimfang4/sdl-gpu/issues/17
#include <windows.h> // <---- for the DWORD
extern "C"
{
__declspec(dllexport) unsigned long NvOptimusEnablement = 0x00000001;
__declspec(dllexport) int AmdPowerXpressRequestHighPerformance = 1;
}
#endif

#include "sre/Renderer.hpp"
#include <cassert>
#include "sre/Shader.hpp"

#include "sre/Mesh.hpp"

#include "sre/impl/GL.hpp"
#include <iostream>
#include <algorithm>
#include <string>
#include <sre/imgui_sre.hpp>

namespace sre {
    Renderer* Renderer::instance = nullptr;

    Renderer::Renderer(SDL_Window * window)
    :window{window}
    {
        if (instance != nullptr){
            std::cerr << "Multiple versions of Renderer initialized. Only a single instance is supported." << std::endl;
        }
        // initialize ImGUI
        ImGui_SRE_Init(window);

        instance = this;
#ifndef EMSCRIPTEN
        glcontext = SDL_GL_CreateContext(window);
#endif
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

		std::string version = (char*)glGetString(GL_VERSION);
        std::cout << "OpenGL version "<<glGetString(GL_VERSION) << std::endl;
        std::cout << "sre version "<<sre_version_major<<"."<<sre_version_minor <<"."<<sre_version_point << std::endl;

        // setup opengl context
        glEnable(GL_DEPTH_TEST);
        glEnable(GL_CULL_FACE);
#ifndef EMSCRIPTEN
        glPointParameteri(GL_POINT_SPRITE_COORD_ORIGIN,GL_LOWER_LEFT);
        glEnable(GL_VERTEX_PROGRAM_POINT_SIZE);
#endif
#ifndef GL_POINT_SPRITE
#define GL_POINT_SPRITE 0x8861
#endif // !GL_POINT_SPRITE
		if (version.find_first_of("3.1") == 0){
			glEnable(GL_POINT_SPRITE);
		}

        // reset render stats
        renderStats.frame = 0;
        renderStats.meshCount = 0;
        renderStats.meshBytes = 0;
        renderStats.textureCount = 0;
        renderStats.textureBytes = 0;
        renderStats.drawCalls = 0;
        renderStatsLast = renderStats;
    }

    Renderer::~Renderer() {
        SDL_GL_DeleteContext(glcontext);
        instance = nullptr;
    }

    RenderPass::RenderPassBuilder Renderer::createRenderPass(){
        return RenderPass::RenderPassBuilder(&renderStats);
    }

    void Renderer::swapWindow() {
        if (RenderPass::instance){
            RenderPass::instance->finish();
        }

        renderStatsLast = renderStats;
        renderStats.frame++;
        renderStats.drawCalls=0;
        renderStats.stateChangesShader = 0;
        renderStats.stateChangesMesh = 0;
        renderStats.stateChangesMaterial = 0;
#ifndef EMSCRIPTEN
        SDL_GL_SwapWindow(window);
#endif
    }

    void Renderer::finishGPUCommandBuffer() {
        glFinish();
    }

    const RenderStats &Renderer::getRenderStats() {
        return renderStatsLast;
    }

    glm::ivec2 Renderer::getWindowSize() {
        glm::ivec2 win;
        SDL_GetWindowSize(window,&win.r,&win.g);
        return win;
    }
}
