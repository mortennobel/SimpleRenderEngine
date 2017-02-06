//
// Created by morten on 31/07/16.
//

#include "SRE/Mesh.hpp"

#include "SRE/impl/GL.hpp"
#include <glm/gtc/constants.hpp>
#include <iostream>
#include <SRE/SimpleRenderEngine.hpp>

#define BUFFER_OFFSET(i) ((char *)NULL + (i))

namespace SRE {
    Mesh::Mesh(const std::vector<glm::vec3> &vertexPositions, const std::vector<glm::vec3> &normals,
               const std::vector<glm::vec4> &uvs, const std::vector<glm::vec4> &colors, std::vector<float> particleSize, const std::vector<uint16_t> &indices, MeshTopology meshTopology)

    {
        glGenBuffers(1, &vertexBufferId);
        glGenBuffers(1, &elementBufferId);
#ifndef EMSCRIPTEN
        glGenVertexArrays(1, &vertexArrayObject);
#endif
        update(vertexPositions, normals, uvs, colors, particleSize,indices, meshTopology);
    }

    Mesh::~Mesh(){
        RenderStats& renderStats = SimpleRenderEngine::instance->renderStats;
        renderStats.meshBytes -= getDataSize();
        renderStats.meshCount--;

#ifndef EMSCRIPTEN
        glDeleteVertexArrays(1, &vertexArrayObject);
#endif
        glDeleteBuffers(1,&vertexBufferId);
        glDeleteBuffers(1,&elementBufferId);
    }

