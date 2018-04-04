/*
 *  SimpleRenderEngine (https://github.com/mortennobel/SimpleRenderEngine)
 *
 *  Created by Morten Nobel-Jørgensen ( http://www.nobel-joergensen.com/ )
 *  License: MIT
 */

#pragma once 
#include <vector>
#include "sre/Sprite.hpp"
#include "Mesh.hpp"
#include "Log.hpp"

/// Sprite batch batches multiple sprites into few draw calls. It is possible to reuse SpriteBatches over multiple
/// frames when the sprites are not changed (e.g. for rendering static level or background geometry).
///
/// Note that sprites are rendered in the following order (the sprite batch is sorted before rendering):
///    sprite.orderInBatch (high values will be rendered on top of sprites with lower values)
///    sprite.texture (textures will be batched together)
///    sprite.drawOrder (sprites added later will be rendered on top of other sprites with same texture and orderInBatch)
namespace sre{

class Shader;

class SpriteBatch {
public:
    class SpriteBatchBuilder {
    public:
        SpriteBatchBuilder& withShader(std::shared_ptr<Shader> shader);
        SpriteBatchBuilder& addSprite(Sprite sprite);
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
    SpriteBatch(std::shared_ptr<Shader> shader, std::vector<Sprite>& sprites);
    std::vector<std::shared_ptr<Material>> materials;
    std::vector<std::shared_ptr<Mesh>> spriteMeshes;
    friend class RenderPass;
};

    template<class InputIt>
    SpriteBatch::SpriteBatchBuilder &SpriteBatch::SpriteBatchBuilder::addSprites(InputIt first, InputIt last) {
        int size = sprites.size();
        auto start = sprites.insert(sprites.end(), first, last);
        while (start != sprites.end()){
            (*start).order.details.drawOrder = static_cast<uint16_t>(size);
            size ++;
            start ++;
        }

        if (size >= USHRT_MAX){
            LOG_ERROR("More than %i sprites in a batch ", USHRT_MAX);
            sprites.resize(USHRT_MAX);
            return *this;
        }
        return *this;
    }

}
