/*
 *  SimpleRenderEngine (https://github.com/mortennobel/SimpleRenderEngine)
 *
 *  Created by Morten Nobel-Jørgensen ( http://www.nobel-joergensen.com/ )
 *  License: MIT
 */

#pragma once

#include <SDL_video.h>
#include "glm/glm.hpp"
#include "sre/Light.hpp"
#include "sre/Camera.hpp"
#include "sre/RenderPass.hpp"

#include "sre/impl/Export.hpp"
#include "RenderStats.hpp"
#include "Mesh.hpp"



namespace sre {

    // forward declaration
    class Mesh;
    class ParticleMesh;

    class Shader;
    class Shader;
	class VR;

    struct RenderInfo{
        bool useFramebufferSRGB = false;
        bool supportTextureSamplerSRGB = false;
        int graphicsAPIVersionMajor;            // For WebGL uses OpenGL ES api version (WebGL 1.0 = OpenGL ES 2.0)
        int graphicsAPIVersionMinor;
        bool graphicsAPIVersionES;
        std::string graphicsAPIVersion;
        std::string graphicsAPIVendor;
    };

    const RenderInfo& renderInfo();

    /// Maintains shared states for rendering.
    /// An object of Renderer must be created once after the SDL_Window has been initialized.
    /// After initialization this object can be referenced using the static field Renderer::instance;
    ///
    /// Renderer has two important states:
    /// - An active camera, which defines how meshes are drawn when rendered using the draw method
    /// - Light information (point lights, directional lights, ambient lights).
    ///
    /// Example (hello-engine-raw.cpp):
    /// SDL_Window *window;
    /// SDL_Init(SDL_INIT_VIDEO);
    /// SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
    /// SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 1);
    /// SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
    /// window = SDL_CreateWindow("Hello Engine",SDL_WINDOWPOS_UNDEFINED,SDL_WINDOWPOS_UNDEFINED,640,480,SDL_WINDOW_OPENGL);
    /// Renderer r{window};
    /// RenderPass rp = RenderPass::create().build();
    /// rp.drawLines({{0,0,0},{1,1,1}});
    /// r.swapWindow();
    /// SDL_Delay(10000);
    /// SDL_DestroyWindow(window);
    /// SDL_Quit();
    ///
    class DllExport Renderer {
    public:
        explicit Renderer(SDL_Window *window, bool vsync = true, int maxSceneLights = 4);    // SimpleRenderEngine constructor
                                                            // param window pointer to the SDL window (must be initialized using OpenGL)
        ~Renderer();
        static constexpr int sre_version_major = 1;
        static constexpr int sre_version_minor = 0;
        static constexpr int sre_version_point = 9;

        glm::ivec2 getWindowSize();                         // Return the current size of the window

        glm::ivec2 getDrawableSize();                       // Get the size of a window's underlying drawable in pixels (for use with glViewport). May be larger than window size in case of HighDPI.

        bool usesVSync();                                   // Return true if vsync is enabled

        void swapWindow();                                  // Update window with OpenGL rendering by swapping buffers
                                                            // Make sure any current are RenderPass objects are finished (if any)

        const RenderStats& getRenderStats();                // Return stats of the last rendered frame
                                                            // RenderStats only includes data maintained by sre (imgui calls are not included)

        static Renderer* instance;                          // Singleton reference to the engine after initialization.

        int getMaxSceneLights();                            // Get maximum amout of scenelights per object

        DEPRECATED("Use sre::renderInfo() instead of Renderer::getRenderInfo()")
        const RenderInfo& getRenderInfo();                  // Get info about the renderer
    private:
        int maxSceneLights = 4;                             // Maximum of scene lights
        SDL_Window *window;
        SDL_GLContext glcontext;
        bool vsync;
        friend class Camera;

        RenderStats renderStatsLast;
        RenderStats renderStats;

        std::vector<Framebuffer*> framebufferObjects;
        std::vector<Mesh*> meshes;
        std::vector<Shader*> shaders;
        std::vector<Texture*> textures;
        std::vector<SpriteAtlas*> spriteAtlases;

        void initGlobalUniformBuffer();
        GLuint globalUniformBuffer = 0;
        GLuint globalUniformBufferSize = 0;

        VR* vr = nullptr;

        friend class Mesh;
        friend class Mesh::MeshBuilder;
        friend class Shader;
        friend class Shader;
        friend class Material;
        friend class Texture;
        friend class Framebuffer;
        friend class RenderPass;
        friend class Inspector;
        friend class SpriteAtlas;
		friend class VR;
        friend class RenderPass::RenderPassBuilder;
    };
}