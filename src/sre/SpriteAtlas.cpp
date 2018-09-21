/*
 *  SimpleRenderEngine (https://github.com/mortennobel/SimpleRenderEngine)
 *
 *  Created by Morten Nobel-Jørgensen ( http://www.nobel-joergensen.com/ )
 *  License: MIT
 */

#include "sre/SpriteAtlas.hpp"
#include "sre/Renderer.hpp"
#include "sre/Sprite.hpp"
#include "sre/Texture.hpp"
#include "sre/Log.hpp"
#include "picojson.h"

#include <fstream>
#include <string>
#include <cstring>
#include <sstream>
#include <cerrno>
#include <iostream>
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/string_cast.hpp>

using namespace std;

namespace sre {
SpriteAtlas::SpriteAtlas(std::map<std::string, Sprite>&& sprites, std::shared_ptr<Texture> texture, std::string atlasName)
        :atlasName{atlasName}
{
    for (auto & s : sprites){
        this->sprites.insert({s.first,s.second});
    }
    this->texture = texture;
    sre::Renderer::instance->spriteAtlases.push_back(this);
}

std::shared_ptr<SpriteAtlas> SpriteAtlas::create(std::string jsonFile, std::shared_ptr<Texture> texture, bool flipAnchorY) {
    picojson::value v;
    std::ifstream t(jsonFile);
    if (!t){
        cerr << "SpriteAtlas json not found "<<jsonFile<< endl;
        return std::shared_ptr<SpriteAtlas>(nullptr);
    } else {
        t >> v;
    }
    std::string err = picojson::get_last_error();
    if (err != ""){
        cerr << err << endl;
        return std::shared_ptr<SpriteAtlas>(nullptr);
    }
    std::map<std::string, Sprite> sprites;
    picojson::array list = v.get("frames").get<picojson::array>();

    for (picojson::value& spriteElement : list){
        string name = spriteElement.get("filename").get<string>();
        // "frame": {"x":154,"y":86,"w":92,"h":44},
        // "pivot": {"x":0.5,"y":0.5}
        // "trimmed": true,
        // "rotated": false, // rotated sprites not supported
        // "spriteSourceSize": {"x":121,"y":94,"w":117,"h":131},
        // "sourceSize": {"w":256,"h":257},
        glm::ivec2 pos;
        glm::ivec2 size;
        glm::ivec2 sourcePos;
        glm::ivec2 sourceSize;

        glm::vec2 pivot;
        pos.x = (int)spriteElement.get("frame").get("x").get<double>();
        pos.y = (int)spriteElement.get("frame").get("y").get<double>();

        size.x = (int)spriteElement.get("frame").get("w").get<double>();
        size.y = (int)spriteElement.get("frame").get("h").get<double>();

        if (spriteElement.contains("rotated") && spriteElement.get("rotated").get<bool>()){
            LOG_ERROR("Rotated sprites not supported: %s", jsonFile.c_str());
        }

        if (spriteElement.contains("trimmed") && spriteElement.get("trimmed").get<bool>()){
            sourcePos.x = (int)spriteElement.get("spriteSourceSize").get("x").get<double>();
            sourcePos.y = (int)spriteElement.get("spriteSourceSize").get("y").get<double>();
            sourceSize.x = (int)spriteElement.get("sourceSize").get("w").get<double>();
            sourceSize.y = (int)spriteElement.get("sourceSize").get("h").get<double>();
            float spriteHeight = (int)spriteElement.get("spriteSourceSize").get("h").get<double>();
            sourcePos.y = sourceSize.y - sourcePos.y - spriteHeight;
        } else {
            sourcePos = {0,0};
            sourceSize = size;
        }
        pos.y = texture->getHeight()-pos.y-size.y;
        if (spriteElement.contains("pivot")){
            pivot.x = (float)spriteElement.get("pivot").get("x").get<double>();
            pivot.y = (float)spriteElement.get("pivot").get("y").get<double>();
        } else {
            pivot.x = 0.5f;
            pivot.y = 0.5f;
        }
        if (flipAnchorY){
            pivot.y = 1.0f - pivot.y;
        }
        Sprite sprite(pos,size,sourcePos,sourceSize,pivot,texture.get());
        sprites.emplace(std::pair<std::string, Sprite>(name, std::move(sprite)));
    }
    return std::shared_ptr<SpriteAtlas>(new SpriteAtlas(std::move(sprites), texture, jsonFile));
}

std::shared_ptr<SpriteAtlas> SpriteAtlas::create(std::string jsonFile, std::string imageFile,bool flipAnchorY) {
    auto texture = Texture::create().withFile(imageFile).build();
    return create(jsonFile, texture, flipAnchorY);
}

std::vector<std::string> SpriteAtlas::getNames() {
    std::vector<std::string> res;
    for (auto & e : sprites){
        res.push_back(e.first);
    }
    return std::move(res);
}

Sprite SpriteAtlas::get(std::string name) {
    if (sprites.find(name) == sprites.end()){
        LOG_WARNING("Cannot find sprite %s in spriteatlas",name.c_str());
        return {};
    }
    return sprites[name];
}

    std::shared_ptr<SpriteAtlas> SpriteAtlas::createSingleSprite(std::shared_ptr<Texture> texture, std::string name, glm::vec2 pivot, glm::ivec2 pos, glm::ivec2 size ) {

        std::map<std::string, Sprite> sprites;
        if (size == glm::ivec2{0,0}){
            size.x = texture->getWidth();
            size.y = texture->getHeight();
        }
        Sprite sprite(pos,size,{0,0},size,pivot,texture.get());
        sprites.emplace(std::pair<std::string, Sprite>(name, std::move(sprite)));

        return std::shared_ptr<SpriteAtlas>(new SpriteAtlas(std::move(sprites), texture, name+"_atlas"));
    }

    SpriteAtlas::~SpriteAtlas() {
        auto r = Renderer::instance;
        if (r){
            r->spriteAtlases.erase(std::remove(r->spriteAtlases.begin(), r->spriteAtlases.end(), this));
        }
    }

    std::string SpriteAtlas::getAtlasName() {
        return atlasName;
    }

    std::shared_ptr<Texture> SpriteAtlas::getTexture(){
        return texture;
    }


}

