//
// Created by Morten Nobel-Jørgensen on 3/26/18.
//

#include <sre/Mesh.hpp>
#include "sre/Skybox.hpp"

namespace sre{
    std::shared_ptr<Skybox> Skybox::create(){
        return std::shared_ptr<Skybox>(new Skybox());
    }

    Skybox::Skybox(){
        // todo trim mesh
        skyboxMesh = Mesh::create().withSphere(16,32,-1).build();
        material = Shader::getSkyboxProcedural()->createMaterial();
        material->set("skyColor",Color(0,0,1));
        material->set("horizonColor",Color(1,1,1));
        material->set("groundColor",Color(0.31,0.197,0.026));
        material->set("skyPow",.5f);
        material->set("groundPow",.2f);
    }

    void Skybox::setMaterial(std::shared_ptr<Material> material) {
        this->material = std::move(material);
    }

    std::shared_ptr<Material> Skybox::getMaterial() {
        return this->material;
    }
}