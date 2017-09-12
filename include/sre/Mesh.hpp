/*
 *  SimpleRenderEngine (https://github.com/mortennobel/SimpleRenderEngine)
 *
 *  Created by Morten Nobel-Jørgensen ( http://www.nobel-joergnesen.com/ )
 *  License: MIT
 */

#pragma once

#include "glm/glm.hpp"
#include <vector>
#include <array>
#include <string>
#include <map>
#include "sre/MeshTopology.hpp"

#include "sre/impl/Export.hpp"
#include "Shader.hpp"
#include "RenderStats.hpp"

namespace sre {
    // forward declaration
    class Shader;

    /**
     * Represents a Mesh object.
     * A mesh is composed of a list of named vertex attributes
     * - positions (vec3)
     * - normals (vec3)
     * - uvs (aka. texture coordinates) (vec4)
     *
     * A mesh also has a meshType, which can be either: MeshTopology::Points, MeshTopology::Lines, or MeshTopology::Triangles
     *
     * The number and types of vertex attributes cannot be changed after the mesh has been created. The number of
     * vertices is allow to change.
     *
     * Note that each mesh can have multiple index sets associated with it which allows for using multiple materials for rendering.
     */
    class DllExport Mesh : public std::enable_shared_from_this<Mesh> {
    public:
        class DllExport MeshBuilder {
        public:
            // primitives
            MeshBuilder& withSphere(int stacks = 16, int slices = 32, float radius = 1);        // Creates a sphere mesh including UV coordinates, positions and normals
            MeshBuilder& withCube(float length = 1);                                            // Creates a cube including UV coordinates, positions and normals
            MeshBuilder& withQuad(float size=1);                                                // Creates a quad z,y = [-size;size] and z=0, UV=[0;1], normals=(0,0,1)
            MeshBuilder& withTorus(int segmentsC = 24, int segmentsA = 24, float radiusC = 1, float radiusA = .25);
                                                                                                // Creates a torus in xy plane. C is in the outer (large) circle, A is the sweeping circle.
            // raw data
            MeshBuilder& withPositions(const std::vector<glm::vec3> &vertexPositions);          // Set vertex attribute "position" of type vec3
            MeshBuilder& withNormals(const std::vector<glm::vec3> &normals);                    // Set vertex attribute "normal" of type vec3
            MeshBuilder& withUVs(const std::vector<glm::vec4> &uvs);                            // Set vertex attribute "uv" of type vec4 (treated as two sets of texture coordinates)
            MeshBuilder& withColors(const std::vector<glm::vec4> &colors);                      // Set vertex attribute "colors" of type vec4
            MeshBuilder& withParticleSizes(const std::vector<float> &particleSize);             // Set vertex attribute "particleSize" of type float
            MeshBuilder& withMeshTopology(MeshTopology meshTopology);                           // Defines the meshTopology (default is Triangles)
            MeshBuilder& withIndices(const std::vector<uint16_t> &indices, MeshTopology meshTopology = MeshTopology::Triangles, int indexSet=0);
                                                                                                // Defines the indices (if no indices defined then the vertices are rendered sequeantial)
            // custom data layout
            MeshBuilder& withAttribute(std::string name, const std::vector<float> &values);       // Set a named vertex attribute of float
            MeshBuilder& withAttribute(std::string name, const std::vector<glm::vec2> &values);   // Set a named vertex attribute of vec2
            MeshBuilder& withAttribute(std::string name, const std::vector<glm::vec3> &values);   // Set a named vertex attribute of vec3
            MeshBuilder& withAttribute(std::string name, const std::vector<glm::vec4> &values);   // Set a named vertex attribute of vec4
            MeshBuilder& withAttribute(std::string name, const std::vector<glm::i32vec4> &values);// Set a named vertex attribute of i32vec4

            // other
            MeshBuilder& withName(const std::string& name);                                       // Defines the name of the mesh

