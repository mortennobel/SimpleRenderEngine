//
// Created by morten on 04/08/16.
//

#pragma once

#include <vector>

#include "glm/glm.hpp"

#include "SRE/impl/Export.hpp"
#include "SRE/MeshTopology.hpp"

namespace SRE {
    // The purpose of this class is to draw debug information into the scene.
    // The Debug class is using the current camera
    class DllExport Debug {
    public:
        static glm::vec4 getColor();
        static void setColor(glm::vec4 color);

        static void drawLine(glm::vec3 from, glm::vec3 to);
        static void drawLines(const std::vector<glm::vec3> &verts, MeshTopology meshTopology = MeshTopology::LineStrip);

    private:
        static glm::vec4 color;
        // For internal debugging of gl errors
        static void checkGLError();
    };
}

