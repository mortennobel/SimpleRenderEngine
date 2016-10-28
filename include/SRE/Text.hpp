#pragma once


#include "SRE/Export.hpp"
#include "SRE/CPPShim.hpp"


namespace SRE {
    class Mesh;

    // A simple factory class for creating monospaced font rendering
    class DllExport Text {
    public:
        // Creates a simple Text mesh with the origin in (0,0,0) and with the size (32,32,0) for each letter.
        // The width of a text mesh is 32*length
        DEPRECATED("Replaced by imgui") static Mesh *createTextMesh(const char* text);
    };
}