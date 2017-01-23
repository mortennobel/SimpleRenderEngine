#pragma once

#include "glm/glm.hpp"
#include <vector>
#include "SRE/MeshTopology.hpp"

#include "SRE/impl/Export.hpp"

namespace SRE {
    /**
     * Represents a Mesh object.
     * When constructing a mesh object, its data is uploaded to the CPU and is no longer available on the CPU.
     * A mesh is composed of a list of
     * - vertexPositions (vec3)
     * - normals (vec3)
     * - uvs (aka. texture coordinates) (vec2)
     *
     * A mesh also has a meshType, which can be either: MeshTopology::Points, MeshTopology::Lines, or MeshTopology::Triangles
     */
    class DllExport Mesh {
    public:
        class DllExport MeshBuilder {
        public:
//            MeshBuilder& withSphere();
//            MeshBuilder& withCube();
//            MeshBuilder& withQuad();
            MeshBuilder& withVertexPositions(const std::vector<glm::vec3> &vertexPositions);
            MeshBuilder& withNormals(const std::vector<glm::vec3> &normals);
            MeshBuilder& withUvs(const std::vector<glm::vec2> &uvs);
            MeshBuilder& withMeshTopology(MeshTopology meshTopology);
            MeshBuilder& withIndices(const std::vector<uint16_t> &indices);
            Mesh* build();
        private:
            MeshBuilder() = default;
            std::vector<glm::vec3> vertexPositions;
            std::vector<glm::vec3> normals;
            std::vector<glm::vec2> uvs;
            MeshTopology meshTopology = MeshTopology::Triangles;
            std::vector<uint16_t> indices;
            Mesh* updateMesh = nullptr;
            friend class Mesh;
        };
        ~Mesh();

        static MeshBuilder create();
        MeshBuilder update();

        int getVertexCount();
        MeshTopology getMeshTopology();

        const std::vector<glm::vec3>& getVertexPositions();
        const std::vector<glm::vec3>& getNormals();
        const std::vector<glm::vec2>& getUVs();
        const std::vector<uint16_t>& getIndices();

        static Mesh* createQuad();
        static Mesh* createCube();
        static Mesh* createSphere();
    private:
        Mesh(const std::vector<glm::vec3> &vertexPositions, const std::vector<glm::vec3> &normals, const std::vector<glm::vec2> &uvs,const std::vector<uint16_t> &indices, MeshTopology meshTopology = MeshTopology::Triangles);
        void update(const std::vector<glm::vec3> &vertexPositions, const std::vector<glm::vec3> &normals, const std::vector<glm::vec2> &uvs, const std::vector<uint16_t> &indices);

        void setVertexAttributePointers();
        MeshTopology meshTopology;
        unsigned int vertexBufferId;
        unsigned int vertexArrayObject;
        unsigned int elementBufferId;
        int vertexCount;

        std::vector<glm::vec3> vertexPositions;
        std::vector<glm::vec3> normals;
        std::vector<glm::vec2> uvs;
        std::vector<uint16_t> indices;

        void bind();

        friend class SimpleRenderEngine;
    };
}