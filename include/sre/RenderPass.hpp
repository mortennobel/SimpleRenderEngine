/*
 *  SimpleRenderEngine (https://github.com/mortennobel/SimpleRenderEngine)
 *
 *  Created by Morten Nobel-Jørgensen ( http://www.nobel-joergnesen.com/ )
 *  License: MIT
 */

#pragma once

#include "sre/Camera.hpp"
#include "sre/Mesh.hpp"
#include "sre/WorldLights.hpp"
#include <string>

#include "sre/impl/Export.hpp"
#include "SpriteBatch.hpp"

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
            RenderStats* renderStats = nullptr;

            bool clearColor = true;
            glm::vec4 clearColorValue = {0,0,0,1};
            bool clearDepth = true;
            float clearDepthValue = 1.0f;
            bool clearStencil = false;
            int clearStencilValue = 0;

            bool gui = true;

            explicit RenderPassBuilder(RenderStats* renderStats);
            friend class RenderPass;
            friend class Renderer;
        };

        static RenderPassBuilder create();   // Create a RenderPass

        RenderPass(const RenderPass&) = delete;
        RenderPass(RenderPass&& rp) noexcept;
        RenderPass& operator=(RenderPass&& other) noexcept;
        virtual ~RenderPass();


        void drawLines(const std::vector<glm::vec3> &verts,             // Draws worldspace lines.
                       glm::vec4 color = {1.0f, 1.0f, 1.0f, 1.0f},      // Note that this member function is not expected
                       MeshTopology meshTopology = MeshTopology::Lines);// to perform as efficient as draw()

        void draw(std::shared_ptr<Mesh>& mesh,                          // Draws a mesh using the given transform and material.
                  glm::mat4 modelTransform,                             // The modelTransform defines the modelToWorld
                  std::shared_ptr<Material>& material);                 // transformation.

        void draw(std::shared_ptr<Mesh>& mesh,                          // Draws a mesh using the given transform and materials.
                  glm::mat4 modelTransform,                             // The modelTransform defines the modelToWorld transformation
                  std::vector<std::shared_ptr<Material>>& materials);   // The number of materials must match the size of index sets in the model

        void draw(std::shared_ptr<SpriteBatch>& spriteBatch,            // Draws a spriteBatch using modelTransform
                  glm::mat4 modelTransform = glm::mat4(1));             // using a model-to-world transformation

        void draw(std::shared_ptr<SpriteBatch>&& spriteBatch,            // Draws a spriteBatch using modelTransform
                  glm::mat4 modelTransform = glm::mat4(1));             // using a model-to-world transformation

        std::vector<glm::vec4> readPixels(unsigned int x,               // Reads pixel(s) from the current framebuffer
                                          unsigned int y,               // The defined rectable must be within the size of the current framebuffer
                                          unsigned int width = 1,
                                          unsigned int height = 1);

        void finishGPUCommandBuffer();                                  // GPU command buffer (must be called when
                                                                        // profiling GPU time - should not be called
                                                                        // when not profiling)
    private:
        static void finish();
        void finishInstance();
        RenderPass::RenderPassBuilder builder;
        explicit RenderPass(RenderPass::RenderPassBuilder& builder);

        void bind(bool newFrame);

        bool containsInstance(RenderPass*);

        void setupShader(const glm::mat4 &modelTransform, Shader *shader);

        Shader* lastBoundShader = nullptr;
        Material* lastBoundMaterial = nullptr;
        int64_t lastBoundMeshId = -1;

        glm::mat4 projection;
        glm::uvec2 viewportOffset;
        glm::uvec2 viewportSize;
        RenderPass* lastInstance = nullptr;
        static RenderPass* instance;
        static std::shared_ptr<Framebuffer> lastFramebuffer;

        friend class Renderer;
    };


}
