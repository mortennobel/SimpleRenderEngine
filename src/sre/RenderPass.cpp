//
// Created by Morten Nobel-Jørgensen on 08/06/2017.
//

#include "sre/RenderPass.hpp"
#include "sre/Mesh.hpp"
#include "sre/Shader.hpp"
#include "sre/RenderStats.hpp"
#include "sre/Texture.hpp"
#include "sre/impl/GL.hpp"
#include <cassert>

namespace sre {
    RenderPass * RenderPass::currentRenderPass = nullptr;

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

    RenderPass RenderPass::RenderPassBuilder::build(){
        return RenderPass(std::move(camera), worldLights, renderStats);
    }

    RenderPass::~RenderPass(){
        if (currentRenderPass == this){
            currentRenderPass = nullptr;
        }
    }

    RenderPass::RenderPass(Camera&& camera, WorldLights* worldLights, RenderStats* renderStats)
    :camera(camera), worldLights(worldLights),renderStats(renderStats)
    {
        currentRenderPass = this;
        glViewport(camera.viewportX, camera.viewportY, camera.viewportWidth, camera.viewportHeight);
        glScissor(camera.viewportX, camera.viewportY, camera.viewportWidth, camera.viewportHeight);
    }


    void RenderPass::draw(Mesh *mesh, glm::mat4 modelTransform, Shader *shader) {
        renderStats->drawCalls++;
        setupShader(modelTransform, shader);

        mesh->bind();
        int indexCount = (int) mesh->getIndices().size();
        if (indexCount == 0){
            glDrawArrays((GLenum) mesh->getMeshTopology(), 0, mesh->getVertexCount());
        } else {
            glDrawElements((GLenum) mesh->getMeshTopology(), indexCount, GL_UNSIGNED_SHORT, 0);
        }
    }

    void RenderPass::setupShader(const glm::mat4 &modelTransform, Shader *shader)  {
        shader->bind();
        if (shader->getType("model").type != UniformType::Invalid) {
            shader->set("model", modelTransform);
        }
        if (shader->getType("view").type != UniformType::Invalid) {
            shader->set("view", camera.getViewTransform());
        }
        if (shader->getType("projection").type != UniformType::Invalid) {
            shader->set("projection", camera.getProjectionTransform());
        }
        if (shader->getType("normalMat").type != UniformType::Invalid){
            auto normalMatrix = transpose(inverse((glm::mat3)(camera.getViewTransform() * modelTransform)));
            shader->set("normalMat", normalMatrix);
        }
        if (shader->getType("view_height").type != UniformType::Invalid){
            shader->set("view_height", (float)camera.viewportHeight);
        }

        shader->setLights(worldLights, camera.getViewTransform());
    }

    void RenderPass::clearScreen(glm::vec4 color, bool clearColorBuffer, bool clearDepthBuffer, bool clearStencil) {
        glClearColor(color.r, color.g, color.b, color.a);
        GLbitfield clear = 0;
        if (clearColorBuffer){
            clear |= GL_COLOR_BUFFER_BIT;
        }
        if (clearDepthBuffer){
            glDepthMask(GL_TRUE);
            clear |= GL_DEPTH_BUFFER_BIT;
        }
        if (clearStencil){
            clear |= GL_STENCIL_BUFFER_BIT;
        }
        glClear(clear);
    }

    void RenderPass::drawLines(const std::vector<glm::vec3> &verts, glm::vec4 color, MeshTopology meshTopology) {
        Mesh *mesh = Mesh::create()
                .withVertexPositions(verts)
                .withMeshTopology(meshTopology)
                .build();
        Shader *shader = Shader::getUnlit();
        shader->set("color", color);
        shader->set("tex", Texture::getWhiteTexture());
        draw(mesh, glm::mat4(1), shader);
        delete mesh;
    }
}