    void Mesh::bind(){
#ifndef EMSCRIPTEN
        glBindVertexArray(vertexArrayObject);
#else
        glBindBuffer(GL_ARRAY_BUFFER, vertexBufferId);
        setVertexAttributePointers();
#endif
        if (indices.empty()){
            glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
        } else {
            glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, elementBufferId);
        }
    }

    MeshTopology Mesh::getMeshTopology() {
        return meshTopology;
    }

    int Mesh::getVertexCount() {
        return vertexCount;
    }

    void Mesh::update(const std::vector<glm::vec3> &vertexPositions, const std::vector<glm::vec3> &normals,
                      const std::vector<glm::vec4> &uvs, const std::vector<glm::vec4> &colors, std::vector<float> particleSize, const std::vector<uint16_t> &indices, MeshTopology meshTopology) {
        this->meshTopology = meshTopology;
        this->vertexPositions = vertexPositions;
        this->normals = normals;
        this->uvs = uvs;
        this->colors = colors;
        this->particleSize = particleSize;
        this->vertexCount = (int) vertexPositions.size();
        this->indices = indices;
        bool hasNormals = normals.size() == vertexPositions.size();
        bool hasUVs = uvs.size() == vertexPositions.size();
        bool hasColors = colors.size() == vertexPositions.size();
        bool hasParticleSize = particleSize.size() == vertexPositions.size();

        // interleave data
        int floatsPerVertex = 15;
        std::vector<float> interleavedData(vertexPositions.size() * floatsPerVertex);
        for (int i=0;i<vertexPositions.size();i++){
            for (int j=0;j<4;j++){
                // vertex position
                if (j<3){
                    interleavedData[i*floatsPerVertex+j] = vertexPositions[i][j];
                }
                if (j==3){
                    interleavedData[i*floatsPerVertex+j] = hasParticleSize ? particleSize[i]:1.0f;
                }
                // normals
                if (j<3) {
                    interleavedData[i * floatsPerVertex + j + 4] = hasNormals ? normals[i][j] : 0.0f;
                }
                interleavedData[i*floatsPerVertex+j+7] = hasUVs ? uvs[i][j] : (j==2?1.0f:0.0f);
                interleavedData[i*floatsPerVertex+j+11] = hasColors ? colors[i][j] : 1.0f;
            }
        }
#ifndef EMSCRIPTEN
        glBindVertexArray(vertexArrayObject);
#endif
        glBindBuffer(GL_ARRAY_BUFFER, vertexBufferId);
        glBufferData(GL_ARRAY_BUFFER, sizeof(float)*interleavedData.size(), interleavedData.data(), GL_STATIC_DRAW);

        setVertexAttributePointers();
        if (!indices.empty()){
            glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, elementBufferId);
            GLsizeiptr indicesSize = indices.size()*sizeof(uint16_t);
            glBufferData(GL_ELEMENT_ARRAY_BUFFER, indicesSize, indices.data(), GL_STATIC_DRAW);
        }
    }

    void Mesh::setVertexAttributePointers(){
        // bind vertex attributes (position+size, normal, uv, color)
        int length[4] = {4,3,4,4};
        int offset[4] = {0,4,7,11};
        int floatsPerVertex = 15;
        for (int i=0;i<4;i++){
            glEnableVertexAttribArray(i);
            glVertexAttribPointer(i, length[i],GL_FLOAT,GL_FALSE, floatsPerVertex * sizeof(float), BUFFER_OFFSET(offset[i] * sizeof(float)));
        }
    }

    const std::vector<glm::vec3> &Mesh::getVertexPositions() {
        return vertexPositions;
    }

    const std::vector<glm::vec3> &Mesh::getNormals() {
        return normals;
    }

    const std::vector<glm::vec4> &Mesh::getUVs() {
        return uvs;
    }

    const std::vector<uint16_t> &Mesh::getIndices() {
        return indices;
    }

    Mesh::MeshBuilder Mesh::update() {
        Mesh::MeshBuilder res;
        res.updateMesh = this;
        res.vertexPositions = vertexPositions;
        res.normals = normals;
        res.uvs = uvs;
        res.particleSize = particleSize;
        res.colors = colors;
        res.indices = indices;
        res.meshTopology = meshTopology;
        return res;
    }

    Mesh::MeshBuilder Mesh::create() {
        return Mesh::MeshBuilder();
    }

    const std::vector<glm::vec4> &Mesh::getColors() {
        return colors;
    }

    const std::vector<float> &Mesh::getParticleSize() {
        return particleSize;
    }

    int Mesh::getDataSize() {
        int size = 0;
        size += vertexPositions.size() * (sizeof(glm::vec3)+
                sizeof(glm::vec3)+ // normals
               sizeof(glm::vec4)+ // uvs
               sizeof(glm::vec4)+ // colors
               sizeof(float)); // particle size
        size += indices.size() * sizeof(uint16_t);
        return size;
    }

    Mesh::MeshBuilder &Mesh::MeshBuilder::withVertexPositions(const std::vector<glm::vec3> &vertexPositions) {
        this->vertexPositions = vertexPositions;
        return *this;
    }

    Mesh::MeshBuilder &Mesh::MeshBuilder::withNormals(const std::vector<glm::vec3> &normals) {
        this->normals = normals;
        return *this;
    }

    Mesh::MeshBuilder &Mesh::MeshBuilder::withUvs(const std::vector<glm::vec4> &uvs) {
        this->uvs = uvs;
        return *this;
    }

    Mesh::MeshBuilder &Mesh::MeshBuilder::withMeshTopology(MeshTopology meshTopology) {
        this->meshTopology = meshTopology;
        return *this;
    }

    Mesh::MeshBuilder &Mesh::MeshBuilder::withIndices(const std::vector<uint16_t> &indices) {
        this->indices = indices;
        return *this;
    }

    Mesh * Mesh::MeshBuilder::build() {
        // update stats
        RenderStats& renderStats = SimpleRenderEngine::instance->renderStats;
        Mesh *res;
        if (updateMesh != nullptr){
            renderStats.meshBytes -= updateMesh->getDataSize();
            updateMesh->update(vertexPositions, normals, uvs, colors, particleSize,indices, meshTopology);
            res = updateMesh;
        } else {
            res = new Mesh(vertexPositions, normals, uvs, colors, particleSize,indices, meshTopology);
            renderStats.meshCount++;
        }
        renderStats.meshBytes += res->getDataSize();

        return res;
    }

    Mesh::MeshBuilder &Mesh::MeshBuilder::withSphere() {
        using namespace glm;
        using namespace std;
        int stacks = 16/2;
        int slices = 32/2;
        float radius = 1.0f;
        size_t vertexCount = (size_t) ((stacks + 1) * slices);
        vector<vec3> vertices{vertexCount};
        vector<vec3> normals{vertexCount};
        vector<vec4> uvs{vertexCount};

        int index = 0;
        // create vertices
        for (unsigned short j = 0; j <= stacks; j++) {
            float latitude1 = (glm::pi<float>() / stacks) * j - (glm::pi<float>() / 2);
            float sinLat1 = sin(latitude1);
            float cosLat1 = cos(latitude1);
            for (int i = 0; i < slices; i++) {
                float longitude = ((glm::pi<float>() * 2) / slices) * i;
                float sinLong = sin(longitude);
                float cosLong = cos(longitude);
                vec3 normal{cosLong * cosLat1,
                            sinLat1,
                            sinLong * cosLat1};
                normal = normalize(normal);
                normals[index] = normal;
                uvs[index] = vec4{1 - i /(float) slices, j /(float) stacks,0,0};
                vertices[index] = normal * radius;
                index++;
            }
        }
        vector<vec3> finalPosition;
        vector<vec3> finalNormals;
        vector<vec4> finalUVs;
        // create indices
        for (int j = 0; j < stacks; j++) {
            for (int i = 0; i < slices; i++) {
                glm::u8vec2 offset [] = {
                        // first triangle
                        glm::u8vec2{i,j},
                        glm::u8vec2{(i+1)%slices,j+1},
                        glm::u8vec2{(i+1)%slices,j},

                        // second triangle
                        glm::u8vec2{i,j},
                        glm::u8vec2{i,j+1},
                        glm::u8vec2{(i+1)%slices,j+1},

                };
                for (auto o : offset){
                    index = o[1] * slices  + o[0];
                    finalPosition.push_back(vertices[index]);
                    finalNormals.push_back(normals[index]);
                    finalUVs.push_back(uvs[index]);
                }

            }
        }

        withVertexPositions(finalPosition);
        withNormals(finalNormals);
        withUvs(finalUVs);
        withMeshTopology(MeshTopology::Triangles);

        return *this;
    }

    Mesh::MeshBuilder &Mesh::MeshBuilder::withCube() {

        using namespace glm;
        using namespace std;
        float length = 1.0f;
        //    v5----- v4
        //   /|      /|
        //  v1------v0|
        //  | |     | |
        //  | |v6---|-|v7
        //  |/      |/
        //  v2------v3
        vec3 p[] = {
                vec3{length, length, length},
                vec3{-length, length, length},
                vec3{-length, -length, length},
                vec3{length, -length, length},

                vec3{length, length, -length},
                vec3{-length, length, -length},
                vec3{-length, -length, -length},
                vec3{length, -length, -length}

        };
        vector<vec3> positions({p[0],p[1],p[2], p[0],p[2],p[3], // v0-v1-v2-v3
                                p[4],p[0],p[3], p[4],p[3],p[7], // v4-v0-v3-v7
                                p[5],p[4],p[7], p[5],p[7],p[6], // v5-v4-v7-v6
                                p[1],p[5],p[6], p[1],p[6],p[2], // v1-v5-v6-v2
                                p[4],p[5],p[1], p[4],p[1],p[0], // v1-v5-v6-v2
                                p[3],p[2],p[6], p[3],p[6],p[7], // v1-v5-v6-v2
                               });
        vec4 u[] = {
                vec4(1,1,0,0),
                vec4(0,1,0,0),
                vec4(0,0,0,0),
                vec4(1,0,0,0)
        };
        vector<vec4> uvs({ u[0],u[1],u[2], u[0],u[2],u[3],
                           u[0],u[1],u[2], u[0],u[2],u[3],
                           u[0],u[1],u[2], u[0],u[2],u[3],
                           u[0],u[1],u[2], u[0],u[2],u[3],
                           u[0],u[1],u[2], u[0],u[2],u[3],
                           u[0],u[1],u[2], u[0],u[2],u[3],
                         });
        vector<vec3> normals({
                                     vec3{0, 0, 1},
                                     vec3{0, 0, 1},
                                     vec3{0, 0, 1},
                                     vec3{0, 0, 1},
                                     vec3{0, 0, 1},
                                     vec3{0, 0, 1},
                                     vec3{1, 0, 0},
                                     vec3{1, 0, 0},
                                     vec3{1, 0, 0},
                                     vec3{1, 0, 0},
                                     vec3{1, 0, 0},
                                     vec3{1, 0, 0},
                                     vec3{0, 0, -1},
                                     vec3{0, 0, -1},
                                     vec3{0, 0, -1},
                                     vec3{0, 0, -1},
                                     vec3{0, 0, -1},
                                     vec3{0, 0, -1},
                                     vec3{-1, 0, 0},
                                     vec3{-1, 0, 0},
                                     vec3{-1, 0, 0},
                                     vec3{-1, 0, 0},
                                     vec3{-1, 0, 0},
                                     vec3{-1, 0, 0},
                                     vec3{0, 1, 0},
                                     vec3{0, 1, 0},
                                     vec3{0, 1, 0},
                                     vec3{0, 1, 0},
                                     vec3{0, 1, 0},
                                     vec3{0, 1, 0},
                                     vec3{0, -1, 0},
                                     vec3{0, -1, 0},
                                     vec3{0, -1, 0},
                                     vec3{0, -1, 0},
                                     vec3{0, -1, 0},
                                     vec3{0, -1, 0},


                             });

        withVertexPositions(positions);
        withNormals(normals);
        withUvs(uvs);
        withMeshTopology(MeshTopology::Triangles);

        return *this;
    }

    Mesh::MeshBuilder &Mesh::MeshBuilder::withQuad() {

        std::vector<glm::vec3> vertices({
                                                glm::vec3{1, -1, 0},
                                                glm::vec3{1, 1, 0},
                                                glm::vec3{-1, -1, 0},
                                                glm::vec3{-1, 1, 0}
                                        });
        std::vector<glm::vec3> normals({
                                               glm::vec3{0, 0, 1},
                                               glm::vec3{0, 0, 1},
                                               glm::vec3{0, 0, 1},
                                               glm::vec3{0, 0, 1}
                                       });
        std::vector<glm::vec4> uvs({
                                           glm::vec4{1, 0,0,0},
                                           glm::vec4{1, 1,0,0},
                                           glm::vec4{0, 0,0,0},
                                           glm::vec4{0, 1,0,0}
                                   });
        std::vector<uint16_t> indices = {
                0,1,2,
                2,1,3
        };
        withVertexPositions(vertices);
        withNormals(normals);
        withUvs(uvs);
        withIndices(indices);
        withMeshTopology(MeshTopology::Triangles);

        return *this;
    }

    Mesh::MeshBuilder &Mesh::MeshBuilder::withColors(const std::vector<glm::vec4> &colors) {
        this->colors = colors;
        return *this;
    }

    Mesh::MeshBuilder &Mesh::MeshBuilder::withParticleSize(const std::vector<float> &particleSize) {
        this->particleSize = particleSize;
        return *this;
    }
}
