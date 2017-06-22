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
    class Material;
    class RenderStats;

    // A render pass encapsulates some render states and allows adding draw-calls.
    // Materials and shaders are assumed not to be modified during a renderpass.
    // Note that only one render pass object can be active at a time.
    class DllExport RenderPass {
    public:
        class DllExport RenderPassBuilder {
        public:
            RenderPassBuilder& withName(const std::string& name);
            RenderPassBuilder& withCamera(const Camera& camera);
            RenderPassBuilder& withWorldLights(WorldLights* worldLights);
            // Set the clear color.
            // Default enabled with the color value {0.0,0.0,0.0,1.0}
            RenderPassBuilder& withClearColor(bool enabled = true,glm::vec4 color = {0,0,0,1});
            // Set the clear depth. Value is clamped between [0.0;1.0]
            // Default: enabled with depth value 1.0
            RenderPassBuilder& withClearDepth(bool enabled = true, float value = 1);
            // Set the clear depth. Value is clamped between
            // Default: disabled
            RenderPassBuilder& withClearStencil(bool enabled = false, int value = 0);
            // Allows ImGui calls to be called in the renderpass and
            // calls ImGui::Render() in the end of the renderpass
            RenderPassBuilder& withGUI(bool enabled = true);
            RenderPass build();
        private:
            std::string name;
            WorldLights* worldLights = nullptr;
            Camera camera;
            RenderStats* renderStats;

            bool clearColor = true;
            glm::vec4 clearColorValue = {0,0,0,1};
            bool clearDepth = true;
            float clearDepthValue = 1.0f;
            bool clearStencil = false;
            int clearStencilValue = 0;

            bool gui = true;

            RenderPassBuilder(RenderStats* renderStats);
            friend class RenderPass;
            friend class Renderer;
        };

        virtual ~RenderPass();

        /**
         * Draws world-space lines.
         * Note that this member function is not expected to perform as efficient as draw()
         * @param verts
         * @param color {1,1,1,1}
         * @param meshTopology {LineStrip}
         */
        void drawLines(const std::vector<glm::vec3> &verts, glm::vec4 color = {1.0f, 1.0f, 1.0f, 1.0f}, MeshTopology meshTopology = MeshTopology::LineStrip);

        /**
         * Draws a mesh using the given transform and material.
         * @param mesh
         * @param modelTransform
         * @param material
         */
        void draw(Mesh *mesh, glm::mat4 modelTransform, Material *material);


    private:
        void finish();

        RenderPass(Camera &&camera, WorldLights *worldLights, RenderStats *renderStats, bool gui);

        void setupShader(const glm::mat4 &modelTransform, Shader *shader);

        Shader* lastBoundShader = nullptr;
        Material* lastBoundMaterial = nullptr;
        Mesh* lastBoundMesh = nullptr;
        bool gui;

        Camera camera;
        WorldLights* worldLights;
        RenderStats* renderStats;

        static RenderPass * instance;

        friend class Renderer;
    };
}
