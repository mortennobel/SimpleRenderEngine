#pragma once


#include "SRE/impl/Export.hpp"

namespace SRE {
    /**
     * Mesh topology used to define the kind of mesh
     */
    enum class MeshTopology {
        Points = 0x0000,
        Lines = 0x0001,
        LineStrip = 0x0003,
        Triangles = 0x0004,
        TriangleStrip = 0x0005,
        TriangleFan = 0x0006
    };
}