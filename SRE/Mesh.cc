//
// Created by morten on 31/07/16.
//

#include "Mesh.h"

#if defined(_WIN32)
#   define GLEW_STATIC
#   include <GL/glew.h>
#else
#   include <OpenGL/gl3.h>
#endif
#include <glm/gtc/constants.hpp>

#include "Shader.h"
#include <iostream>

#define BUFFER_OFFSET(i) ((char *)NULL + (i))

namespace SRE {
    Mesh::Mesh()
        :vertexCount{0}
    {
        glGenBuffers(1, &vertexBufferId);
        glGenVertexArrays(1, &vertexArrayObject);
    }
    Mesh::~Mesh(){
        glDeleteVertexArrays(1, &vertexArrayObject);
        glDeleteBuffers(1,&vertexBufferId);
    }

    void Mesh::bind(){
        glBindVertexArray(vertexArrayObject);
    }

    void Mesh::updateMesh(std::vector<glm::vec3> &vertexPositions, std::vector<glm::vec3> &normals, std::vector<glm::vec2> &uvs, MeshTopology meshTopology){
        this->meshTopology = meshTopology;
        this->vertexCount = (int) vertexPositions.size();
        if (normals.size() < vertexPositions.size()){
            normals.resize(vertexPositions.size(), glm::vec3(0,0,0));
        }
        if (uvs.size() < vertexPositions.size()){
            uvs.resize(vertexPositions.size(), glm::vec2(0,0));
        }
        // interleave data
        int floatsPerVertex = 8;
        std::vector<float> interleavedData(vertexPositions.size() * floatsPerVertex);
        for (int i=0;i<vertexPositions.size();i++){
            for (int j=0;j<3;j++){
                interleavedData[i*8+j] = vertexPositions[i][j];
                interleavedData[i*8+j+3] = normals[i][j];
                if (j<2){
                    interleavedData[i*8+j+6] = uvs[i][j];
                }
            }
        }

        glBindVertexArray(vertexArrayObject);
        glBindBuffer(GL_ARRAY_BUFFER, vertexBufferId);
        glBufferData(GL_ARRAY_BUFFER, sizeof(float)*interleavedData.size(), interleavedData.data(), GL_STATIC_DRAW);

        // bind vertex attributes (position, normal, uv)
        int length[3] = {3,3,2};
        int offset[3] = {0,3,6};
        for (int i=0;i<3;i++){
            glEnableVertexAttribArray(i);
            glVertexAttribPointer(i, length[i],GL_FLOAT,GL_FALSE, 8 * sizeof(float), BUFFER_OFFSET(offset[i] * sizeof(float)));
        }
    }

    Mesh* Mesh::createQuad(){
        Mesh *res = new Mesh();
        std::vector<glm::vec3> vertices({
                                       glm::vec3{1, -1, 0},
                                       glm::vec3{1, 1, 0},
                                       glm::vec3{-1, -1, 0},
                                       glm::vec3{-1, -1, 0},
                                       glm::vec3{1, 1, 0},
                                       glm::vec3{-1, 1, 0}
                               });
        std::vector<glm::vec3> normals({
                                       glm::vec3{0, 0, 1},
                                       glm::vec3{0, 0, 1},
                                       glm::vec3{0, 0, 1},
                                       glm::vec3{0, 0, 1},
                                       glm::vec3{0, 0, 1},
                                       glm::vec3{0, 0, 1}
                               });
        std::vector<glm::vec2> uvs({
                                       glm::vec2{1, 0},
                                       glm::vec2{1, 1},
                                       glm::vec2{0, 0},
                                       glm::vec2{0, 0},
                                       glm::vec2{1, 1},
                                       glm::vec2{0, 1}
                               });
        res->updateMesh(vertices,normals,uvs, MeshTopology::Triangles);

        return res;
    }

    Mesh* Mesh::createCube(){

        using namespace glm;
        using namespace std;
        float length = 1.0f;
        Mesh *res = new Mesh();
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
        vec2 u[] = {
                vec2(1,1),
                vec2(0,1),
                vec2(0,0),
                vec2(1,0)
        };
        vector<vec2> uvs({ u[0],u[1],u[2], u[0],u[2],u[3],
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
        res->updateMesh(positions,normals,uvs, MeshTopology::Triangles);
        return res;
    }

    Mesh* Mesh::createSphere(){
        using namespace glm;
        using namespace std;
        Mesh *res = new Mesh();
        int stacks = 16;
        int slices = 32;
        float radius = 1.0f;
        size_t vertexCount = (size_t) ((stacks + 1) * slices);
        vector<vec3> vertices{vertexCount};
        vector<vec3> normals{vertexCount};
        vector<vec2> uvs{vertexCount};

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
                uvs[index] = vec2{1 - i /(float) slices, j /(float) stacks};
                vertices[index] = normal * radius;
                index++;
            }
        }
        vector<vec3> finalPosition;
        vector<vec3> finalNormals;
        vector<vec2> finalUVs;
        // create indices
        for (int j = 0; j < stacks; j++) {
            for (int i = 0; i <= slices; i++) {
                glm::u8vec2 offset [] = {
                        // first triangle
                        glm::u8vec2{i,j},
                        glm::u8vec2{(i+1)%slices,j},
                        glm::u8vec2{(i+1)%slices,j+1},
                        // second triangle
                        glm::u8vec2{i,j},
                        glm::u8vec2{(i+1)%slices,j+1},
                        glm::u8vec2{i,j+1},
                };
                for (auto o : offset){
                    index = o[1] * slices  + o[0];
                    finalPosition.push_back(vertices[index]);
                    finalNormals.push_back(normals[index]);
                    finalUVs.push_back(uvs[index]);
                }

            }
        }
        res->updateMesh(finalPosition,finalNormals,finalUVs, MeshTopology::Triangles);

        return res;
    }

    MeshTopology Mesh::getMeshTopology() {
        return meshTopology;
    }

    int Mesh::getVertexCount() {
        return vertexCount;
    }
}