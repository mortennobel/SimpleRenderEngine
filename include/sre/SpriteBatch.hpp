//
// Created by Morten Nobel-Jørgensen on 17/07/2017.
//

#pragma once

#include <vector>
#include "sre/Sprite.hpp"
#include "Mesh.hpp"

namespace sre{

class Shader;

class SpriteBatch {
public:
    class SpriteBatchBuilder {
    public:
        SpriteBatchBuilder& withShader(std::shared_ptr<Shader> shader);
        SpriteBatchBuilder& addSprite(const Sprite& sprite);
        std::shared_ptr<SpriteBatch> build();
    private:
        SpriteBatchBuilder();
        std::shared_ptr<Shader> shader;
        std::vector<Sprite> sprites;
        friend class SpriteBatch;
    };

    static SpriteBatchBuilder create();

private:
    SpriteBatch(std::shared_ptr<Shader> shader, std::vector<Sprite>&& sprites);
    std::vector<std::shared_ptr<Material>> materials;
    std::vector<std::shared_ptr<Mesh>> spriteMeshes;
    friend class RenderPass;
};

}
