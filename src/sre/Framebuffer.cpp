//
// Created by Morten Nobel-Jørgensen on 10/07/2017.
//

#include "sre/Framebuffer.hpp"
#include "sre/Texture.hpp"
#include "sre/impl/GL.hpp"

namespace sre{
    Framebuffer::FrameBufferBuilder & Framebuffer::FrameBufferBuilder::withTexture(std::shared_ptr<Texture> texture) {
        this->size = {texture->getWidth(), texture->getHeight()};
        textures.push_back(texture);
        return *this;
    }

    Framebuffer::Framebuffer() {
    }

    Framebuffer::~Framebuffer() {
        if (renderBufferDepth){
            glDeleteRenderbuffers(1, &renderBufferDepth);
        }
        glDeleteFramebuffers(1,&frameBufferObjectId);
    }

    Framebuffer::FrameBufferBuilder Framebuffer::create() {
        return Framebuffer::FrameBufferBuilder();
    }

    int Framebuffer::getMaximumColorAttachments() {
        return 1;
    }

    void checkStatus() {
        using namespace std;
        GLenum frameBufferStatus = glCheckFramebufferStatus(GL_FRAMEBUFFER);
        if (frameBufferStatus != GL_FRAMEBUFFER_COMPLETE) {
            switch (frameBufferStatus) {
                case GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT:
                    cerr << ("GL_FRAMEBUFFER_UNDEFINED");
                    break;
#ifdef GL_ES_VERSION_2_0
                case GL_FRAMEBUFFER_INCOMPLETE_DIMENSIONS:
                    cerr << ("GL_FRAMEBUFFER_INCOMPLETE_DIMENSIONS");
                    break;
#endif
                case GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT:
                    cerr << ("GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT");
                    break;
                case GL_FRAMEBUFFER_UNSUPPORTED:
                    cerr << ("FRAMEBUFFER_UNSUPPORTED");
                    break;
#ifndef GL_ES_VERSION_2_0
                case GL_FRAMEBUFFER_INCOMPLETE_DRAW_BUFFER:
                    cerr << ("GL_FRAMEBUFFER_INCOMPLETE_DRAW_BUFFER");
                    break;
                case GL_FRAMEBUFFER_INCOMPLETE_READ_BUFFER:
                    cerr << ("GL_FRAMEBUFFER_INCOMPLETE_READ_BUFFER");
                    break;
                case GL_FRAMEBUFFER_UNDEFINED:
                    cerr << ("FRAMEBUFFER_INCOMPLETE_ATTACHMENT");
                    break;
                case GL_FRAMEBUFFER_INCOMPLETE_MULTISAMPLE:
                    cerr << ("GL_FRAMEBUFFER_INCOMPLETE_MULTISAMPLE");
                    break;
                case GL_FRAMEBUFFER_INCOMPLETE_LAYER_TARGETS:
                    cerr << ("GL_FRAMEBUFFER_INCOMPLETE_LAYER_TARGETS");
                    break;
#endif
                default:
                    cerr << string("Invalid framebuffer ") + std::to_string(frameBufferStatus);
                    break;
            }
        }
    }

    std::shared_ptr<Framebuffer> Framebuffer::FrameBufferBuilder::build() {
        auto framebuffer = new Framebuffer();
        framebuffer->size = size;
        glGenRenderbuffers(1,&framebuffer->renderBufferDepth); // Create a renderbuffer object

        glBindRenderbuffer(GL_RENDERBUFFER, framebuffer->renderBufferDepth);
        glRenderbufferStorage(GL_RENDERBUFFER,
#ifdef GL_ES_VERSION_2_0
                GL_DEPTH_COMPONENT16
#else
                              GL_DEPTH_COMPONENT32
#endif
                , size.x, size.y);

        glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT,
                              size.x, size.y);
        glBindRenderbuffer(GL_RENDERBUFFER, 0);

        glGenFramebuffers(1, &(framebuffer->frameBufferObjectId));
        glBindFramebuffer(GL_FRAMEBUFFER, framebuffer->frameBufferObjectId);

        for (unsigned i=0;i<textures.size();i++){
            glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0+i, GL_TEXTURE_2D, textures[i]->textureId, 0);
        }

        // attach the renderbuffer to depth attachment point
        glFramebufferRenderbuffer(GL_FRAMEBUFFER,
                                  GL_DEPTH_ATTACHMENT,
                                  GL_RENDERBUFFER,
                                  framebuffer->renderBufferDepth);

        // Check if FBO is configured correctly
        checkStatus();

        framebuffer->textures = textures;

        glBindFramebuffer(GL_FRAMEBUFFER, 0);

        return std::shared_ptr<Framebuffer>(framebuffer);
    }
}