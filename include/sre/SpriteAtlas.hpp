/*
 *  SimpleRenderEngine
 *
 *  Created by Morten Nobel-Jørgensen ( http://www.nobel-joergnesen.com/ )
 *  License: MIT
 */

#pragma once

#include <vector>
#include <string>
#include <map>
#include "sre/Sprite.hpp"

//
// Sprite atlases owns sprite definitions using a single texture.
// When the sprite is rendered its sprite atlas must still exist.
//
namespace sre{
class SpriteAtlas {
public:
    ~SpriteAtlas();
    static std::shared_ptr<SpriteAtlas> create(std::string jsonFile, std::string imageFile);
                                                            // Create sprite atlas based on JSON file
    static std::shared_ptr<SpriteAtlas> createSingleSprite(std::shared_ptr<Texture> texture, std::string name = "sprite", glm::vec2 pivot = {0.5f,0.5f}, glm::ivec2 pos = {0,0},glm::ivec2 size = {0,0} );
                                                            // Create sprite atlas (with single sprite) using texture
    Sprite get(std::string name);                           // Return a copy of a Sprite object.

    std::vector<std::string> getNames();                    // Returns a list of sprite names in the SpriteAtlas container

    std::string getAtlasName();
private:
    SpriteAtlas(std::map<std::string, Sprite>&& sprites, std::shared_ptr<Texture> texture, std::string atlasName);
    std::string atlasName;
    std::map<std::string, Sprite> sprites;
    std::shared_ptr<Texture> texture;
};
}