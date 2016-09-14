#pragma once

#include <SDL_video.h>
#include "glm/glm.hpp"
#include "SRE/Light.hpp"
#include "SRE/Camera.hpp"

#include "SRE/Export.hpp"



namespace SRE {

// forward declaration
    class Mesh;

    class Shader;

    ///
    /// Maintains shares states for rendering
    ///
    class DllExport SimpleRenderEngine {
    public:
        SimpleRenderEngine(SDL_Window *window);
        ~SimpleRenderEngine();
        static constexpr int maxSceneLights = 4;
        static constexpr int sre_version_major = 0;
        static constexpr int sre_version_minor = 1;

        void setLight(int lightIndex, Light light);

        Light getLight(int lightIndex);

        void setAmbientLight(const glm::vec3 &ambientLight);

        glm::vec3 getAmbientLight() const;

        void draw(Mesh *mesh, glm::mat4 modelTransform, Shader *shader);

        void setCamera(Camera *camera);

        Camera* getCamera();

        Camera* getDefaultCamera();

        void clearScreen(glm::vec4 color, bool clearColorBuffer=true, bool clearDepthBuffer=true);

        // Update window with OpenGL rendering
        void swapWindow();

        static SimpleRenderEngine* instance;
    private:
        glm::vec4 ambientLight = glm::vec4(0.2,0.2,0.2,0.2);
        Light sceneLights[maxSceneLights];
        Camera defaultCamera;
        Camera *camera;
        SDL_Window *window;
        SDL_GLContext glcontext;
        friend class Camera;
    };
}