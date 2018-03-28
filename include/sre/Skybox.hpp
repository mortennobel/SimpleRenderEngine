/*
 *  SimpleRenderEngine (https://github.com/mortennobel/SimpleRenderEngine)
 *
 *  Created by Morten Nobel-Jørgensen ( http://www.nobel-joergensen.com/ )
 *  License: MIT
 */

#pragma once

#include <memory>
#include "sre/Material.hpp"

namespace sre {
    class Skybox {
    public:
        static std::shared_ptr<Skybox> create();

        std::shared_ptr<Material> getMaterial();
        void setMaterial(std::shared_ptr<Material> material);
    private:
        Skybox();
        std::shared_ptr<Material> material;
        std::shared_ptr<Mesh> skyboxMesh;

        friend class RenderPass;
    };
}