/*
 *  SimpleRenderEngine (https://github.com/mortennobel/SimpleRenderEngine)
 *
 *  Created by Morten Nobel-Jørgensen ( http://www.nobel-joergensen.com/ )
 *  License: MIT
 */

#include "sre/Framebuffer.hpp"
#include "sre/Texture.hpp"
#include "sre/impl/GL.hpp"

#include "sre/Log.hpp"
#include <algorithm>
#include <sre/Renderer.hpp>

namespace sre{
    Framebuffer::FrameBufferBuilder& Framebuffer::FrameBufferBuilder::withColorTexture(std::shared_ptr<Texture> texture) {
        assert(!texture->isDepthTexture());
        auto s = glm::uvec2{texture->getWidth(), texture->getHeight()};
        if (!textures.empty() || depthTexture.get()){
            assert(this->size == s);
        } else {
            this->size = s;
        }
        textures.push_back(texture);
        return *this;
    }

    Framebuffer::FrameBufferBuilder& Framebuffer::FrameBufferBuilder::withDepthTexture(std::shared_ptr<Texture> texture) {
        assert(texture->isDepthTexture());
        auto s = glm::uvec2{texture->getWidth(), texture->getHeight()};
        if (!textures.empty() || depthTexture.get()){
            assert(this->size == s);
        } else {
            this->size = s;
        }
        depthTexture = texture;
        return *this;
    }

    Framebuffer::FrameBufferBuilder &Framebuffer::FrameBufferBuilder::withName(std::string name) {
        this->name = std::move(name);
        return *this;
    }

    Framebuffer::Framebuffer(std::string name)
    :name(std::move(name))
    {
        auto r = Renderer::instance;
        r->framebufferObjects.push_back(this);
    }

    Framebuffer::~Framebuffer() {
        auto r = Renderer::instance;
        if (r){
            r->framebufferObjects.erase(std::remove(r->framebufferObjects.begin(), r->framebufferObjects.end(), this));
            if (renderbuffer != 0){
                glDeleteRenderbuffers(1, &renderbuffer);
            }
            glDeleteFramebuffers(1,&frameBufferObjectId);
        }
    }

    Framebuffer::FrameBufferBuilder Framebuffer::create() {
        return Framebuffer::FrameBufferBuilder();
    }

    int Framebuffer::getMaximumColorAttachments() {
        if (renderInfo().graphicsAPIVersionES && renderInfo().graphicsAPIVersionMajor<= 2){
            return 1;
        }
        static int maxColorBuffers;
        static bool once = [&](){
            static GLint maxAttach = 0;
            static GLint maxDrawBuf = 0;
            glGetIntegerv(GL_MAX_COLOR_ATTACHMENTS, &maxAttach);
            glGetIntegerv(GL_MAX_DRAW_BUFFERS, &maxDrawBuf);
            maxColorBuffers = std::min(maxAttach, maxDrawBuf);
            return true;
        } ();

        return maxColorBuffers;
    }

    int Framebuffer::getMaximumDepthAttachments() {
        if (renderInfo().graphicsAPIVersionES && renderInfo().graphicsAPIVersionMajor <= 2){
            return 0;
        } else {
            return 1;
        }
    }

    const std::string& Framebuffer::getName() {
        return name;
    }

    void Framebuffer::setColorTexture(std::shared_ptr<Texture> tex, int index) {
        assert(textures.size() > index && index >= 0);
        assert(!tex->isDepthTexture());
        textures[index] = tex;
        dirty = true;
    }

    void Framebuffer::setDepthTexture(std::shared_ptr<Texture> tex) {
        assert(tex->isDepthTexture());
        depthTexture = tex;
        dirty = true;
    }


