#pragma once

#include "glm/glm.hpp"
#include <vector>
#include "SRE/MeshTopology.hpp"

#include "SRE/impl/Export.hpp"

namespace SRE {
    // The size of the particles is relative to the viewport height
    class DllExport ParticleMesh {
    public:
        ParticleMesh(const std::vector<glm::vec3> &vertexPositions, const std::vector<glm::vec4> &colors, const std::vector<glm::vec2> &uv,const std::vector<float> &uvSize,const std::vector<float> &uvRotation, const std::vector<float> &particleSizes);
        ~ParticleMesh();

        void update(const std::vector<glm::vec3> &vertexPositions, const std::vector<glm::vec4> &colors, const std::vector<glm::vec2> &uv, const std::vector<float> &uvSize,const std::vector<float> &uvRotation, const std::vector<float> &particleSizes);

        int getVertexCount();

        const std::vector<glm::vec3>& getVertexPositions();
        const std::vector<glm::vec4>& getColors();
        const std::vector<glm::vec2>& getUV();
        const std::vector<float>&getUVSize();
        const std::vector<float>&getUVRotation();
        const std::vector<float>& getParticleSizes();
    private:
        unsigned int vertexBufferId;
        unsigned int vertexArrayObject;
        int vertexCount;

        std::vector<glm::vec3> vertexPositions;
        std::vector<glm::vec4> colors;
        std::vector<glm::vec2> uvCenter;
        std::vector<float> uv;
        std::vector<float> uvRotation;
        std::vector<float> particleSizes;

        void bind();

        friend class SimpleRenderEngine;
    };
}