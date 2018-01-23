/*
 *  SimpleRenderEngine (https://github.com/mortennobel/SimpleRenderEngine)
 *
 *  Created by Morten Nobel-Jørgensen ( http://www.nobel-joergnesen.com/ )
 *  License: MIT
 */

#pragma once

#include <glm/glm.hpp>

namespace sre {

    // The color class represent a sRGBA color, (this means that RGB is in gamma space, whereas alpha is in linear space)
    class Color {
    public:
        Color() = default;
        Color(float r, float g, float b, float a = 1.0f);

        float& operator[] (int index);

        glm::vec4 toLinear();             // Return color values in linear space
        void setFromLinear(glm::vec4 linear); // Set sRGBA values from linear space

        float r = 0;
        float g = 0;
        float b = 0;
        float a = 1;
    };
}