/*
 *  SimpleRenderEngine
 *
 *  Created by Morten Nobel-Jørgensen ( http://www.nobel-joergnesen.com/ )
 *  License: MIT
 */

#include "sre/Mesh.hpp"

#include <algorithm>
#include "sre/impl/GL.hpp"
#include <glm/gtc/constants.hpp>
#include <iostream>
#include <glm/gtx/string_cast.hpp>
#include <iomanip>
#include "sre/Renderer.hpp"
#include "sre/Shader.hpp"
#include "sre/Log.hpp"

#define BUFFER_OFFSET(i) ((char *)NULL + (i))

namespace sre {
    Mesh::Mesh(std::map<std::string,std::vector<float>>& attributesFloat,std::map<std::string,std::vector<glm::vec2>>& attributesVec2, std::map<std::string,std::vector<glm::vec3>>& attributesVec3,std::map<std::string,std::vector<glm::vec4>>& attributesVec4,std::map<std::string,std::vector<glm::ivec4>>& attributesIVec4, const std::vector<std::vector<uint16_t>> &indices, std::vector<MeshTopology> meshTopology, std::string name,RenderStats& renderStats)
    {
        if ( Renderer::instance == nullptr){
            LOG_FATAL("Cannot instantiate sre::Mesh before sre::Renderer is created.");
        }
        glGenBuffers(1, &vertexBufferId);
        update(attributesFloat, attributesVec2, attributesVec3,attributesVec4,attributesIVec4, indices, meshTopology,name,renderStats);
        Renderer::instance->meshes.emplace_back(this);
    }

    Mesh::~Mesh(){
        auto r = Renderer::instance;
        if (r != nullptr){
            RenderStats& renderStats = r->renderStats;
            auto datasize = getDataSize();
            renderStats.meshBytes -= datasize;
            renderStats.meshBytesDeallocated += datasize;
            renderStats.meshCount--;
            r->meshes.erase(std::remove(r->meshes.begin(), r->meshes.end(), this));
        }

#ifndef EMSCRIPTEN
        for (auto arrayObj : shaderToVertexArrayObject) {
            glDeleteVertexArrays(1, &(arrayObj.second));
        }
#endif
        glDeleteBuffers(1, &vertexBufferId);
        glDeleteBuffers((GLsizei)elementBufferId.size(), elementBufferId.data());

    }

