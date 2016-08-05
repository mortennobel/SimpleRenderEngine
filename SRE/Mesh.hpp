#pragma once

#include "glm/glm.hpp"
#include <vector>
#include "MeshTopology.hpp"

namespace SRE {

    class Shader;



    class Mesh {
    public:
        Mesh(std::vector<glm::vec3> &vertexPositions, std::vector<glm::vec3> &normals, std::vector<glm::vec2> &uvs, MeshTopology meshTopology = MeshTopology::Triangles);
        ~Mesh();

        void update(std::vector<glm::vec3> &vertexPositions, std::vector<glm::vec3> &normals, std::vector<glm::vec2> &uvs);

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