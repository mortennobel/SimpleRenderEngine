/*
 *  SimpleRenderEngine (https://github.com/mortennobel/SimpleRenderEngine)
 *
 *  Created by Morten Nobel-Jørgensen ( http://www.nobel-joergensen.com/ )
 *  License: MIT
 */

#include "sre/Color.hpp"
#include "glm/gtc/color_space.hpp"

namespace sre {
    Color::Color(float r, float g, float b, float a)
    :r(r), g(g), b(b), a(a)
    {
    }

    float& Color::operator[] (int index){
        switch (index){
            case 0:
                return r;
            case 1:
                return g;
            case 2:
                return b;
            default:
                return a;
        }
    }

    glm::vec4 Color::toLinear(){
        glm::vec3 color{r,g,b};
        return glm::vec4(convertSRGBToLinear(color),a);

    }

    void Color::setFromLinear(glm::vec4 linear){
        glm::vec3 color{linear.x,linear.y,linear.z};
        color = convertLinearToSRGB(color);
        for (int i=0;i<3;i++){
            (*this)[i] = color[i];
        }
        a = linear.w;
    }

}
