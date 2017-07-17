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
    class Framebuffer;

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

            RenderPassBuilder& withClearColor(bool enabled = true,glm::vec4 color = {0,0,0,1});    // Set the clear color.
                                                                                                   // Default enabled with the color value {0.0,0.0,0.0,1.0}

            RenderPassBuilder& withClearDepth(bool enabled = true, float value = 1);               // Set the clear depth. Value is clamped between [0.0;1.0]
                                                                                                   // Default: enabled with depth value 1.0

            RenderPassBuilder& withClearStencil(bool enabled = false, int value = 0);              // Set the clear depth. Value is clamped between
                                                                                                   // Default: disabled

            RenderPassBuilder& withGUI(bool enabled = true);                                       // Allows ImGui calls to be called in the renderpass and
                                                                                                   // calls ImGui::Render() in the end of the renderpass

            RenderPassBuilder& withFramebuffer(std::shared_ptr<Framebuffer> framebuffer);
            RenderPass build();
        private:
            RenderPassBuilder() = default;
            // prevent creating instances of this object
            // (note it is still possible to keep a universal reference)
            RenderPassBuilder(const RenderPassBuilder& r) = default;
            std::string name;
            std::shared_ptr<Framebuffer> framebuffer;
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

        static RenderPassBuilder create();   // Create a RenderPass

        virtual ~RenderPass();


        void drawLines(const std::vector<glm::vec3> &verts, glm::vec4 color = {1.0f, 1.0f, 1.0f, 1.0f}, MeshTopology meshTopology = MeshTopology::Lines);
                                                                    // Draws worldspace lines.
                                                                    // Note that this member function is not expected to perform as efficient as draw()

        void draw(std::shared_ptr<Mesh>& mesh, glm::mat4 modelTransform, std::shared_ptr<Material>& material);
                                                                    // Draws a mesh using the given transform and material.
                                                                    // The modelTransform defines the modelToWorld transformation

        void draw(std::shared_ptr<Mesh>& mesh, glm::mat4 modelTransform, std::vector<std::shared_ptr<Material>>& materials);
                                                                    // Draws a mesh using the given transform and materials.
                                                                    // The modelTransform defines the modelToWorld transformation
                                                                    // The number of materials must match the size of index sets in the model

        std::vector<glm::vec4> readPixels(unsigned int x, unsigned int y, unsigned int width = 1, unsigned int height = 1);
                                                                    // Reads pixel(s) from the current framebuffer
                                                                    // The defined rectable must be within the size of the current framebuffer

        void finishGPUCommandBuffer();                      // GPU command buffer (must be called when profiling GPU time - should not be called when not profiling)
    private:
        static void finish();

        RenderPass(RenderPass::RenderPassBuilder& builder);

        void setupShader(const glm::mat4 &modelTransform, Shader *shader);

        Shader* lastBoundShader = nullptr;
        Material* lastBoundMaterial = nullptr;
        Mesh* lastBoundMesh = nullptr;

        Camera camera;
        WorldLights* worldLights;
        RenderStats* renderStats;
        std::shared_ptr<Framebuffer> framebuffer;
        glm::mat4 projection;
        glm::uvec2 viewportOffset;
        glm::uvec2 viewportSize;


        static bool instance;
        static bool lastGui;
        static std::shared_ptr<Framebuffer> lastFramebuffer;

        friend class Renderer;
    };
}
