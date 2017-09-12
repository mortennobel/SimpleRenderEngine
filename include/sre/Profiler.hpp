/*
 *  SimpleRenderEngine (https://github.com/mortennobel/SimpleRenderEngine)
 *
 *  Created by Morten Nobel-Jørgensen ( http://www.nobel-joergnesen.com/ )
 *  License: MIT
 */

#pragma once

#include <vector>
#include <chrono>
#include "sre/RenderStats.hpp"
#include "sre/Framebuffer.hpp"
#include "sre/WorldLights.hpp"

namespace sre {
    // forward declarations
    class Texture;

    class Mesh;

    class Shader;

    class SpriteAtlas;

    class SDLRenderer;

    /**
     * The profiler measures resources used by SimpleRenderEngine.
     * Profiler.update() records the current state and must be called each frame.
     * Profiler.gui() draws gui. Must be called within a RenderPass with GUI enabled.
     */
    class Profiler {
    public:
        explicit Profiler(int frames = 300, SDLRenderer *sdlRenderer = nullptr);

        void update();                  // must be called each in the beginning of each frame to capture data
        void gui(bool useWindow = true);// called when gui should be shown

    private:
        std::shared_ptr<Texture> getTmpTexture();

        std::vector<std::shared_ptr<Texture>> offscreenTextures;
        int usedTextures = 0;
        std::shared_ptr<Framebuffer> framebuffer;
        int frames;
        int frameCount;
        std::vector<float> milliseconds;
        std::vector<RenderStats> stats;

        std::vector<float> data;

        float time;

        std::chrono::time_point<std::chrono::high_resolution_clock> lastTick;
        SDLRenderer *sdlRenderer;

        void showMesh(Mesh *mesh);

        void showShader(Shader *shader);

        void showTexture(Texture *tex);

        void showSpriteAtlas(SpriteAtlas *pAtlas);

        WorldLights worldLights;

        void initFramebuffer();

        const float previewSize = 100;
    };

}