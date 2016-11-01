#pragma once


#include "SRE/impl/Export.hpp"

namespace SRE {
    /**
     * Mesh topology used to define the kind of mesh
     */
    enum class MeshTopology {
        Points = 0x0000,
        Lines = 0x0001,
        Triangles = 0x0004
    };
}