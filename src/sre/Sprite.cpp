/*
 *  SimpleRenderEngine (https://github.com/mortennobel/SimpleRenderEngine)
 *
 *  Created by Morten Nobel-Jørgensen ( http://www.nobel-joergnesen.com/ )
 *  License: MIT
 */

#include <iostream>
#include <glm/gtx/transform.hpp>
#include "sre/Sprite.hpp"
#include "sre/Texture.hpp"
#include "sre/Log.hpp"


sre::Sprite::Sprite(glm::ivec2 spritePos, glm::ivec2 spriteSize, glm::vec2 spriteAnchor, Texture* texture)
:spritePos(spritePos),spriteSize(spriteSize),spriteAnchor(spriteAnchor),texture(texture)
{
}

float sre::Sprite::getRotation() const {
    return rotation;
}

void sre::Sprite::setRotation(float rotation) {
    Sprite::rotation = rotation;
}

const glm::vec2 &sre::Sprite::getPosition() const {
    return position;
}

void sre::Sprite::setPosition(const glm::vec2 &position) {
    Sprite::position = position;
}

const glm::vec2 &sre::Sprite::getScale() const {
    return scale;
}

void sre::Sprite::setScale(const glm::vec2 &scale) {
    Sprite::scale = scale;
    if (scale.x < 0){
        Sprite::scale.x = 0;
        LOG_WARNING("Sprite.scale.x must be larger or equal to 0");
    }
    if (scale.y < 0){
        Sprite::scale.y = 0;
        LOG_WARNING("Sprite.scale.y must be larger or equal to 0");
    }
}

const glm::bvec2 &sre::Sprite::getFlip() const {
    return flip;
}

void sre::Sprite::setFlip(const glm::bvec2 &flip) {
    Sprite::flip = flip;
}

int sre::Sprite::getOrderInBatch() const {
    return orderInBatch;
}

void sre::Sprite::setOrderInBatch(int orderInBatch) {
    Sprite::orderInBatch = orderInBatch;
}

const glm::vec4 &sre::Sprite::getColor() const {
    return color;
}

void sre::Sprite::setColor(const glm::vec4 &color) {
    Sprite::color = color;
}

const glm::ivec2 &sre::Sprite::getSpritePos() const {
    return spritePos;
}

const glm::ivec2 &sre::Sprite::getSpriteSize() const {
    return spriteSize;
}

const glm::vec2 &sre::Sprite::getSpriteAnchor() const {
    return spriteAnchor;
}

sre::Sprite::Sprite()
:spritePos{0,0},
 spriteSize{0,0},
 spriteAnchor{0,0},
 texture{nullptr}
{
}

sre::Sprite::Sprite(const Sprite &s)
        :rotation(s.rotation),
         position(s.position),
         scale(s.scale),
         flip(s.flip),
         orderInBatch(s.orderInBatch),
         color(s.color),
         spritePos(s.spritePos),
         spriteSize(s.spriteSize),
         spriteAnchor(s.spriteAnchor),
         texture(s.texture)
{
}

std::array<glm::vec2, 4> sre::Sprite::getCorners() {
    float x0 = 0 - spriteAnchor.x * spriteSize.x;
    float x1 = spriteSize.x - spriteAnchor.x * spriteSize.x;
    float y0 = 0 - spriteAnchor.y * spriteSize.y;
    float y1 = spriteSize.y - spriteAnchor.y * spriteSize.y;

    std::array<glm::vec2, 4> res;
    res[0] = {x1,y0};
    res[1] = {x1,y1};
    res[2] = {x0,y1};
    res[3] = {x0,y0};

    // compute transformation
    glm::mat4 trs = glm::translate(glm::vec3{position,0.0}) * glm::rotate(glm::radians(rotation),glm::vec3{0,0,1}) * glm::scale(glm::vec3{scale,1.0});
    for (int i=0;i<4;i++){
        res[i] = (glm::vec2)(trs*glm::vec4(res[i],0.0,1.0));
    }

    return res;
}

std::array<glm::vec2, 4> sre::Sprite::getUVs() {
    std::array<glm::vec2, 4> res;
    float texWidth = texture->getWidth();
    float texHeight = texture->getHeight();

    float x0 = (spritePos.x)/texWidth;
    float x1 = (spritePos.x+spriteSize.x)/texWidth;
    float y0 = (spritePos.y)/texHeight;
    float y1 = (spritePos.y+spriteSize.y)/texHeight;

    if (flip.x){
        std::swap(x0,x1);
    }
    if (flip.y){
        std::swap(y0,y1);
    }

    res[0] = {x1,y0};
    res[1] = {x1,y1};
    res[2] = {x0,y1};
    res[3] = {x0,y0};

    return res;
}
