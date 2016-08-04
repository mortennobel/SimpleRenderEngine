#pragma once

#include <SDL_video.h>
#include "glm/glm.hpp"
#include "Light.h"
#include "Camera.h"

namespace SRE {

// forward declaration
    class Mesh;

    class Shader;

///
/// Maintains shares states for rendering
///
    class SimpleRenderEngine {
    public:
        SimpleRenderEngine(SDL_Window *window);
        ~SimpleRenderEngine();
        static constexpr int maxSceneLights = 4;
        static constexpr int sre_version_major = 0;
        static constexpr int sre_version_minor = 1;
        void setLight(int lightIndex, Light light);

        Light getLight(int lightIndex);

        void render(Mesh *mesh, glm::mat4 modelTransform, Shader *shader);

        void setCamera(Camera *camera);

        Camera* getCamera();

        Camera* getDefaultCamera();

        void clearScreen(glm::vec4 color, bool clearColorBuffer=true, bool clearDepthBuffer=true);

        // Update window with OpenGL rendering
        void swapWindow();

        static SimpleRenderEngine* instance;
    private:
        Light sceneLights[maxSceneLights];
        Camera defaultCamera;
        Camera *camera;
        SDL_Window *window;
        SDL_GLContext glcontext;

    };
}