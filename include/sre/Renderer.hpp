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

    /// Maintains shared states for rendering.
    /// An object of Renderer must be created once after the SDL_Window has been initialized.
    /// After initialization this object can be referenced using the static field Renderer::instance;
    ///
    /// Renderer has two important states:
    /// - An active camera, which defines how meshes are drawn when rendered using the draw method
    /// - Light information (point lights, directional lights, ambient lights).
    ///
    /// Example (hello-engine.cpp):
    /// SDL_Window *window;
    /// SDL_Init(SDL_INIT_VIDEO);
    /// SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
    /// SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 1);
    /// SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
    /// window = SDL_CreateWindow("Hello Engine", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, 640, 480, SDL_WINDOW_OPENGL);
    /// Renderer r{window};
    /// r.clearScreen({1,0,0,1});
    /// r.swapWindow();
    /// SDL_Delay(10000);
    /// SDL_DestroyWindow(window);
    /// SDL_Quit();
    class DllExport Renderer {
    public:
        /// SimpleRenderEngine constructor
        /// \param window pointer to the SDL window (must be initialized using OpenGL)
        Renderer(SDL_Window *window);
        ~Renderer();
        static constexpr int maxSceneLights = 4;
        static constexpr int sre_version_major = 0;
        static constexpr int sre_version_minor = 9;
        static constexpr int sre_version_point = 0;


        RenderPass::RenderPassBuilder createRenderPass();

        /**
         * Return the current size of the window
         * @return window
         */
        glm::ivec2 getWindowSize();

        /**
         * Update window with OpenGL rendering by swapping buffers
         */
        void swapWindow();

        /**
         * GPU command buffer (must be called when profiling GPU time - should not be called when not profiling)
         */
        void finishGPUCommandBuffer();

        /** Return stats of the last rendered frame
         *  RenderStats only includes data maintained by sre (imgui calls are not included)
         */
        const RenderStats& getRenderStats();

        /**
         * Singleton reference to the engine after initialization.
         */
        static Renderer* instance;
    private:
        SDL_Window *window;
        SDL_GLContext glcontext;
        friend class Camera;

        RenderStats renderStatsLast;
        RenderStats renderStats;

        friend class Mesh;
        friend class Mesh::MeshBuilder;
        friend class Shader;
        friend class Shader;
        friend class Texture;
        friend class RenderPass;
    };
}