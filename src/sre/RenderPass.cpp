/*
 *  SimpleRenderEngine (https://github.com/mortennobel/SimpleRenderEngine)
 *
 *  Created by Morten Nobel-Jørgensen ( http://www.nobel-joergnesen.com/ )
 *  License: MIT
 */

#include "sre/RenderPass.hpp"
#include "sre/Mesh.hpp"
#include "sre/Shader.hpp"
#include "sre/Material.hpp"
#include "sre/RenderStats.hpp"
#include "sre/Texture.hpp"
#include "sre/impl/GL.hpp"
#include <cassert>
#include <algorithm>
#include <glm/gtc/type_ptr.hpp>
#include <sre/imgui_sre.hpp>
#include <sre/Renderer.hpp>
#include <imgui_internal.h>
#include <glm/gtc/type_precision.hpp>
#include <glm/gtc/color_space.hpp>
#include <glm/gtx/transform.hpp>


namespace sre {

    RenderPass::RenderPassBuilder RenderPass::create() {
        return RenderPass::RenderPassBuilder(&Renderer::instance->renderStats);
    }

    RenderPass::RenderPassBuilder::RenderPassBuilder(RenderStats* renderStats)
    :renderStats(renderStats)
    {
    }

    RenderPass::RenderPassBuilder& RenderPass::RenderPassBuilder::withName(const std::string& name){
        this->name = name;
        return *this;
    }

    RenderPass::RenderPassBuilder& RenderPass::RenderPassBuilder::withCamera(const Camera& camera){
        this->camera = camera;
        return *this;
    }

    RenderPass::RenderPassBuilder& RenderPass::RenderPassBuilder::withWorldLights(WorldLights* worldLights){
        this->worldLights = worldLights;
        return *this;
    }

    RenderPass::RenderPassBuilder &RenderPass::RenderPassBuilder::withClearColor(bool enabled, Color color) {
        if (renderInfo().useFramebufferSRGB){
            auto col3 = glm::convertSRGBToLinear(glm::vec3(color.r, color.g, color.b));
            this->clearColorValue = glm::vec4(col3, color.a);
        } else {
            this->clearColorValue = glm::vec4(color.r, color.g, color.b, color.a);
        }
        this->clearColor = enabled;

        return *this;
    }

    RenderPass::RenderPassBuilder &RenderPass::RenderPassBuilder::withClearDepth(bool enabled, float value) {
        this->clearDepth = enabled;
        this->clearDepthValue = value;
        return *this;
    }

    RenderPass::RenderPassBuilder &RenderPass::RenderPassBuilder::withClearStencil(bool enabled, int value) {
        this->clearStencil = enabled;
        this->clearStencilValue = value;
        return *this;
    }

    RenderPass::RenderPassBuilder &RenderPass::RenderPassBuilder::withGUI(bool enabled) {
        this->gui = enabled;
        return *this;
    }

    RenderPass RenderPass::RenderPassBuilder::build(){

        return RenderPass(*this);
    }

    RenderPass::RenderPassBuilder & RenderPass::RenderPassBuilder::withFramebuffer(std::shared_ptr<Framebuffer> framebuffer) {
        this->framebuffer = std::move(framebuffer);
        return *this;
    }

    RenderPass::RenderPass(RenderPass::RenderPassBuilder& builder)
        :builder(builder)
    {
        if (builder.gui) {
            ImGui_SRE_NewFrame(Renderer::instance->window);
        }
    }

    RenderPass::RenderPass(RenderPass &&rp) noexcept {
        builder = rp.builder;
        std::swap(mIsFinished,rp.mIsFinished);
        std::swap(lastBoundShader,rp.lastBoundShader);
        std::swap(lastBoundMaterial,rp.lastBoundMaterial);
        std::swap(lastBoundMeshId,rp.lastBoundMeshId);
        std::swap(projection,rp.projection);
        std::swap(viewportOffset,rp.viewportOffset);
        std::swap(viewportSize,rp.viewportSize);
    }

    RenderPass &RenderPass::operator=(RenderPass &&rp) noexcept {
        finish();
        builder = rp.builder;
        std::swap(mIsFinished,rp.mIsFinished);
        std::swap(lastBoundShader,rp.lastBoundShader);
        std::swap(lastBoundMaterial,rp.lastBoundMaterial);
        std::swap(lastBoundMeshId,rp.lastBoundMeshId);
        std::swap(projection,rp.projection);
        std::swap(viewportOffset,rp.viewportOffset);
        std::swap(viewportSize,rp.viewportSize);
        return *this;
    }

    RenderPass::~RenderPass(){
        finish();
    }

    void RenderPass::draw(std::shared_ptr<Mesh>& meshPtr, glm::mat4 modelTransform, std::shared_ptr<Material>& material_ptr) {
        assert(!mIsFinished && "RenderPass is finished. Can no longer be modified.");
        renderQueue.emplace_back(RenderQueueObj{meshPtr, modelTransform, {material_ptr}});
    }