    void Framebuffer::bind() {
        glBindFramebuffer(GL_FRAMEBUFFER, frameBufferObjectId);
        if (dirty){
            for (int i=0;i<textures.size();i++){
                glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0+i, GL_TEXTURE_2D, textures[i]->textureId, 0);
            }
            if (depthTexture){
                glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, depthTexture->textureId, 0);
            }
            dirty = false;
        }
    }

    void checkStatus() {
        using namespace std;
        GLenum frameBufferStatus = glCheckFramebufferStatus(GL_FRAMEBUFFER);
        if (frameBufferStatus != GL_FRAMEBUFFER_COMPLETE) {
            char array[50];
            const char* errorMsg = nullptr;
            switch (frameBufferStatus) {
                case GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT:
                    errorMsg = "GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT";
                    break;
#ifdef GL_ES_VERSION_2_0
                case GL_FRAMEBUFFER_INCOMPLETE_DIMENSIONS:
                    errorMsg = "GL_FRAMEBUFFER_INCOMPLETE_DIMENSIONS";
                    break;
#endif
                case GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT:
                    errorMsg = "GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT";
                    break;
                case GL_FRAMEBUFFER_UNSUPPORTED:
                    errorMsg = "FRAMEBUFFER_UNSUPPORTED";
                    break;
#ifndef GL_ES_VERSION_2_0
                case GL_FRAMEBUFFER_INCOMPLETE_DRAW_BUFFER:
                    errorMsg = "GL_FRAMEBUFFER_INCOMPLETE_DRAW_BUFFER";
                    break;
                case GL_FRAMEBUFFER_INCOMPLETE_READ_BUFFER:
                    errorMsg = "GL_FRAMEBUFFER_INCOMPLETE_READ_BUFFER";
                    break;
                case GL_FRAMEBUFFER_UNDEFINED:
                    errorMsg = "GL_FRAMEBUFFER_UNDEFINED";
                    break;
                case GL_FRAMEBUFFER_INCOMPLETE_MULTISAMPLE:
                    errorMsg = "GL_FRAMEBUFFER_INCOMPLETE_MULTISAMPLE";
                    break;
                case GL_FRAMEBUFFER_INCOMPLETE_LAYER_TARGETS:
                    errorMsg = "GL_FRAMEBUFFER_INCOMPLETE_LAYER_TARGETS";
                    break;
#endif
                default:
                    snprintf(array,sizeof(array), "Unknown error code: %i",frameBufferStatus);
                    errorMsg = array;
                    break;
            }
            LOG_ERROR("Invalid framebuffer: %s", errorMsg);
        }
    }

    std::shared_ptr<Framebuffer> Framebuffer::FrameBufferBuilder::build() {
        if (name.length()==0){
            name = "Framebuffer";
        }
        auto framebuffer = new Framebuffer(name);
        framebuffer->size = size;

        glGenFramebuffers(1, &(framebuffer->frameBufferObjectId));
        glBindFramebuffer(GL_FRAMEBUFFER, framebuffer->frameBufferObjectId);

        std::vector<GLenum> drawBuffers;
        for (unsigned i=0;i<textures.size();i++){
            glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0+i, GL_TEXTURE_2D, textures[i]->textureId, 0);
            drawBuffers.push_back(GL_COLOR_ATTACHMENT0+i);
        }

        if (textures.empty()){
            glGenRenderbuffers(1,&framebuffer->renderbuffer); // Create a renderbuffer object
            glBindRenderbuffer(GL_RENDERBUFFER, framebuffer->renderbuffer);
            glRenderbufferStorage(GL_RENDERBUFFER, GL_RGBA8, size.x, size.y);

            glBindRenderbuffer(GL_RENDERBUFFER, 0);
            // attach the renderbuffer to depth attachment point
            glFramebufferRenderbuffer(GL_FRAMEBUFFER,
                                      GL_COLOR_ATTACHMENT0,
                                      GL_RENDERBUFFER,
                                      framebuffer->renderbuffer);
        }
        
        if (depthTexture){
            glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, depthTexture->textureId, 0);
        } else {
            glGenRenderbuffers(1,&framebuffer->renderbuffer); // Create a renderbuffer object
            glBindRenderbuffer(GL_RENDERBUFFER, framebuffer->renderbuffer);

            if (renderInfo().graphicsAPIVersionES){
                glRenderbufferStorage(GL_RENDERBUFFER, renderInfo().graphicsAPIVersionMajor<=2?GL_DEPTH_COMPONENT16:GL_DEPTH_COMPONENT24,
                                  size.x, size.y);
            } else {
                glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT,
                                  size.x, size.y);
            }
            glBindRenderbuffer(GL_RENDERBUFFER, 0);
            // attach the renderbuffer to depth attachment point
            glFramebufferRenderbuffer(GL_FRAMEBUFFER,
                                      GL_DEPTH_ATTACHMENT,
                                      GL_RENDERBUFFER,
                                      framebuffer->renderbuffer);
        }
        if (!renderInfo().graphicsAPIVersionES || renderInfo().graphicsAPIVersionMajor>=3){
            glDrawBuffers((GLsizei)drawBuffers.size(), drawBuffers.data());
            if (drawBuffers.empty()){
                glReadBuffer(GL_NONE);
            } else {
                glReadBuffer(GL_COLOR_ATTACHMENT0);
            }
        }
        // Check if FBO is configured correctly
        checkStatus();
        framebuffer->textures = textures;
        framebuffer->depthTexture = depthTexture;
        glBindFramebuffer(GL_FRAMEBUFFER, 0);

        return std::shared_ptr<Framebuffer>(framebuffer);
    }
}
