//
// Created by Morten Nobel-Jørgensen on 08/06/2017.
//

#pragma once

#include "sre/Camera.hpp"
#include "sre/Mesh.hpp"
#include "sre/WorldLights.hpp"
#include <string>

#include "sre/impl/Export.hpp"

namespace sre {
    class Renderer;

    class Shader;
    class RenderStats;

    // A render pass encapsulates some render states and allows adding draw-calls.
    // Note that only one render pass object can be active at a time.
    class DllExport RenderPass {
    public:
        class DllExport RenderPassBuilder {
        public:
            RenderPassBuilder& withName(const std::string& name);
            RenderPassBuilder& withCamera(const Camera& camera);
            RenderPassBuilder& withWorldLights(WorldLights* worldLights);
            RenderPass build();
        private:
            std::string name;
            WorldLights* worldLights = nullptr;
            Camera camera;
            RenderStats* renderStats;
            RenderPassBuilder(RenderStats* renderStats);
            friend class RenderPass;
            friend class Renderer;
        };

        virtual ~RenderPass();

        /**
         * Clear the screen with the given color (default behavior is also clearing color and depth buffer but not the stencil buffer)
         * @param color
         * @param clearColorBuffer  {=true}
         * @param clearDepthBuffer {=true}
         * @param clearStencil {=false}
         */
        void clearScreen(glm::vec4 color, bool clearColorBuffer = true, bool clearDepthBuffer = true,
                         bool clearStencil = false);

        /**
         * Draws world-space lines.
         * Note that this member function is not expected to perform as efficient as draw()
         * @param verts
         * @param color {1,1,1,1}
         * @param meshTopology {LineStrip}
         */
        void drawLines(const std::vector<glm::vec3> &verts, glm::vec4 color = {1.0f, 1.0f, 1.0f, 1.0f}, MeshTopology meshTopology = MeshTopology::LineStrip);

        /**
         * Draws a mesh instance to the current scene. Uses the current camera object to render the mesh in the scene.
         * @param mesh
         * @param modelTransform
         * @param shader
         */
        void draw(Mesh *mesh, glm::mat4 modelTransform, Shader *shader);
    private:
        RenderPass(Camera&& camera, WorldLights* worldLights, RenderStats* renderStats);

        void setupShader(const glm::mat4 &modelTransform, Shader *shader);

        Camera camera;
        WorldLights* worldLights;
        RenderStats* renderStats;

        static RenderPass * currentRenderPass;

        friend class Renderer;
    };
}
