/*
 *  SimpleRenderEngine
 *
 *  Created by Morten Nobel-Jørgensen ( http://www.nobel-joergnesen.com/ )
 *  License: MIT
 */

#pragma once

#include <vector>
#include <chrono>
#include "sre/RenderStats.hpp"

namespace sre {
    // forward declarations
    class Texture;
    class SpriteAtlas;
    class SDLRenderer;

class Profiler {
public:
    explicit Profiler(int frames = 300,SDLRenderer* sdlRenderer = nullptr);

    void update();                  // must be called each in the beginning of each frame to capture data
    void gui();                     // called when gui should be shown
private:
    int frames;
    int frameCount;
    std::vector<float> milliseconds;
    std::vector<RenderStats> stats;

    std::vector<float> data;

    std::chrono::time_point<std::chrono::high_resolution_clock> lastTick;
    SDLRenderer* sdlRenderer;

    void showTexture(Texture *tex);

    void showSpriteAtlas(SpriteAtlas *pAtlas);
};

}