            std::shared_ptr<Mesh> build();
        private:
            MeshBuilder() = default;
            MeshBuilder(const MeshBuilder&) = default;
            std::map<std::string,std::vector<float>> attributesFloat;
            std::map<std::string,std::vector<glm::vec2>> attributesVec2;
            std::map<std::string,std::vector<glm::vec3>> attributesVec3;
            std::map<std::string,std::vector<glm::vec4>> attributesVec4;
            std::map<std::string,std::vector<glm::i32vec4>> attributesIVec4;
            std::vector<MeshTopology> meshTopology = {MeshTopology::Triangles};
            std::vector<std::vector<uint16_t>> indices;
            Mesh *updateMesh = nullptr;
            std::string name;
            friend class Mesh;
        };
        ~Mesh();

        static MeshBuilder create();                                // Create Mesh using the builder pattern. (Must end with build()).
        MeshBuilder update();                                       // Update the mesh using the builder pattern. (Must end with build()).

        int getVertexCount();                                       // Number of vertices in mesh

        std::vector<glm::vec3> getPositions();                      // Get position vertex attribute
        std::vector<glm::vec3> getNormals();                        // Get normal vertex attribute
        std::vector<glm::vec4> getUVs();                            // Get uv vertex attribute
        std::vector<glm::vec4> getColors();                         // Get color vertex attribute
        std::vector<float> getParticleSizes();                      // Get particle size vertex attribute

        int getIndexSets();                                         // Return the number of index sets
        MeshTopology getMeshTopology(int indexSet=0);               // Mesh topology used
        const std::vector<uint16_t>& getIndices(int indexSet=0);    // Indices used in the mesh
        int getIndicesSize(int indexSet=0);                         // Return the size of the index set

        template<typename T>
        inline T get(std::string attributeName);                    // Get the vertex attribute of a given type. Type must be float,glm::vec2,glm::vec3,glm::vec4,glm::i32vec4

        std::pair<int,int> getType(const std::string& name);        // return element type, element count

        std::vector<std::string> getAttributeNames();               // Names of the vertex attributes

        std::array<glm::vec3,2> getBoundsMinMax();                  // get the local axis aligned bounding box (AABB)

        const std::string& getName();                               // Return the mesh name

        int getDataSize();                                          // get size of the mesh in bytes on GPU
    private:
        struct Attribute {
            int offset;
            int elementCount;
            int dataType;
            int attributeType;
        };

        Mesh       (std::map<std::string,std::vector<float>>& attributesFloat, std::map<std::string,std::vector<glm::vec2>>& attributesVec2, std::map<std::string, std::vector<glm::vec3>>& attributesVec3, std::map<std::string,std::vector<glm::vec4>>& attributesVec4,std::map<std::string,std::vector<glm::i32vec4>>& attributesIVec4, const std::vector<std::vector<uint16_t>> &indices, std::vector<MeshTopology> meshTopology,std::string name,RenderStats& renderStats);
        void update(std::map<std::string,std::vector<float>>& attributesFloat, std::map<std::string,std::vector<glm::vec2>>& attributesVec2, std::map<std::string, std::vector<glm::vec3>>& attributesVec3, std::map<std::string,std::vector<glm::vec4>>& attributesVec4,std::map<std::string,std::vector<glm::i32vec4>>& attributesIVec4, const std::vector<std::vector<uint16_t>> &indices, std::vector<MeshTopology> meshTopology,std::string name,RenderStats& renderStats);

        int totalBytesPerVertex = 0;

        void setVertexAttributePointers(Shader* shader);
        std::vector<MeshTopology> meshTopology;
        unsigned int vertexBufferId;
        std::map<unsigned int,unsigned int> shaderToVertexArrayObject;
        std::vector<unsigned int> elementBufferId;
        int vertexCount;
        int dataSize;
        std::string name;
        std::map<std::string,Attribute> attributeByName;
        std::map<std::string,std::vector<float>> attributesFloat;
        std::map<std::string,std::vector<glm::vec2>> attributesVec2;
        std::map<std::string,std::vector<glm::vec3>> attributesVec3;
        std::map<std::string,std::vector<glm::vec4>> attributesVec4;
        std::map<std::string,std::vector<glm::i32vec4>> attributesIVec4;

        std::vector<std::vector<uint16_t>> indices;

        std::array<glm::vec3,2> boundsMinMax;

        void bind(Shader* shader);
        void bindIndexSet(int indexSet);

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
    inline const std::vector<glm::i32vec4>& Mesh::get(std::string uniformName) {
        return attributesIVec4[uniformName];
    }
}