#pragma once

#include "sre/impl/Export.hpp"

namespace sre {
    // Render stats maintained by SimpleRenderEngine
    struct DllExport RenderStats {
        int frame;
        int meshCount;
        int meshBytes;
        int textureCount;
        int textureBytes;
        int shaderCount;
        int drawCalls;
        int stateChangesShader;
        int stateChangesMaterial;
        int stateChangesMesh;
    };
}
