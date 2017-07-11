#pragma once

#include "sre/impl/Export.hpp"

namespace sre {
    // Render stats maintained by SimpleRenderEngine
    struct DllExport RenderStats {
        int frame;                                          // The frameid the render stat is captured
        int meshCount;                                      // Number of allocated meshes
        int meshBytes;                                      // Size of allocated meshes in bytes
        int textureCount;                                   // Number of allocated textures
        int textureBytes;                                   // Size of allocated textures in bytes
        int shaderCount;                                    // Number of allocated shaders
        int drawCalls;                                      // Number of drawCalls per frame
        int stateChangesShader;                             // Number of state changes for shaders
        int stateChangesMaterial;                           // Number of state changes for materials
        int stateChangesMesh;                               // Number of state changes for meshes
    };
}
