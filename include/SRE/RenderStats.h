#pragma once

#include "SRE/impl/Export.hpp"

namespace SRE {
    // Render stats maintained by SimpleRenderEngine
    struct DllExport RenderStats {
        int frame;
        int meshCount;
        int meshBytes;
        int textureCount;
        int textureBytes;
        int shaderCount;
        int drawCalls;
    };
}
