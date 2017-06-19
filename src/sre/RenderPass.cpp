//
// Created by Morten Nobel-Jørgensen on 08/06/2017.
//

#include "sre/RenderPass.hpp"
#include "sre/Mesh.hpp"
#include "sre/Shader.hpp"
#include "sre/Material.hpp"
#include "sre/RenderStats.hpp"
#include "sre/Texture.hpp"
#include "sre/impl/GL.hpp"
#include <cassert>
#include <glm/gtc/type_ptr.hpp>

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

    void RenderPass::draw(Mesh *mesh, glm::mat4 modelTransform, Material *material) {
        renderStats->drawCalls++;
        setupShader(modelTransform, material->getShader());
        if (material != lastBoundMaterial){
            renderStats->stateChangesMaterial++;
            lastBoundMaterial = material;
            material->bind();
        }
        if (mesh != lastBoundMesh){
            renderStats->stateChangesMesh++;
            lastBoundMesh = mesh;
            mesh->bind();
        }

        int indexCount = (int) mesh->getIndices().size();
        if (indexCount == 0){
            glDrawArrays((GLenum) mesh->getMeshTopology(), 0, mesh->getVertexCount());
        } else {
            glDrawElements((GLenum) mesh->getMeshTopology(), indexCount, GL_UNSIGNED_SHORT, 0);
        }
    }

    void RenderPass::setupShader(const glm::mat4 &modelTransform, Shader *shader)  {
        if (lastBoundShader == shader){
            if (shader->uniformLocationModel != -1){
                glUniformMatrix4fv(shader->uniformLocationModel, 1, GL_FALSE, glm::value_ptr(modelTransform));
            }
            if (shader->uniformLocationNormal != -1){
                auto normalMatrix = transpose(inverse((glm::mat3)(camera.getViewTransform() * modelTransform)));
                glUniformMatrix4fv(shader->uniformLocationNormal, 1, GL_FALSE, glm::value_ptr(normalMatrix));
            }
        } else {
            renderStats->stateChangesShader++;
            lastBoundShader = shader;
            shader->bind();
            if (shader->uniformLocationModel != -1){
                glUniformMatrix4fv(shader->uniformLocationModel, 1, GL_FALSE, glm::value_ptr(modelTransform));
            }
            if (shader->uniformLocationView != -1){
                glUniformMatrix4fv(shader->uniformLocationView, 1, GL_FALSE, glm::value_ptr(camera.viewTransform));
            }
            if (shader->uniformLocationProjection != -1){
                glUniformMatrix4fv(shader->uniformLocationProjection, 1, GL_FALSE, glm::value_ptr(camera.projectionTransform));
            }
            if (shader->uniformLocationNormal != -1){
                auto normalMatrix = transpose(inverse(((glm::mat3)camera.getViewTransform()) * ((glm::mat3)modelTransform)));
                glUniformMatrix3fv(shader->uniformLocationNormal, 1, GL_FALSE, glm::value_ptr(normalMatrix));
            }
            if (shader->uniformLocationViewport != -1){
                glm::vec4 viewport((float)camera.viewportWidth,(float)camera.viewportHeight,0,0);
                glUniform4fv(shader->uniformLocationViewport, 1, glm::value_ptr(viewport));
            }
            shader->setLights(worldLights, camera.getViewTransform());
        }
    }

    void RenderPass::clearScreen(glm::vec4 color, bool clearColorBuffer, bool clearDepthBuffer, bool clearStencil) {
        glClearColor(color.r, color.g, color.b, color.a);
        GLbitfield clear = 0;
        if (clearColorBuffer) {
            clear |= GL_COLOR_BUFFER_BIT;
        }
        if (clearDepthBuffer) {
            glDepthMask(GL_TRUE);
            clear |= GL_DEPTH_BUFFER_BIT;
        }
        if (clearStencil) {
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
        static Material material{shader};
        material.setColor(color);
        draw(mesh, glm::mat4(1), &material);
        delete mesh;
    }
}
