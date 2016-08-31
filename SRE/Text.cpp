//
// Created by morten on 07/08/16.
//

#include "Text.hpp"

#include "Mesh.hpp"
#include "glm/glm.hpp"
#include <vector>
#include <cstring>

namespace SRE {
Mesh *Text::createTextMesh(const char *text) {
    size_t size = strlen(text);

    std::vector<glm::vec3> vertices(size*6);
    std::vector<glm::vec3> normals(size*6);
    std::vector<glm::vec2> uvs(size*6);

    int sizeOfChar = 32;
    int charsPerRow = 16;
    for (int i=0;i<size;i++){
        int c = text[i];
        int posX = i*sizeOfChar;
        int posY = 0;
        vertices.push_back({posX, posY, 0});
        vertices.push_back({posX+sizeOfChar, posY, 0});
        vertices.push_back({posX+sizeOfChar, posY+sizeOfChar, 0});
        vertices.push_back({posX, posY, 0});
        vertices.push_back({posX+sizeOfChar, posY+sizeOfChar, 0});
        vertices.push_back({posX, posY+sizeOfChar, 0});

        for (int j=0;j<6;j++){
            normals.push_back({0,0,1});
        }

        float delta = 1.0f / charsPerRow;
        float u = (c%charsPerRow) / (float)charsPerRow;
        float v = 1.0f-delta-((c/charsPerRow) / (float)charsPerRow);
        uvs.push_back({u,v});
        uvs.push_back({u+delta,v});
        uvs.push_back({u+delta,v+delta});

        uvs.push_back({u,v});
        uvs.push_back({u+delta,v+delta});
        uvs.push_back({u,v+delta});
    }

    Mesh *res = new Mesh(vertices, normals, uvs, MeshTopology::Triangles);
    return res;
}

}
