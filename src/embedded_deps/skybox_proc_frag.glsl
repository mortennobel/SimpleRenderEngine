#version 330
out vec4 fragColor;
in vec3 vUV;

uniform vec4 skyColor;
uniform vec4 horizonColor;
uniform vec4 groundColor;
uniform float skyPow;
uniform float groundPow;

#pragma include "sre_utils_incl.glsl"

void main(void)
{
    fragColor = vUV.y>0.0 ?
        mix(horizonColor,skyColor, pow(vUV.y, skyPow)) :
        mix(horizonColor,groundColor, pow(-vUV.y, groundPow));
    fragColor = toOutput(fragColor);
}