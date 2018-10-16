/*
 *  SimpleRenderEngine (https://github.com/mortennobel/SimpleRenderEngine)
 *
 *  Created by Morten Nobel-Jørgensen ( http://www.nobel-joergensen.com/ )
 *  License: MIT
 */

#pragma once
#include <memory>
#include <vector>
#include <string>
#include "glm/glm.hpp"
#include "Texture.hpp"
#include "impl/CPPShim.hpp"


namespace sre {
    class RenderPass;
    class Texture;

    /**
     * A framebuffer object allows rendering into textures instead of the screen.
     *
     * A framebuffer is created with a destination texture. It is important that this texture is not used in
     * materials when rendering to the framebuffer (reading and writing to a texture at the same time is not supported).
     *
     * To use a framebuffer, pass it to RenderPassBuilder when a renderpass is being created.
     */
    class Framebuffer {
    public:
        class FrameBufferBuilder {
        public:
            FrameBufferBuilder& withColorTexture(std::shared_ptr<Texture> texture);
            FrameBufferBuilder& withDepthTexture(std::shared_ptr<Texture> texture);
            FrameBufferBuilder& withName(std::string name);
            std::shared_ptr<Framebuffer> build();
        private:
            std::vector<std::shared_ptr<Texture>> textures;
            std::shared_ptr<Texture> depthTexture;
            glm::uvec2 size;
            std::string name;
            FrameBufferBuilder() = default;
            FrameBufferBuilder(const FrameBufferBuilder&) = default;
            friend class Framebuffer;
        };

        ~Framebuffer();

        static FrameBufferBuilder create();

        static int getMaximumDepthAttachments();
        static int getMaximumColorAttachments();

        void setColorTexture(std::shared_ptr<Texture> tex, int index = 0);
        void setDepthTexture(std::shared_ptr<Texture> tex);

        const std::string& getName();
    private:
        void bind();
        bool dirty = true;
        explicit Framebuffer(std::string name);
        std::vector<std::shared_ptr<Texture>> textures;
        std::shared_ptr<Texture> depthTexture;
        unsigned int frameBufferObjectId;
        uint32_t renderbuffer = 0;
        std::string name;
        glm::uvec2 size;
        friend class RenderPass;
        friend class Inspector;
		friend class VROculus;
    };
}


