/*
 *  SimpleRenderEngine
 *
 *  Created by Morten Nobel-Jørgensen ( http://www.nobel-joergnesen.com/ )
 *  License: MIT
 */

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
        template< class InputIt >
        SpriteBatchBuilder& addSprites(InputIt first, InputIt last);
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

    template<class InputIt>
    SpriteBatch::SpriteBatchBuilder &SpriteBatch::SpriteBatchBuilder::addSprites(InputIt first, InputIt last) {
        sprites.insert(sprites.end(), first, last);
        return *this;
    }

}