    void RenderPass::setupShader(const glm::mat4 &modelTransform, Shader *shader)  {
        if (lastBoundShader == shader){
			if (shader->uniformLocationModel != -1){
                glUniformMatrix4fv(shader->uniformLocationModel, 1, GL_FALSE, glm::value_ptr(modelTransform));
            }
            if (shader->uniformLocationModelViewInverseTranspose != -1){
                auto normalMatrix = transpose(inverse(((glm::mat3)builder.camera.getViewTransform()) * ((glm::mat3)modelTransform)));
				glUniformMatrix3fv(shader->uniformLocationModelViewInverseTranspose, 1, GL_FALSE, glm::value_ptr(normalMatrix));
            }
            if (shader->uniformLocationModelInverseTranspose != -1){
                auto normalMatrix = transpose(inverse((glm::mat3)modelTransform));
				glUniformMatrix3fv(shader->uniformLocationModelInverseTranspose, 1, GL_FALSE, glm::value_ptr(normalMatrix));
            }
        } else {
            builder.renderStats->stateChangesShader++;
            lastBoundShader = shader;
            shader->bind();
            if (shader->uniformLocationModel != -1) {
                glUniformMatrix4fv(shader->uniformLocationModel, 1, GL_FALSE, glm::value_ptr(modelTransform));
            }
            if (shader->uniformLocationView != -1) {
                glUniformMatrix4fv(shader->uniformLocationView, 1, GL_FALSE, glm::value_ptr(builder.camera.viewTransform));
            }
            if (shader->uniformLocationProjection != -1) {
                glUniformMatrix4fv(shader->uniformLocationProjection, 1, GL_FALSE, glm::value_ptr(projection));
            }
            if (shader->uniformLocationModelViewInverseTranspose != -1) {
                auto normalMatrix = transpose(inverse(((glm::mat3)builder.camera.getViewTransform()) * ((glm::mat3)modelTransform)));
                glUniformMatrix3fv(shader->uniformLocationModelViewInverseTranspose, 1, GL_FALSE, glm::value_ptr(normalMatrix));
            }
            if (shader->uniformLocationModelInverseTranspose != -1) {
                auto normalMatrix = transpose(inverse((glm::mat3)modelTransform));
                glUniformMatrix3fv(shader->uniformLocationModelInverseTranspose, 1, GL_FALSE, glm::value_ptr(normalMatrix));
            }
            if (shader->uniformLocationViewport != -1) {
                glm::vec4 viewport((float)viewportSize.x,(float)viewportSize.y,(float)viewportOffset.x,(float)viewportOffset.y);
                glUniform4fv(shader->uniformLocationViewport, 1, glm::value_ptr(viewport));
            }
            if (shader->uniformLocationCameraPosition != -1) {
                glm::vec4 cameraPos = glm::vec4(this->builder.camera.getPosition(),1.0f);
                glUniform4fv(shader->uniformLocationCameraPosition, 1, glm::value_ptr(cameraPos));
            }
            shader->setLights(builder.worldLights);
        }
    }

    void RenderPass::drawLines(const std::vector<glm::vec3> &verts, Color color, MeshTopology meshTopology) {
        assert(!mIsFinished && "RenderPass is finished. Can no longer be modified.");

        // Keep a shared mesh and material
        auto material = Shader::getUnlit()->createMaterial();
        auto mesh = Mesh::create()
                .withPositions(verts)
                .withMeshTopology(meshTopology)
                .build();

        // update material
        material->setColor(color);

        renderQueue.emplace_back(RenderQueueObj{
                                         mesh,
                                         glm::mat4(1),
                                         {material}
                                 });
    }

    void RenderPass::finish(){
        if (mIsFinished){
            return;
        }
        if (builder.framebuffer!=nullptr){
            builder.framebuffer->bind();
        } else {
            glBindFramebuffer(GL_FRAMEBUFFER, 0);
        }

        glm::vec2 windowSize;
        if (builder.framebuffer){
            windowSize = builder.framebuffer->size;
        } else {
            windowSize = static_cast<glm::vec2>(Renderer::instance->getDrawableSize());
        }
        viewportOffset = static_cast<glm::uvec2>(builder.camera.viewportOffset * windowSize);
        viewportSize = static_cast<glm::uvec2>(windowSize * builder.camera.viewportSize);
        glEnable(GL_SCISSOR_TEST);
        glScissor(viewportOffset.x, viewportOffset.y, viewportSize.x,viewportSize.y);
        glViewport(viewportOffset.x, viewportOffset.y, viewportSize.x,viewportSize.y);

        GLbitfield clear = 0;
        if (builder.clearColor) {
            glClearColor(builder.clearColorValue.r, builder.clearColorValue.g, builder.clearColorValue.b, builder.clearColorValue.a);
            clear |= GL_COLOR_BUFFER_BIT;
        }
        if (builder.clearDepth) {
            glClearDepthf(builder.clearDepthValue);
            glDepthMask(GL_TRUE);
            clear |= GL_DEPTH_BUFFER_BIT;
        }
        if (builder.clearStencil) {
            glClearStencil(builder.clearStencilValue);
            clear |= GL_STENCIL_BUFFER_BIT;
        }
        if (clear != 0u) {
            glClear(clear);
        }



        projection = builder.camera.getProjectionTransform(viewportSize);

        for (auto & rqObj : renderQueue){
            drawInstance(rqObj);
        }

        if (builder.gui) {
            ImGui::Render();
        }
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        if (builder.framebuffer != nullptr){
            for(auto& tex : builder.framebuffer->textures){
                if (tex->generateMipmap){
                    glBindTexture(tex->target,tex->textureId);
                    glGenerateMipmap(tex->target);
                    glBindTexture(tex->target,0);
                }
            }
        }
        mIsFinished = true;
#ifndef NDEBUG
        checkGLError();
#endif
    }

