#pragma once

#include "glm/glm.hpp"
#include <vector>
#include <array>
#include <string>
#include <map>
#include "sre/MeshTopology.hpp"

#include "sre/impl/Export.hpp"
#include "Shader.hpp"

namespace sre {
    // forward declaration
    class Shader;

    /**
     * Represents a Mesh object.
     * A mesh is composed of a list of named vertex attribute lists
     * - positions (vec3)
     * - normals (vec3)
     * - uvs (aka. texture coordinates) (vec4)
     *
     * A mesh also has a meshType, which can be either: MeshTopology::Points, MeshTopology::Lines, or MeshTopology::Triangles
     *
     * The number and types of vertex attributes cannot be changed after the mesh has been created. The number of
     * vertices is allow to change.
     */
    class DllExport Mesh : public std::enable_shared_from_this<Mesh> {
    public:
        class DllExport MeshBuilder {
        public:
            MeshBuilder& withSphere(int stacks = 16, int slices = 32, float radius = 1);
            MeshBuilder& withCube(float length = 1);
            MeshBuilder& withQuad();
            MeshBuilder& withPositions(const std::vector<glm::vec3> &vertexPositions);
            MeshBuilder& withNormals(const std::vector<glm::vec3> &normals);
            MeshBuilder& withUVs(const std::vector<glm::vec4> &uvs);
            MeshBuilder& withColors(const std::vector<glm::vec4> &colors);
            MeshBuilder& withParticleSizes(const std::vector<float> &particleSize);
            MeshBuilder& withUniform(std::string name,const std::vector<float> &values);
            MeshBuilder& withUniform(std::string name,const std::vector<glm::vec2> &values);
            MeshBuilder& withUniform(std::string name,const std::vector<glm::vec3> &values);
            MeshBuilder& withUniform(std::string name,const std::vector<glm::vec4> &values);
            MeshBuilder& withUniform(std::string name,const std::vector<glm::i32vec4> &values);
            MeshBuilder& withMeshTopology(MeshTopology meshTopology);
            MeshBuilder& withIndices(const std::vector<uint16_t> &indices);
            std::shared_ptr<Mesh> build();
        private:
            MeshBuilder() = default;
            MeshBuilder(const MeshBuilder&) = default;
            std::map<std::string,std::vector<float>> attributesFloat;
            std::map<std::string,std::vector<glm::vec2>> attributesVec2;
            std::map<std::string,std::vector<glm::vec3>> attributesVec3;
            std::map<std::string,std::vector<glm::vec4>> attributesVec4;
            std::map<std::string,std::vector<glm::i32vec4>> attributesIVec4;
            MeshTopology meshTopology = MeshTopology::Triangles;
            std::vector<uint16_t> indices;
            Mesh *updateMesh = nullptr;
            friend class Mesh;
        };
        ~Mesh();

        static MeshBuilder create();
        MeshBuilder update();

        int getVertexCount();
        MeshTopology getMeshTopology();

        std::vector<glm::vec3> getPositions();
        std::vector<glm::vec3> getNormals();
        std::vector<glm::vec4> getUVs();
        std::vector<glm::vec4> getColors();
        std::vector<float> getParticleSizes();

        template<typename T>
        inline T get(std::string attributeName);

        // return element type, element count
        std::pair<int,int> getType(const std::string& name);

        std::vector<std::string> getNames();

        std::vector<uint16_t> getIndices();

        // get the local axis aligned bounding box (AABB)
        std::array<glm::vec3,2> getBoundsMinMax();

        // get size of the mesh in bytes on GPU
        int getDataSize();
    private:
        struct Attribute {
            int offset;
            int elementCount;
            int dataType;
            int attributeType;
        };

        Mesh       (std::map<std::string,std::vector<float>>& attributesFloat, std::map<std::string,std::vector<glm::vec2>>& attributesVec2, std::map<std::string, std::vector<glm::vec3>>& attributesVec3, std::map<std::string,std::vector<glm::vec4>>& attributesVec4,std::map<std::string,std::vector<glm::i32vec4>>& attributesIVec4, const std::vector<uint16_t> &indices, MeshTopology meshTopology);
        void update(std::map<std::string,std::vector<float>>& attributesFloat, std::map<std::string,std::vector<glm::vec2>>& attributesVec2, std::map<std::string, std::vector<glm::vec3>>& attributesVec3, std::map<std::string,std::vector<glm::vec4>>& attributesVec4,std::map<std::string,std::vector<glm::i32vec4>>& attributesIVec4, const std::vector<uint16_t> &indices, MeshTopology meshTopology);

        int totalBytesPerVertex = 0;

        void setVertexAttributePointers(Shader* shader);
        MeshTopology meshTopology;
        unsigned int vertexBufferId;
        std::map<unsigned int,unsigned int> shaderToVertexArrayObject;
        unsigned int elementBufferId;
        int vertexCount;

        std::map<std::string,Attribute> attributeByName;
        std::map<std::string,std::vector<float>> attributesFloat;
        std::map<std::string,std::vector<glm::vec2>> attributesVec2;
        std::map<std::string,std::vector<glm::vec3>> attributesVec3;
        std::map<std::string,std::vector<glm::vec4>> attributesVec4;
        std::map<std::string,std::vector<glm::i32vec4>> attributesIVec4;

        std::vector<uint16_t> indices;

        std::array<glm::vec3,2> boundsMinMax;

        void bind(Shader* shader);

        friend class RenderPass;
    };

    template<>
    inline const std::vector<float>& Mesh::get(std::string uniformName) {
        return attributesFloat[uniformName];
    }

    template<>
    inline const std::vector<glm::vec2>& Mesh::get(std::string uniformName) {
        return attributesVec2[uniformName];
    }

    template<>
    inline const std::vector<glm::vec3>& Mesh::get(std::string uniformName) {
        return attributesVec3[uniformName];
    }

    template<>
    inline const std::vector<glm::vec4>& Mesh::get(std::string uniformName) {
        return attributesVec4[uniformName];
    }

    template<>
    inline const std::vector<glm::ivec4>& Mesh::get(std::string uniformName) {
        return attributesIVec4[uniformName];
    }
}