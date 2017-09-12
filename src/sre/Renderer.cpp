/*
 *  SimpleRenderEngine (https://github.com/mortennobel/SimpleRenderEngine)
 *
 *  Created by Morten Nobel-Jørgensen ( http://www.nobel-joergnesen.com/ )
 *  License: MIT
 */

#if defined(_WIN32) // force high performance graphics card see https://github.com/grimfang4/sdl-gpu/issues/17
#include <windows.h> // <---- for the DWORD
extern "C"
{
__declspec(dllexport) unsigned long NvOptimusEnablement = 0x00000001;
__declspec(dllexport) int AmdPowerXpressRequestHighPerformance = 1;
}
#endif

#include "sre/Log.hpp"

#include "sre/Renderer.hpp"
#include "sre/Framebuffer.hpp"
#include "sre/Texture.hpp"

#include "sre/impl/GL.hpp"

namespace sre {
    Renderer* Renderer::instance = nullptr;

    Renderer::Renderer(SDL_Window * window)
    :window{window}
    {
        if (instance != nullptr){
            LOG_ERROR("Multiple versions of Renderer initialized. Only a single instance is supported.");
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
			LOG_FATAL("Error initializing OpenGL using GLEW: %s",glewGetErrorString(err));
		}
#elif defined __LINUX__
        glewExperimental = GL_TRUE;
        GLenum err = glewInit();
		if (GLEW_OK != err)
		{
			/* Problem: glewInit failed, something is seriously wrong. */
			LOG_FATAL("Error initializing OpenGL using GLEW: %s",glewGetErrorString(err));
		}
#endif

		std::string version = (char*)glGetString(GL_VERSION);
        LOG_INFO("OpenGL version %s",glGetString(GL_VERSION) );
        LOG_INFO("sre version %i.%i.%i",sre_version_major,sre_version_minor ,sre_version_point  );

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
        renderStatsLast = renderStats;
    }

    Renderer::~Renderer() {
        SDL_GL_DeleteContext(glcontext);
        instance = nullptr;
    }

    void Renderer::swapWindow() {
        if (RenderPass::instance != nullptr){
            RenderPass::finish();
        }

        renderStatsLast = renderStats;
        renderStats.frame++;
        renderStats.meshBytesAllocated=0;
        renderStats.meshBytesDeallocated=0;
        renderStats.textureBytesAllocated=0;
        renderStats.textureBytesDeallocated=0;
        renderStats.drawCalls=0;
        renderStats.stateChangesShader = 0;
        renderStats.stateChangesMesh = 0;
        renderStats.stateChangesMaterial = 0;
#ifndef EMSCRIPTEN
        SDL_GL_SwapWindow(window);
#endif
    }

    const RenderStats &Renderer::getRenderStats() {
        return renderStatsLast;
    }

    glm::ivec2 Renderer::getWindowSize() {
        glm::ivec2 win;
        SDL_GetWindowSize(window,&win.r,&win.g);
        return win;
    }

    glm::ivec2 Renderer::getDrawableSize() {
        glm::ivec2 win;
        SDL_GL_GetDrawableSize(window,&win.r,&win.g);
        return win;
    }
}