    std::vector<Color> RenderPass::readPixels(unsigned int x, unsigned int y, unsigned int width, unsigned int height) {
        assert(mIsFinished);
        if (builder.framebuffer!=nullptr){
            builder.framebuffer->bind();
        }
        std::vector<Color> res(width * height);
        std::vector<glm::u8vec4> resUnsigned(width * height);

        glReadPixels(x,y,width, height, GL_RGBA,GL_UNSIGNED_BYTE,resUnsigned.data());
        for (int i=0;i<resUnsigned.size();i++){
            for (int j=0;j<4;j++){
                res[i][j] = resUnsigned[i][j]/255.0f;
            }
        }
        // set default framebuffer
        if (builder.framebuffer!=nullptr) {
            glBindFramebuffer(GL_FRAMEBUFFER, 0);
        }

        return res;
    }

    void RenderPass::draw(std::shared_ptr<Mesh> &meshPtr, glm::mat4 modelTransform,
                          std::vector<std::shared_ptr<Material>> materials) {
        assert(!mIsFinished && "RenderPass is finished. Can no longer be modified.");
        assert(meshPtr->indices.size() == 0 || meshPtr->indices.size() == materials.size());
        renderQueue.emplace_back(RenderQueueObj{meshPtr, modelTransform, materials});
    }

    void RenderPass::drawInstance(RenderQueueObj& rqObj) {
        for (int i=0;i<rqObj.materials.size();i++){
            auto material_ptr = rqObj.materials[i];
            Mesh* mesh = rqObj.mesh.get();
            auto material = material_ptr.get();
            auto shader = material->getShader().get();
            assert(mesh  != nullptr);
            builder.renderStats->drawCalls++;
            setupShader(rqObj.modelTransform, shader);
            if (material != lastBoundMaterial) {
                builder.renderStats->stateChangesMaterial++;
                lastBoundMaterial = material;
                lastBoundMeshId = -1; // force mesh to rebind
                material->bind();
            }
            if (mesh->meshId != lastBoundMeshId) {
                builder.renderStats->stateChangesMesh++;
                lastBoundMeshId = mesh->meshId;
                mesh->bind(shader);
            }
            if (mesh->getIndexSets() == 0){
                glDrawArrays((GLenum) mesh->getMeshTopology(), 0, mesh->getVertexCount());
            } else {
                auto offsetCount = mesh->elementBufferOffsetCount[i];

                GLsizei indexCount = offsetCount.second;
                glDrawElements((GLenum) mesh->getMeshTopology(i), indexCount, GL_UNSIGNED_SHORT, (void*)offsetCount.first);
            }
        }
    }

    void RenderPass::finishGPUCommandBuffer() {
        glFinish();
    }

    void RenderPass::draw(std::shared_ptr<SpriteBatch>& spriteBatch, glm::mat4 modelTransform) {
        assert(!mIsFinished && "RenderPass is finished. Can no longer be modified.");
        if (spriteBatch == nullptr) return;

        for (int i=0;i<spriteBatch->materials.size();i++) {
            renderQueue.emplace_back(RenderQueueObj{spriteBatch->spriteMeshes[i], modelTransform, {spriteBatch->materials[i]}});
        }
    }

    void RenderPass::draw(std::shared_ptr<SpriteBatch>&& spriteBatch, glm::mat4 modelTransform) {
        assert(!mIsFinished && "RenderPass is finished. Can no longer be modified.");
        if (spriteBatch == nullptr) return;

        for (int i=0;i<spriteBatch->materials.size();i++) {
            renderQueue.emplace_back(RenderQueueObj{spriteBatch->spriteMeshes[i], modelTransform, {spriteBatch->materials[i]}});
        }
    }

    bool RenderPass::isFinished() {
        return mIsFinished;
    }

    void RenderPass::blit(std::shared_ptr<Texture> texture, glm::mat4 transformation) {
        auto material = Shader::getBlit()->createMaterial();
        material->setTexture(texture);
        blit(material, transformation);
    }

    void RenderPass::blit(std::shared_ptr<Material> material, glm::mat4 transformation) {
        static std::shared_ptr<Mesh> mesh;
        static bool once = [](){
            mesh = Mesh::create().withQuad().build();
            // always render
            float m = std::numeric_limits<float>::max();
            std::array<glm::vec3,2> minMax;
            minMax[0] = glm::vec3{-m};
            minMax[1] = glm::vec3{m};
            mesh->setBoundsMinMax(minMax);
            return true;
        } ();

        draw(mesh, transformation, material);
    }
}
