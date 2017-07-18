//
// Created by Morten Nobel-Jørgensen on 17/07/2017.
//

#pragma once

#include <string>
#include <map>
#include "sre/Sprite.hpp"

namespace sre{
class SpriteAtlas {
public:
    static std::shared_ptr<SpriteAtlas> create(std::string jsonFile, std::string imageFile);

    Sprite get(std::string name);

    std::vector<std::string> getNames();
private:
    SpriteAtlas(std::map<std::string, Sprite>&& sprites, std::shared_ptr<Texture> texture);
    std::map<std::string, Sprite> sprites;
    std::shared_ptr<Texture> texture;
};
}