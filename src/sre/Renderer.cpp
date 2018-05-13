/*
 *  SimpleRenderEngine (https://github.com/mortennobel/SimpleRenderEngine)
 *
 *  Created by Morten Nobel-Jørgensen ( http://www.nobel-joergensen.com/ )
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
#include "sre/VR.hpp"

#include "sre/Renderer.hpp"
#include "sre/Framebuffer.hpp"
#include "sre/Texture.hpp"

#include "sre/impl/GL.hpp"

#ifdef EMSCRIPTEN
#include "emscripten.h"
#endif

namespace sre {
    Renderer* Renderer::instance = nullptr;
    RenderInfo renderInfo_;

    const RenderInfo& renderInfo(){
        return renderInfo_;
    }

    Renderer::Renderer(SDL_Window * window, bool vsync_, int maxSceneLights)
    :window{window},vsync(vsync_),maxSceneLights(maxSceneLights)
    {
        if (instance != nullptr){
            LOG_ERROR("Multiple versions of Renderer initialized. Only a single instance is supported.");
        }


		instance = this;

        glcontext = SDL_GL_CreateContext(window);
        renderInfo_.graphicsAPIVersion = (char*)glGetString(GL_VERSION);
#ifdef EMSCRIPTEN


        bool isWebGL1 = strcmp((const char*)renderInfo_.graphicsAPIVersion.c_str(), "WebGL 1") != -1;
        if (isWebGL1){
            const char* embeddedCode =
                "window.WebGL2RenderingContext = window.WebGL2RenderingContext || window.WebGLRenderingContext;\n"
                "window.WebGLQuery = window.WebGLQuery || function(){};"
                "window.WebGLSampler = window.WebGLSampler || function(){};"
                "window.WebGLSync = window.WebGLSync || function(){};"
                "window.WebGLTransformFeedback = window.WebGLTransformFeedback || function(){};"
                "window.WebGLVertexArrayObject = window.WebGLVertexArrayObject || function(){};";
            emscripten_run_script(embeddedCode);
        }
        vsync = true; // WebGL uses vsync like approach
        renderInfo_.graphicsAPIVersionES = true;
        if (renderInfo_.graphicsAPIVersion.find("WebGL 1.0") != std::string::npos){
            renderInfo_.graphicsAPIVersionMajor = 2;
            renderInfo_.graphicsAPIVersionMinor = 0;
        } else {
            renderInfo_.graphicsAPIVersionMajor = 3;
            renderInfo_.graphicsAPIVersionMinor = 0;
        }
        renderInfo_.useFramebufferSRGB = false;
        renderInfo_.supportTextureSamplerSRGB = false;
#else
        if (vsync){
            vsync = SDL_GL_SetSwapInterval(1) == 0; // return 0 is success
        }
        renderInfo_.useFramebufferSRGB = true;
        renderInfo_.graphicsAPIVersionES = false;
        renderInfo_.supportTextureSamplerSRGB = true;
        glEnable(GL_FRAMEBUFFER_SRGB);
        glGetIntegerv(GL_MAJOR_VERSION,&renderInfo_.graphicsAPIVersionMajor);
        glGetIntegerv(GL_MINOR_VERSION,&renderInfo_.graphicsAPIVersionMinor);
#endif
#if defined(_WIN32) || defined(__LINUX__)
		glewExperimental = GL_TRUE;
		GLenum err = glewInit();
		if (GLEW_OK != err)
		{
			/* Problem: glewInit failed, something is seriously wrong. */
			LOG_FATAL("Error initializing OpenGL using GLEW: %s",glewGetErrorString(err));
		}
#endif
        renderInfo_.graphicsAPIVendor = (char*)glGetString(GL_VENDOR);

        LOG_INFO("OpenGL version %s (%i.%i)",renderInfo_.graphicsAPIVersion.c_str(), renderInfo_.graphicsAPIVersionMajor,renderInfo_.graphicsAPIVersionMinor);
        LOG_INFO("sre version %i.%i.%i", sre_version_major, sre_version_minor , sre_version_point);

        // setup opengl context
        glEnable(GL_DEPTH_TEST);
#ifndef EMSCRIPTEN
        glPointParameteri(GL_POINT_SPRITE_COORD_ORIGIN,GL_LOWER_LEFT);
        glEnable(GL_VERTEX_PROGRAM_POINT_SIZE);
#endif
#ifndef GL_POINT_SPRITE
#define GL_POINT_SPRITE 0x8861
#endif // !GL_POINT_SPRITE

		if (!renderInfo_.graphicsAPIVersionES && (renderInfo_.graphicsAPIVersionMajor == 3 && renderInfo_.graphicsAPIVersionMinor <= 1)){
			glEnable(GL_POINT_SPRITE);
		}
		if (!renderInfo_.graphicsAPIVersionES && (
		        (renderInfo_.graphicsAPIVersionMajor == 3 && renderInfo_.graphicsAPIVersionMinor>=2) ||
                (renderInfo_.graphicsAPIVersionMajor > 3))){
#ifndef GL_TEXTURE_CUBE_MAP_SEAMLESS
#define GL_TEXTURE_CUBE_MAP_SEAMLESS 0x884F
#endif
            glEnable(GL_TEXTURE_CUBE_MAP_SEAMLESS);
		}
        renderInfo_.supportFBODepthAttachment = !renderInfo_.graphicsAPIVersionES || renderInfo_.graphicsAPIVersionMajor>2;

        initGlobalUniformBuffer();

        // initialize ImGUI
        imGuiContext = ImGui::CreateContext();
        ImGui_SRE_Init(window);

        // reset render stats
        renderStatsLast = renderStats;
    }

    Renderer::~Renderer() {
        ImGui_SRE_Shutdown();
        ImGui::DestroyContext(imGuiContext);
        glDeleteBuffers(1,&globalUniformBuffer);
        SDL_GL_DeleteContext(glcontext);
        instance = nullptr;
    }

    void Renderer::swapWindow() {
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

    bool Renderer::usesVSync() {
        return vsync;
    }

    int Renderer::getMaxSceneLights() {
        return maxSceneLights;
    }

    void Renderer::initGlobalUniformBuffer(){
        if (renderInfo_.graphicsAPIVersionMajor <= 2){
            globalUniformBuffer = 0;
            return; //
        }
        glGenBuffers(1,&globalUniformBuffer);
        size_t lightSize = sizeof(glm::vec4)*(1 + maxSceneLights*2);
        globalUniformBufferSize = sizeof(glm::mat4)*2+sizeof(glm::vec4)*2 + lightSize;
        glBindBuffer(GL_UNIFORM_BUFFER, globalUniformBuffer);
        glBufferData(GL_UNIFORM_BUFFER, globalUniformBufferSize, NULL, GL_STREAM_DRAW);
        glBindBuffer(GL_UNIFORM_BUFFER, 0);
    }
}
