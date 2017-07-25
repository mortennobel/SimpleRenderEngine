//
// Created by Morten Nobel-Jørgensen on 10/07/2017.
//

#pragma once
#include <memory>
#include <vector>
#include <string>
#include "glm/glm.hpp"


namespace sre {
    class RenderPass;
    class Texture;

    class Framebuffer {
    public:
        class FrameBufferBuilder {
        public:
            FrameBufferBuilder& withTexture(std::shared_ptr<Texture> texture);
            FrameBufferBuilder& withName(std::string name);
            std::shared_ptr<Framebuffer> build();
        private:
            std::vector<std::shared_ptr<Texture>> textures;
            glm::uvec2 size;
            std::string name;
            FrameBufferBuilder() = default;
            FrameBufferBuilder(const FrameBufferBuilder&) = default;
            friend class Framebuffer;
        };

        ~Framebuffer();

        static FrameBufferBuilder create();

        static int getMaximumColorAttachments();

        const std::string& getName();

    private:
        Framebuffer(std::string name);
        std::vector<std::shared_ptr<Texture>> textures;
        unsigned int frameBufferObjectId;
        uint32_t renderBufferDepth = 0;
        std::string name;
        glm::uvec2 size;
        friend class RenderPass;
    };
}


