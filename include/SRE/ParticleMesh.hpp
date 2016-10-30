#pragma once

#include "glm/glm.hpp"
#include <vector>
#include "SRE/MeshTopology.hpp"

#include "SRE/Export.hpp"

namespace SRE {
    class DllExport ParticleMesh {
    public:
        ParticleMesh(const std::vector<glm::vec3> &vertexPositions, const std::vector<glm::vec4> &colors, const std::vector<glm::vec4> &uvs, const std::vector<float> &particleSizes);
        ~ParticleMesh();

        void update(const std::vector<glm::vec3> &vertexPositions, const std::vector<glm::vec4> &colors, const std::vector<glm::vec4> &uvs, const std::vector<float> &particleSizes);

        int getVertexCount();

        const std::vector<glm::vec3>& getVertexPositions();
        const std::vector<glm::vec4>& getColors();
        const std::vector<glm::vec4>& getUVs();
        const std::vector<float>& getParticleSizes();
    private:
        unsigned int vertexBufferId;
        unsigned int vertexArrayObject;
        int vertexCount;

        std::vector<glm::vec3> vertexPositions;
        std::vector<glm::vec4> colors;
        std::vector<glm::vec4> uvs;
        std::vector<float> particleSizes;

        void bind();

        friend class SimpleRenderEngine;
    };
}