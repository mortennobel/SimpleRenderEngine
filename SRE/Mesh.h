#pragma once

#include "glm/glm.hpp"
#include <vector>

namespace SRE {

    class Shader;

    enum class MeshTopology {
        Points = 0x0000,
        Lines = 0x0001,
        Triangles = 0x0004
    };

    class Mesh {
    public:
        Mesh();
        ~Mesh();

        void bind();
        void updateMesh(std::vector<glm::vec3> &vertexPositions, std::vector<glm::vec3> &normals, std::vector<glm::vec2> &uvs, MeshTopology meshTopology = MeshTopology::Triangles);

        int getVertexCount();
        MeshTopology getMeshTopology();

        static Mesh* createQuad();
        static Mesh* createCube();
        static Mesh* createSphere();
    private:
        MeshTopology meshTopology;
        unsigned int vertexBufferId;
        unsigned int vertexArrayObject;
        int vertexCount;
    };
}