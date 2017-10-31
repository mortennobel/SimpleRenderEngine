/*
 *  SimpleRenderEngine (https://github.com/mortennobel/SimpleRenderEngine)
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
// The json-format of the sprite atlas is compatible with ﻿www.codeandweb.com/texturepacker (free) with the following
// settings (advanced mode):
// Data Format: JSON (Array)
// Layout - Size constraints: POT (Power of 2)
// Allow rotation: false
// Trim mode: None
//
// The json layout is as follows
// {"frames": [                                             // sprites must be defined as an array with the name frames
// {
// "filename": "background.png",                            // filename is the name which identifies object (dot not have to be actual filenames)
// "frame": {"x":0,"y":125,"w":100,"h":100},                // the location of the sprite in atlas (origin in upper left corner)
// "rotated": false,                                        // rotation must be false
// "trimmed": false,                                        // trimmed must be false
// "spriteSourceSize": {"x":0,"y":0,"w":100,"h":100},       // not used
// "sourceSize": {"w":100,"h":100},                         // not used
// "pivot": {"x":0.5,"y":0.5}                               // pivot point relative to frame. (Normalized values [0.0,1.0])
// },
// {                                                        // add as many sprites as needed
// "filename": "berry.png",
// "frame": {"x":25,"y":0,"w":25,"h":25},
// "rotated": false,
// "trimmed": false,
// "spriteSourceSize": {"x":0,"y":0,"w":25,"h":25},
// "sourceSize": {"w":25,"h":25},
// "pivot": {"x":0.5,"y":0.5}
// },
// }]
// }
//
namespace sre{
class SpriteAtlas {
public:
    ~SpriteAtlas();
    static std::shared_ptr<SpriteAtlas> create(std::string jsonFile,    // Create sprite atlas based on JSON file
                                               std::string imageFile,
                                               bool flipAnchorY = true);

    static std::shared_ptr<SpriteAtlas> createSingleSprite(std::shared_ptr<Texture> texture, // Create sprite atlas
                                                           std::string name = "sprite",      // (with single sprite)
                                                           glm::vec2 pivot = {0.5f,0.5f},    // using texture
                                                           glm::ivec2 pos = {0,0},
                                                           glm::ivec2 size = {0,0} );

    Sprite get(std::string name);                           // Return a copy of a Sprite object.

    std::vector<std::string> getNames();                    // Returns a list of sprite names in the SpriteAtlas container

    std::string getAtlasName();

    std::shared_ptr<Texture> getTexture();                  // Return sprite texture
private:
    SpriteAtlas(std::map<std::string, Sprite>&& sprites, std::shared_ptr<Texture> texture, std::string atlasName);
    std::string atlasName;
    std::map<std::string, Sprite> sprites;
    std::shared_ptr<Texture> texture;
};
}