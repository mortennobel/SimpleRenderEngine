#pragma once

#include "glm/glm.hpp"
#include <vector>
#include "MeshTopology.h"

namespace SRE {

    class Shader;



    class Mesh {
    public:
        Mesh();
        ~Mesh();

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

        void bind();

        friend class SimpleRenderEngine;
    };
}