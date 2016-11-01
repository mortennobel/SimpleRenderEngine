//
// Created by morten on 31/07/16.
//

#include "SRE/ParticleMesh.hpp"

#include "SRE/GL.hpp"
#include <glm/gtc/constants.hpp>
#include <iostream>

#define BUFFER_OFFSET(i) ((char *)NULL + (i))

namespace SRE {
    ParticleMesh::ParticleMesh(const std::vector<glm::vec3> &vertexPositions, const std::vector<glm::vec4> &colors, const std::vector<glm::vec2> &uv,const std::vector<float> &uvSize,const std::vector<float> &uvRotation, const std::vector<float> &particleSizes)
    {
        glGenBuffers(1, &vertexBufferId);
        glGenVertexArrays(1, &vertexArrayObject);

        update(vertexPositions, colors, uv, uvSize, uvRotation, particleSizes);
    }

    ParticleMesh::~ParticleMesh(){
        glDeleteVertexArrays(1, &vertexArrayObject);
        glDeleteBuffers(1,&vertexBufferId);
    }

    void ParticleMesh::bind(){
        glBindVertexArray(vertexArrayObject);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
    }

    int ParticleMesh::getVertexCount() {
        return vertexCount;
    }

    void ParticleMesh::update(const std::vector<glm::vec3> &vertexPositions, const std::vector<glm::vec4> &colors, const std::vector<glm::vec2> &uv, const std::vector<float> &uvSize,const std::vector<float> &uvRotation, const std::vector<float> &particleSizes) {
        this->vertexPositions = vertexPositions;
        this->colors = colors;
        this->uvCenter = uv;
        this->uv = uvSize;
        this->uvRotation = uvRotation;
        this->particleSizes = particleSizes;
        this->vertexCount = (int) vertexPositions.size();

        bool hasColors = colors.size() == vertexPositions.size();
        bool hasUVs = uv.size() == vertexPositions.size();
        bool hasUVSize = uvSize.size() == vertexPositions.size();
        bool hasUVRotation = uvRotation.size() == vertexPositions.size();
        bool hasParticleSizes = particleSizes.size() == vertexPositions.size();

        // interleave data
        int floatsPerVertex = 12;
        std::vector<float> interleavedData(vertexPositions.size() * floatsPerVertex);
        for (int i=0;i<vertexPositions.size();i++){
            for (int j=0;j<4;j++){
                if (j<3){
                    interleavedData[i*floatsPerVertex+j] = vertexPositions[i][j];
                } else {
                    interleavedData[i*floatsPerVertex+j] = hasParticleSizes ? particleSizes[i] : 1.0f;
                }
                interleavedData[i*floatsPerVertex+j+4] = hasColors ? colors[i][j] : 1.0f;
                // default uv values [0,0,1,1]
                if (j<2){
                    interleavedData[i*floatsPerVertex+j+8] = hasUVs ? uv[i][j] : 0.0f;
                } else if (j==2){
                    interleavedData[i*floatsPerVertex+j+8] = hasUVSize ? uvSize[i]  : 1.0f;
                } else if (j==3){
                    interleavedData[i*floatsPerVertex+j+8] = hasUVRotation ? uvRotation[i] : 0.0f;
                }
            }
        }

        glBindVertexArray(vertexArrayObject);
        glBindBuffer(GL_ARRAY_BUFFER, vertexBufferId);
        glBufferData(GL_ARRAY_BUFFER, sizeof(float)*interleavedData.size(), interleavedData.data(), GL_STATIC_DRAW);

        // bind vertex attributes (position+size, color, uvs)
        int length[3] = {4,4,4};
        int offset[3] = {0,4,8};
        for (int i=0;i<3;i++){
            glEnableVertexAttribArray(i);
            glVertexAttribPointer(i, length[i],GL_FLOAT,GL_FALSE, 8 * sizeof(float), BUFFER_OFFSET(offset[i] * sizeof(float)));
        }
    }

    const std::vector<glm::vec3> &ParticleMesh::getVertexPositions() {
        return vertexPositions;
    }

    const std::vector<glm::vec4> &ParticleMesh::getColors() {
        return colors;
    }

    const std::vector<glm::vec2> &ParticleMesh::getUV() {
        return uvCenter;
    }

    const std::vector<float> &ParticleMesh::getParticleSizes() {
        return particleSizes;
    }

    const std::vector<float> &ParticleMesh::getUVSize() {
        return uv;
    }

    const std::vector<float> &ParticleMesh::getUVRotation() {
        return uvRotation;
    }
}