    void Mesh::bind(Shader* shader) {
#ifndef EMSCRIPTEN
        auto res = shaderToVertexArrayObject.find(shader->shaderProgramId);
        if (res != shaderToVertexArrayObject.end()) {
            GLuint vao = res->second;
            glBindVertexArray(vao);
        } else {
            GLuint index;
            glGenVertexArrays(1, &index);
            glBindVertexArray(index);
            setVertexAttributePointers(shader);
            shaderToVertexArrayObject[shader->shaderProgramId] = index;
        }
#else
        setVertexAttributePointers(shader);
#endif
    }
    void Mesh::bindIndexSet(int indexSet){
        if (indices.empty()){
            glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
        } else {
            glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, elementBufferId[indexSet]);
        }
    }

    MeshTopology Mesh::getMeshTopology(int indexSet) {
        return meshTopology[indexSet];
    }

    int Mesh::getVertexCount() {
        return vertexCount;
    }

    void Mesh::update(std::map<std::string,std::vector<float>>& attributesFloat,std::map<std::string,std::vector<glm::vec2>>& attributesVec2, std::map<std::string,std::vector<glm::vec3>>& attributesVec3,std::map<std::string,std::vector<glm::vec4>>& attributesVec4,std::map<std::string,std::vector<glm::ivec4>>& attributesIVec4, const std::vector<std::vector<uint16_t>> &indices, std::vector<MeshTopology> meshTopology,std::string name,RenderStats& renderStats) {
        this->meshTopology = meshTopology;
        this->name = name;

        vertexCount = 0;
        totalBytesPerVertex = 0;
        dataSize = 0;
        std::vector<int> offset;

#ifndef EMSCRIPTEN
        for (auto arrayObj : shaderToVertexArrayObject){
            glDeleteVertexArrays(1, &(arrayObj.second));
        }
        shaderToVertexArrayObject.clear();
#endif
        attributeByName.clear();

        // enforced std140 layout rules ( https://learnopengl.com/#!Advanced-OpenGL/Advanced-GLSL )
        // the order is vec3, vec4, ivec4, vec2, float
        for (auto & pair : attributesVec3){
            vertexCount = std::max(vertexCount, (int)pair.second.size());
            offset.push_back(totalBytesPerVertex);
            attributeByName[pair.first] = {totalBytesPerVertex, 3, GL_FLOAT, GL_FLOAT_VEC3};
            totalBytesPerVertex += sizeof(glm::vec4); // note use vec4 size
        }
        for (auto & pair : attributesVec4){
            vertexCount = std::max(vertexCount, (int)pair.second.size());
            offset.push_back(totalBytesPerVertex);
            attributeByName[pair.first] = {totalBytesPerVertex, 4, GL_FLOAT, GL_FLOAT_VEC4};
            totalBytesPerVertex += sizeof(glm::vec4);
        }
        for (auto & pair : attributesIVec4){
            vertexCount = std::max(vertexCount, (int)pair.second.size());
            offset.push_back(totalBytesPerVertex);
            attributeByName[pair.first] = {totalBytesPerVertex, 4,GL_INT, GL_INT_VEC4};
            totalBytesPerVertex += sizeof(glm::i32vec4);
        }
        for (auto & pair : attributesVec2){
            vertexCount = std::max(vertexCount, (int)pair.second.size());
            offset.push_back(totalBytesPerVertex);
            attributeByName[pair.first] = {totalBytesPerVertex, 2, GL_FLOAT,GL_FLOAT_VEC2};
            totalBytesPerVertex += sizeof(glm::vec2);
        }
        for (auto & pair : attributesFloat){
            vertexCount = std::max(vertexCount, (int)pair.second.size());
            offset.push_back(totalBytesPerVertex);
            attributeByName[pair.first] = {totalBytesPerVertex, 1, GL_FLOAT, GL_FLOAT};
            totalBytesPerVertex += sizeof(float);
        }
        // add final padding (make vertex align with vec4)
        if (totalBytesPerVertex%(sizeof(float)*4) != 0) {
            totalBytesPerVertex += sizeof(float)*4 - totalBytesPerVertex%(sizeof(float)*4);
        }
        std::vector<float> interleavedData((vertexCount * totalBytesPerVertex) / sizeof(float), 0);
        const char * dataPtr = (char*) interleavedData.data();

        // add data (copy each element into interleaved buffer)
        for (auto & pair : attributesVec3){
            auto& offsetBytes = attributeByName[pair.first];
            for (int i=0;i<pair.second.size();i++){
                glm::vec3 * locationPtr = (glm::vec3 *) (dataPtr + (totalBytesPerVertex * i) + offsetBytes.offset);
                *locationPtr = pair.second[i];
            }
        }
        for (auto & pair : attributesVec4){
            auto& offsetBytes = attributeByName[pair.first];
            for (int i=0;i<pair.second.size();i++) {
                glm::vec4 * locationPtr = (glm::vec4 *) (dataPtr + totalBytesPerVertex * i + offsetBytes.offset);
                *locationPtr = pair.second[i];
            }
        }
        for (auto & pair : attributesIVec4){
            auto& offsetBytes = attributeByName[pair.first];
            for (int i=0;i<pair.second.size();i++) {
                glm::i32vec4 * locationPtr = (glm::i32vec4 *) (dataPtr + totalBytesPerVertex * i + offsetBytes.offset);
                *locationPtr = pair.second[i];
            }
        }
        for (auto & pair : attributesVec2){
            auto& offsetBytes = attributeByName[pair.first];
            for (int i=0;i<pair.second.size();i++) {
                glm::vec2 * locationPtr = (glm::vec2 *) (dataPtr + totalBytesPerVertex * i + offsetBytes.offset);
                *locationPtr = pair.second[i];
            }
        }
        for (auto & pair : attributesFloat){
            auto& offsetBytes = attributeByName[pair.first];
            for (int i=0;i<pair.second.size();i++) {
                float * locationPtr = (float *) (dataPtr + totalBytesPerVertex * i + offsetBytes.offset);
                *locationPtr = pair.second[i];
            }
        }

#ifndef EMSCRIPTEN
        glBindVertexArray(0);
#endif
        glBindBuffer(GL_ARRAY_BUFFER, vertexBufferId);
        glBufferData(GL_ARRAY_BUFFER, sizeof(float)*interleavedData.size(), interleavedData.data(), GL_STATIC_DRAW);

        if (!indices.empty()){
            for (int i=0;i<indices.size();i++){
                unsigned int eBufferId;
                if (i >= elementBufferId.size()){
                    glGenBuffers(1, &eBufferId);
                    elementBufferId.push_back(eBufferId);
                } else {
                    eBufferId = elementBufferId[i];
                }
                glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, eBufferId);
                GLsizeiptr indicesSize = indices[i].size()*sizeof(uint16_t);
                glBufferData(GL_ELEMENT_ARRAY_BUFFER, indicesSize, indices[i].data(), GL_STATIC_DRAW);
                dataSize += indicesSize;
            }
        }

        this->indices = std::move(indices);
        this->attributesFloat = std::move(attributesFloat);
        this->attributesVec2  = std::move(attributesVec2);
        this->attributesVec3  = std::move(attributesVec3);
        this->attributesVec4  = std::move(attributesVec4);
        this->attributesIVec4 = std::move(attributesIVec4);

        boundsMinMax[0] = glm::vec3{std::numeric_limits<float>::max()};
        boundsMinMax[1] = glm::vec3{-std::numeric_limits<float>::max()};
        auto pos = this->attributesVec3.find("position");
        if (pos != this->attributesVec3.end()){
            for (auto v : pos->second){
                boundsMinMax[0] = glm::min(boundsMinMax[0], v);
                boundsMinMax[1] = glm::max(boundsMinMax[1], v);
            }
        }
        dataSize = totalBytesPerVertex*vertexCount;

        renderStats.meshBytes += dataSize;
        renderStats.meshBytesAllocated += dataSize;
    }

    void Mesh::setVertexAttributePointers(Shader* shader) {
        glBindBuffer(GL_ARRAY_BUFFER, vertexBufferId);
        int vertexAttribArray = 0;
        for (auto attribute : shader->attributes) {
            auto res = attributeByName.find(attribute.first);
            if (res != attributeByName.end() && attribute.second.type == res->second.attributeType && attribute.second.arraySize == 1) {
                glEnableVertexAttribArray(attribute.second.position);
                glVertexAttribPointer(attribute.second.position, res->second.elementCount, res->second.dataType, GL_FALSE, totalBytesPerVertex, BUFFER_OFFSET(res->second.offset));
                vertexAttribArray++;
            } else {
                glDisableVertexAttribArray(attribute.second.position);
                switch (attribute.second.type){
                    case GL_FLOAT:
                        if (attribute.second.arraySize==1){
                            glVertexAttrib1f(attribute.second.position,0);
                        } else if (attribute.second.arraySize==2){
                            glVertexAttrib2f(attribute.second.position,0,0);
                        } else if (attribute.second.arraySize==3){
                            glVertexAttrib3f(attribute.second.position,0,0,0);
                        } else if (attribute.second.arraySize==4){
                            glVertexAttrib4f(attribute.second.position,0,0,0,0);
                        }
                        break;
#ifndef EMSCRIPTEN
                    case GL_INT:
                        if (attribute.second.arraySize==1){
                            glVertexAttribI1i(attribute.second.position,0);
                        } else if (attribute.second.arraySize==2){
                            glVertexAttribI2i(attribute.second.position,0,0);
                        } else if (attribute.second.arraySize==3){
                            glVertexAttribI3i(attribute.second.position,0,0,0);
                        } else if (attribute.second.arraySize==4){
                            glVertexAttribI4i(attribute.second.position,0,0,0,0);
                        }
                        break;
#endif
                }
            }
        }
    }

    std::vector<glm::vec3> Mesh::getPositions() {
        std::vector<glm::vec3> res;
        auto ref = attributesVec3.find("position");
        if (ref != attributesVec3.end()){
            res = ref->second;
        }
        return res;
    }

    std::vector<glm::vec3> Mesh::getNormals() {
        std::vector<glm::vec3> res;
        auto ref = attributesVec3.find("normal");
        if (ref != attributesVec3.end()){
            res = ref->second;
        }
        return res;
    }

    std::vector<glm::vec4> Mesh::getUVs() {
        std::vector<glm::vec4> res;
        auto ref = attributesVec4.find("uv");
        if (ref != attributesVec4.end()){
            res = ref->second;
        }
        return res;
    }

    const std::vector<uint16_t>& Mesh::getIndices(int indexSet) {
        return indices.at(indexSet);
    }

    Mesh::MeshBuilder Mesh::update() {
        Mesh::MeshBuilder res;
        res.updateMesh = this;

        res.attributesFloat = attributesFloat;
        res.attributesVec2 = attributesVec2;
        res.attributesVec3 = attributesVec3;
        res.attributesVec4 = attributesVec4;
        res.attributesIVec4 = attributesIVec4;

        res.indices = indices;
        res.meshTopology = meshTopology;
        return res;
    }

    Mesh::MeshBuilder Mesh::create() {
        return Mesh::MeshBuilder();
    }

    std::vector<glm::vec4> Mesh::getColors() {
        std::vector<glm::vec4> res;
        auto ref = attributesVec4.find("color");
        if (ref != attributesVec4.end()){
            res = ref->second;
        }
        return res;
    }

    std::vector<float> Mesh::getParticleSizes() {
        std::vector<float> res;
        auto ref = attributesFloat.find("particleSize");
        if (ref != attributesFloat.end()){
            res = ref->second;
        }
        return res;
    }

    int Mesh::getDataSize() {
        return dataSize;
    }

    std::array<glm::vec3,2> Mesh::getBoundsMinMax() {
        return boundsMinMax;
    }

    std::pair<int,int> Mesh::getType(const std::string &name) {
        auto res = attributeByName.find(name);
        if (res != attributeByName.end()){
            return {res->second.dataType,res->second.elementCount};
        }
        return {-1,-1};
    }

    std::vector<std::string> Mesh::getAttributeNames() {
        std::vector<std::string> res;
        for (auto & u : attributeByName){
            res.push_back(u.first);
        }
        return res;
    }

    int Mesh::getIndexSets() {
        return indices.size();
    }

    const std::string& Mesh::getName() {
        return name;
    }

    int Mesh::getIndicesSize(int indexSet) {
        return indices.at(indexSet).size();
    }

    Mesh::MeshBuilder &Mesh::MeshBuilder::withPositions(const std::vector<glm::vec3> &vertexPositions) {
        withAttribute("position", vertexPositions);
        return *this;
    }

    Mesh::MeshBuilder &Mesh::MeshBuilder::withNormals(const std::vector<glm::vec3> &normals) {
        withAttribute("normal", normals);
        return *this;
    }

    Mesh::MeshBuilder &Mesh::MeshBuilder::withUVs(const std::vector<glm::vec4> &uvs) {
        withAttribute("uv", uvs);
        return *this;
    }

    Mesh::MeshBuilder &Mesh::MeshBuilder::withColors(const std::vector<glm::vec4> &colors) {
        withAttribute("color", colors);
        return *this;
    }

    Mesh::MeshBuilder &Mesh::MeshBuilder::withParticleSizes(const std::vector<float> &particleSize) {
        withAttribute("particleSize", particleSize);
        return *this;
    }

    Mesh::MeshBuilder &Mesh::MeshBuilder::withMeshTopology(MeshTopology meshTopology) {
        if (this->meshTopology.empty()){
            this->meshTopology.emplace_back();
        }
        this->meshTopology[0] = meshTopology;
        return *this;
    }

    Mesh::MeshBuilder &Mesh::MeshBuilder::withIndices(const std::vector<uint16_t> &indices,MeshTopology meshTopology, int indexSet) {
        while (indexSet >= this->indices.size()){
            this->indices.emplace_back();
        }
        while (indexSet >= this->meshTopology.size()){
            this->meshTopology.emplace_back();
        }

        this->indices[indexSet] = indices;
        this->meshTopology[indexSet] = meshTopology;
        return *this;
    }

    std::shared_ptr<Mesh> Mesh::MeshBuilder::build() {
        // update stats
        RenderStats& renderStats = Renderer::instance->renderStats;

        if (name.length()==0){
            name = "Unnamed Mesh";
        }

        if (updateMesh != nullptr){
            renderStats.meshBytes -= updateMesh->getDataSize();
            updateMesh->update(this->attributesFloat, this->attributesVec2, this->attributesVec3, this->attributesVec4, this->attributesIVec4, indices, meshTopology,name,renderStats);


            return updateMesh->shared_from_this();
        }

        auto res = new Mesh(this->attributesFloat, this->attributesVec2, this->attributesVec3, this->attributesVec4, this->attributesIVec4, indices, meshTopology,name,renderStats);
        renderStats.meshCount++;

        return std::shared_ptr<Mesh>(res);

    }

    Mesh::MeshBuilder &Mesh::MeshBuilder::withSphere(int stacks, int slices, float radius) {
        using namespace glm;
        using namespace std;

        if (name.length() == 0){
            std::stringstream ss;
            ss <<"SRE Sphere "<< stacks<<"-"<<slices<<"-"<<std::setprecision( 2 ) <<radius;
            name = ss.str();
        }

        size_t vertexCount = (size_t) ((stacks + 1) * (slices+1));
        vector<vec3> vertices{vertexCount};
        vector<vec3> normals{vertexCount};
        vector<vec4> uvs{vertexCount};

        int index = 0;
        // create vertices
        for (unsigned short j = 0; j <= stacks; j++) {
            double latitude1 = (glm::pi<double>() / stacks) * j - (glm::pi<double>() / 2);
            double sinLat1 = sin(latitude1);
            double cosLat1 = cos(latitude1);
            for (int i = 0; i <= slices; i++) {
                double longitude = ((glm::pi<double>() * 2) / slices) * i;
                double sinLong = sin(longitude);
                double cosLong = cos(longitude);
                dvec3 normalD{cosLong * cosLat1,
                            sinLat1,
                            sinLong * cosLat1};
                vec3 normal = (vec3)normalize(normalD);
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
            for (int i = 0; i <= slices; i++) {
                glm::u8vec2 offset [] = {
                        // first triangle
                        glm::u8vec2{i,j},
                        glm::u8vec2{(i+1)%(slices+1),j+1},
                        glm::u8vec2{(i+1)%(slices+1),j},

                        // second triangle
                        glm::u8vec2{i,j},
                        glm::u8vec2{i,j+1},
                        glm::u8vec2{(i+1)%(slices+1),j+1},

                };
                for (auto o : offset){
                    index = o[1] * (slices+1)  + o[0];
                    finalPosition.push_back(vertices[index]);
                    finalNormals.push_back(normals[index]);
                    finalUVs.push_back(uvs[index]);
                }

            }
        }

        withPositions(finalPosition);
        withNormals(finalNormals);
        withUVs(finalUVs);
        withMeshTopology(MeshTopology::Triangles);

        return *this;
    }

    Mesh::MeshBuilder &Mesh::MeshBuilder::withTorus(int segmentsC, int segmentsA, float radiusC, float radiusA) {
        using namespace glm;
        using namespace std;

        //  losely based on http://mathworld.wolfram.com/Torus.html
        if (name.length() == 0){
            std::stringstream ss;
            ss <<"SRE Torus "<< segmentsC<<"-"<<segmentsA<<"-"<<std::setprecision( 2 ) <<radiusC<<"-"<<radiusA;
            name = ss.str();
        }

        size_t vertexCount = (size_t) ((segmentsC + 1) * (segmentsA+1));
        vector<vec3> vertices{vertexCount};
        vector<vec3> normals{vertexCount};
        vector<vec4> uvs{vertexCount};

        int index = 0;

        // create vertices
        for (unsigned short j = 0; j <= segmentsC; j++) {
            for (int i = 0; i <= segmentsA; i++) {

                //vec3 normal = (vec3)normalize(normalD);
                //normals[index] = normal;
                float u = glm::two_pi<float>() * j/(float)segmentsC;
                float v = glm::two_pi<float>() * i/(float)segmentsA;
                glm::vec3 pos {
                        (radiusC+radiusA*cos(v))*cos(u),
                        (radiusC+radiusA*cos(v))*sin(u),
                        radiusA*sin(v)
                };
                glm::vec3 posOuter {
                        (radiusC+(radiusA*2)*cos(v))*cos(u),
                        (radiusC+(radiusA*2)*cos(v))*sin(u),
                        (radiusA*2)*sin(v)
                };
                uvs[index] = vec4{1 - j /(float) segmentsC, i /(float) segmentsA,0,0};
                vertices[index] = pos;
                normals[index] = glm::normalize(posOuter - pos);
                index++;
            }
        }
        vector<vec3> finalPosition;
        vector<vec3> finalNormals;
        vector<vec4> finalUVs;
        // create indices
        for (int j = 0; j < segmentsC; j++) {
            for (int i = 0; i <= segmentsA; i++) {
                glm::u8vec2 offset [] = {
                        // first triangle
                        glm::u8vec2{i,j},
                        glm::u8vec2{(i+1)%(segmentsA+1),j+1},
                        glm::u8vec2{(i+1)%(segmentsA+1),j},

                        // second triangle
                        glm::u8vec2{i,j},
                        glm::u8vec2{i,j+1},
                        glm::u8vec2{(i+1)%(segmentsA+1),j+1},

                };
                for (auto o : offset){
                    index = o[1] * (segmentsA+1)  + o[0];
                    finalPosition.push_back(vertices[index]);
                    finalNormals.push_back(normals[index]);
                    finalUVs.push_back(uvs[index]);
                }

            }
        }

        withPositions(finalPosition);
        withNormals(finalNormals);
        withUVs(finalUVs);
        withMeshTopology(MeshTopology::Triangles);

        return *this;
    }

    Mesh::MeshBuilder &Mesh::MeshBuilder::withCube(float length) {
        using namespace glm;
        using namespace std;

        if (name.length() == 0){
            std::stringstream ss;
            ss <<"SRE Cube "<<std::setprecision( 2 ) <<length;
            name = ss.str();
        }


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

        withPositions(positions);
        withNormals(normals);
        withUVs(uvs);
        withMeshTopology(MeshTopology::Triangles);

        return *this;
    }

    Mesh::MeshBuilder &Mesh::MeshBuilder::withQuad(float size) {
        if (name.length() == 0){
            std::stringstream ss;
            ss <<"SRE Quad "<<std::setprecision( 2 ) <<size;
            name = ss.str();
        }

        std::vector<glm::vec3> vertices({
                                                glm::vec3{ size,-size, 0},
                                                glm::vec3{ size, size, 0},
                                                glm::vec3{-size,-size, 0},
                                                glm::vec3{-size, size, 0}
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
        withPositions(vertices);
        withNormals(normals);
        withUVs(uvs);
        withIndices(indices);
        withMeshTopology(MeshTopology::Triangles);

        return *this;
    }

    Mesh::MeshBuilder &Mesh::MeshBuilder::withAttribute(std::string name, const std::vector<float> &values) {
        if (updateMesh != nullptr && attributesFloat.find(name) == attributesFloat.end()){
            LOG_ERROR("Cannot change mesh structure. %s dis not exist in the original mesh as a float.",name.c_str());
        } else {
            attributesFloat[name] = values;
        }
        return *this;
    }

    Mesh::MeshBuilder &Mesh::MeshBuilder::withAttribute(std::string name, const std::vector<glm::vec2> &values) {
        if (updateMesh != nullptr && attributesVec2.find(name) == attributesVec2.end()){
            LOG_ERROR("Cannot change mesh structure. %s dis not exist in the original mesh as a vec2.",name.c_str());
        } else {
            attributesVec2[name] = values;
        }
        return *this;
    }

    Mesh::MeshBuilder &Mesh::MeshBuilder::withAttribute(std::string name, const std::vector<glm::vec3> &values) {
        if (updateMesh != nullptr && attributesVec3.find(name) == attributesVec3.end()){
            LOG_ERROR("Cannot change mesh structure. %s dis not exist in the original mesh as a vec3.",name.c_str());
        } else {
            attributesVec3[name] = values;
        }
        return *this;
    }

    Mesh::MeshBuilder &Mesh::MeshBuilder::withAttribute(std::string name, const std::vector<glm::vec4> &values) {
        if (updateMesh != nullptr && attributesVec4.find(name) == attributesVec4.end()){
            LOG_ERROR("Cannot change mesh structure. %s dis not exist in the original mesh as a vec4.",name.c_str());
        } else {
            attributesVec4[name] = values;
        }
        return *this;
    }

    Mesh::MeshBuilder &Mesh::MeshBuilder::withAttribute(std::string name, const std::vector<glm::ivec4> &values) {
        if (updateMesh != nullptr && attributesIVec4.find(name) == attributesIVec4.end()){
            LOG_ERROR("Cannot change mesh structure. %s dis not exist in the original mesh as a ivec4.",name.c_str());
        } else {
            attributesIVec4[name] = values;
        }
        return *this;
    }

    Mesh::MeshBuilder &Mesh::MeshBuilder::withName(const std::string& name) {
        this->name = name;
        return *this;
    }
}
