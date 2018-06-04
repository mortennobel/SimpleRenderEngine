/*
 *  SimpleRenderEngine (https://github.com/mortennobel/SimpleRenderEngine)
 *
 *  Created by Morten Nobel-Jørgensen ( http://www.nobel-joergensen.com/ )
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
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/transform.hpp>
#include <glm/gtx/string_cast.hpp>

#define BUFFER_OFFSET(i) ((char *)NULL + (i))

namespace sre {
    // declare static variable
    RenderPass::FrameInspector RenderPass::frameInspector;

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

    RenderPass::RenderPassBuilder &RenderPass::RenderPassBuilder::withSkybox(std::shared_ptr<Skybox> skybox) {
        this->clearColor = false;
        this->skybox = skybox;
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
        if (builder.skybox){
            renderQueue.push_back({}); // reserve empty obj
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

    RenderPass::~RenderPass(){
        finish();
    }

    void RenderPass::draw(std::shared_ptr<Mesh>& meshPtr, glm::mat4 modelTransform, std::shared_ptr<Material>& material_ptr) {
        assert(!mIsFinished && "RenderPass is finished. Can no longer be modified.");
        renderQueue.emplace_back(RenderQueueObj{meshPtr, modelTransform, material_ptr});
    }

    void RenderPass::setupShaderRenderPass(Shader *shader){
        if (shader->uniformLocationView != -1) {
            glUniformMatrix4fv(shader->uniformLocationView, 1, GL_FALSE, glm::value_ptr(builder.camera.viewTransform));
        }
        if (shader->uniformLocationProjection != -1) {
            glUniformMatrix4fv(shader->uniformLocationProjection, 1, GL_FALSE, glm::value_ptr(projection));
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

    void RenderPass::setupShaderRenderPass(const RenderPass::GlobalUniforms &globalUniforms) {
        *globalUniforms.g_view = builder.camera.viewTransform;
        *globalUniforms.g_projection = projection;
        *globalUniforms.g_viewport = glm::vec4 ((float)viewportSize.x,(float)viewportSize.y,(float)viewportOffset.x,(float)viewportOffset.y);;
        *globalUniforms.g_cameraPos = glm::vec4(this->builder.camera.getPosition(),1.0f);;
        int maxSceneLights = Renderer::instance->maxSceneLights;
        size_t lightSize = sizeof(glm::vec4)*(1 + maxSceneLights*2);
        memset(globalUniforms.g_ambientLight,0, lightSize); // ambient + (lightPosType + lightColorRange) * maxSceneLights
        if (builder.worldLights){
            *globalUniforms.g_ambientLight = glm::vec4(builder.worldLights->getAmbientLight(),1.0);

            for (int i=0;i<maxSceneLights;i++){
                auto light = builder.worldLights->getLight(i);
                if (light == nullptr || light->lightType == LightType::Unused) {
                    globalUniforms.g_lightPosType[i] = glm::vec4(0.0f,0.0f,0.0f, 2);

                    continue;
                } else if (light->lightType == LightType::Point) {
                    globalUniforms.g_lightPosType[i] = glm::vec4(light->position, 1);
                } else if (light->lightType == LightType::Directional) {
                    globalUniforms.g_lightPosType[i] = glm::vec4(glm::normalize(light->direction), 0);
                }
                globalUniforms.g_lightColorRange[i] = glm::vec4(light->color, light->range);
            }
        }
        glBindBuffer(GL_UNIFORM_BUFFER, Renderer::instance->globalUniformBuffer);
        glBufferData(GL_UNIFORM_BUFFER, Renderer::instance->globalUniformBufferSize, globalUniforms.g_view, GL_STREAM_DRAW);
        glBindBuffer(GL_UNIFORM_BUFFER, 0);
    }

    void RenderPass::setupShader(const glm::mat4 &modelTransform, Shader *shader)  {
        if (lastBoundShader != shader){
            builder.renderStats->stateChangesShader++;
            lastBoundShader = shader;
            shader->bind();
        }
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
                                         material
                                 });
    }

    void RenderPass::setupGlobalShaderUniforms(){
        static std::vector<char> buffer;
        static GlobalUniforms globalUniforms;
        static bool once = [&](){
            // allocate buffer and setup pointers into buffer
            buffer.resize(Renderer::instance->globalUniformBufferSize, 0);
            globalUniforms.g_view = reinterpret_cast<glm::mat4 *>(buffer.data());
            globalUniforms.g_projection = reinterpret_cast<glm::mat4 *>(buffer.data() + sizeof(glm::mat4));
            globalUniforms.g_viewport = reinterpret_cast<glm::vec4 *>(buffer.data() + sizeof(glm::mat4)*2);
            globalUniforms.g_cameraPos = reinterpret_cast<glm::vec4 *>(buffer.data() + sizeof(glm::mat4)*2 + sizeof(glm::vec4));
            globalUniforms.g_ambientLight = reinterpret_cast<glm::vec4 *>(buffer.data() + sizeof(glm::mat4)*2 + sizeof(glm::vec4)*2);
            int lightColorRangeOffset = sizeof(glm::mat4)*2 + sizeof(glm::vec4)*3;
            globalUniforms.g_lightColorRange = reinterpret_cast<glm::vec4*>(buffer.data() + lightColorRangeOffset);
            int g_lightPosTypeOffset = lightColorRangeOffset+ sizeof(glm::vec4)*(Renderer::instance->maxSceneLights);
            globalUniforms.g_lightPosType = reinterpret_cast<glm::vec4*>(buffer.data() + g_lightPosTypeOffset );
            return true;
        } ();
        auto& rinfo = renderInfo();
        if (Renderer::instance->globalUniformBuffer){
            setupShaderRenderPass(globalUniforms);
        } else {
            // find list of used shaders
            std::set<Shader*> shaders;

            for (auto &rqObj : renderQueue) {
                assert(rqObj.material.get());
                assert(rqObj.material->getShader().get());
                assert(rqObj.mesh.get());
                shaders.insert(rqObj.material->getShader().get());
            }
            // update global uniforms
            for (auto shader : shaders){
                glUseProgram(shader->shaderProgramId);
                setupShaderRenderPass(shader);
            }
        }
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
            glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
        }
        if (builder.clearDepth) {
            glClearDepthf(builder.clearDepthValue);
            glDepthMask(GL_TRUE);
            clear |= GL_DEPTH_BUFFER_BIT;
            glDepthMask(GL_TRUE);
        }
        if (builder.clearStencil) {
            glClearStencil(builder.clearStencilValue);
            clear |= GL_STENCIL_BUFFER_BIT;
            glStencilMask(0xFFFF);
        }
        if (clear != 0u) {
            glClear(clear);
        }

        projection = builder.camera.getProjectionTransform(viewportSize);

        if (builder.skybox) {
            // Create an infinite projection
            glm::mat4 inf = builder.camera.getInfiniteProjectionTransform(viewportSize);
            renderQueue[0] = {builder.skybox->skyboxMesh,
                                inf, // passing the inf projection as the model matrix
                                builder.skybox->material};
        }

        setupGlobalShaderUniforms();

        for (auto & rqObj : renderQueue){
            drawInstance(rqObj);
        }

        if (builder.gui) {
            ImGui::Render();
            ImGui_SRE_RenderDrawData(ImGui::GetDrawData());
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
        checkGLError("RenderPass");
#endif
        if (frameInspector.frameid == Renderer::instance->getRenderStats().frame){
            // make a copy of this renderpass as a shared_ptr
            frameInspector.renderPasses.push_back(std::shared_ptr<RenderPass>(new RenderPass(*this)));
        } 
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
        int subMesh = 0;
        for (auto & mat : materials){
            renderQueue.emplace_back(RenderQueueObj{meshPtr, modelTransform, mat,subMesh});
            subMesh++;
        }
    }

    void RenderPass::drawInstance(RenderQueueObj& rqObj) {


        Mesh* mesh = rqObj.mesh.get();
        auto material = rqObj.material.get();
        auto shader = material->getShader().get();
        assert(mesh  != nullptr);
        builder.renderStats->drawCalls++;
        setupShader(rqObj.modelTransform, shader);
        if (material != lastBoundMaterial)
        {
            builder.renderStats->stateChangesMaterial++;
            lastBoundMaterial = material;
            lastBoundMeshId = -1; // force mesh to rebind
            material->bind();
        }
        if (mesh->meshId != lastBoundMeshId)
        {
            builder.renderStats->stateChangesMesh++;
            lastBoundMeshId = mesh->meshId;
            mesh->bind(shader);
        }
        if (mesh->elementBufferOffsetCount.empty()){
            glDrawArrays((GLenum) mesh->getMeshTopology(), 0, mesh->getVertexCount());
        } else {
            auto offsetCount = mesh->elementBufferOffsetCount[rqObj.subMesh];
            glDrawElements((GLenum) mesh->getMeshTopology(rqObj.subMesh), offsetCount.size, offsetCount.type, BUFFER_OFFSET(offsetCount.offset));
        }
    }

    void RenderPass::finishGPUCommandBuffer() {
        glFinish();
    }

    void RenderPass::draw(std::shared_ptr<SpriteBatch>& spriteBatch, glm::mat4 modelTransform) {
        assert(!mIsFinished && "RenderPass is finished. Can no longer be modified.");
        if (spriteBatch == nullptr) return;

        for (int i=0;i<spriteBatch->materials.size();i++) {
            renderQueue.emplace_back(RenderQueueObj{spriteBatch->spriteMeshes[i], modelTransform, spriteBatch->materials[i]});
        }
    }

    void RenderPass::draw(std::shared_ptr<SpriteBatch>&& spriteBatch, glm::mat4 modelTransform) {
        assert(!mIsFinished && "RenderPass is finished. Can no longer be modified.");
        if (spriteBatch == nullptr) return;

        for (int i=0;i<spriteBatch->materials.size();i++) {
            renderQueue.emplace_back(RenderQueueObj{spriteBatch->spriteMeshes[i], modelTransform, spriteBatch->materials[i]});
        }
    }

    bool RenderPass::isFinished() {
        return mIsFinished;
    }

    void RenderPass::blit(std::shared_ptr<Texture> texture, glm::mat4 transformation) {
        auto material = Shader::getBlit()->createMaterial();
        material->setTexture(std::move(texture));
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
