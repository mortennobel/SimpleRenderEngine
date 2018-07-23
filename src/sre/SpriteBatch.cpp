/*
 *  SimpleRenderEngine (https://github.com/mortennobel/SimpleRenderEngine)
 *
 *  Created by Morten Nobel-Jørgensen ( http://www.nobel-joergensen.com/ )
 *  License: MIT
 */

#include "sre/SpriteBatch.hpp"
#include  <algorithm>
#include <sre/Sprite.hpp>
#include <sre/Log.hpp>
#include "sre/Texture.hpp"
#include "sre/Material.hpp"



namespace sre{

    SpriteBatch::SpriteBatchBuilder SpriteBatch::create() {
        return {};
    }

    SpriteBatch::SpriteBatch(std::shared_ptr<Shader> shader, std::vector<Sprite>& sprites)
    {
        std::sort(sprites.begin(), sprites.end(), [](const Sprite & a,const Sprite & b){
            return a.order.globalOrder < b.order.globalOrder;
        });
        std::vector<glm::vec3> vertices;
        std::vector<glm::vec4> colors;
        std::vector<glm::vec4> uvs;
        std::vector<uint32_t> indices;
        sre::Texture* lastTexture = nullptr;

        auto pushCurrentMesh = [&](){
            spriteMeshes.push_back(Mesh::create()
                                           .withName(std::string("DynamicSpriteBatch")+std::to_string(spriteMeshes.size()))
                                           .withPositions(vertices)
                                           .withUVs(uvs)
                                           .withIndices(indices)
                                           .withAttribute("vertex_color",colors)
                                           .build());
            auto mat = shader->createMaterial();
            mat->setTexture(lastTexture->shared_from_this());
            materials.push_back(mat);
        };

        // create meshes
        for (auto & s : sprites){
            if (lastTexture && lastTexture != s.texture){
                pushCurrentMesh();
                vertices.clear();
                colors.clear();
                uvs.clear();
                indices.clear();
            }
            lastTexture = s.texture;

            auto corners = s.getTrimmedCorners();
            auto cornerUvs = s.getUVs();

            uint16_t idx = (uint16_t)vertices.size();
            indices.push_back(idx);
            indices.push_back(idx+1);
            indices.push_back(idx+2);
            indices.push_back(idx);
            indices.push_back(idx+2);
            indices.push_back(idx+3);

            for (int i=0;i<4;i++){
                vertices.push_back({corners[i],0});
                uvs.push_back({cornerUvs[i],0,0});
                colors.push_back(s.color);
            }
        }
        if (vertices.size()>0){
            pushCurrentMesh();
        }

    }

    SpriteBatch::SpriteBatchBuilder::SpriteBatchBuilder() {
        shader = Shader::getUnlitSprite();
    }

    SpriteBatch::SpriteBatchBuilder &SpriteBatch::SpriteBatchBuilder::withShader(std::shared_ptr<Shader> shader) {
        this->shader = std::move(shader);
        return *this;
    }

    SpriteBatch::SpriteBatchBuilder &SpriteBatch::SpriteBatchBuilder::addSprite(Sprite sprite) {
        size_t size = sprites.size();
        if (size +1 >= std::numeric_limits<uint16_t>::max()){
            LOG_ERROR("More than %i sprites in a batch ",std::numeric_limits<uint16_t>::max());
            return *this;
        }
        sprite.order.details.drawOrder = static_cast<uint16_t>(size);
        sprites.push_back(std::move(sprite));
        return *this;
    }

    std::shared_ptr<SpriteBatch> SpriteBatch::SpriteBatchBuilder::build() {
        return std::shared_ptr<SpriteBatch>{new SpriteBatch(shader, sprites)};
    }

}