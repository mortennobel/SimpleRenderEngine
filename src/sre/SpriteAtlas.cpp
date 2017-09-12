/*
 *  SimpleRenderEngine (https://github.com/mortennobel/SimpleRenderEngine)
 *
 *  Created by Morten Nobel-Jørgensen ( http://www.nobel-joergnesen.com/ )
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

std::shared_ptr<SpriteAtlas> SpriteAtlas::create(std::string jsonFile, std::string imageFile) {
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
    auto texture = Texture::create().withFile(imageFile).build();
    for (picojson::value& spriteElement : list){
        string name = spriteElement.get("filename").get<string>();
        // "frame": {"x":154,"y":86,"w":92,"h":44},
        // "pivot": {"x":0.5,"y":0.5}
        int x = (int)spriteElement.get("frame").get("x").get<double>();
        int y = (int)spriteElement.get("frame").get("y").get<double>();

        int w = (int)spriteElement.get("frame").get("w").get<double>();
        int h = (int)spriteElement.get("frame").get("h").get<double>();
        y = texture->getHeight()-y-h;
        float px = (float)spriteElement.get("pivot").get("x").get<double>();
        float py = (float)spriteElement.get("pivot").get("y").get<double>();
        Sprite sprite({x,y},{w,h},{px,py},texture.get());
        sprites.emplace(std::pair<std::string, Sprite>(name, std::move(sprite)));
    }
    return std::shared_ptr<SpriteAtlas>(new SpriteAtlas(std::move(sprites), texture, jsonFile));
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
        Sprite sprite(pos,size,pivot,texture.get());
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
}