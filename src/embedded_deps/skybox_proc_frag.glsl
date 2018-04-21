#version 330
out vec4 fragColor;
in vec3 vUV;
in vec3 vLightDirection;

uniform vec4 skyColor;
uniform vec4 horizonColor;
uniform vec4 groundColor;
uniform float skyPow;
uniform float sunIntensity;
uniform float groundPow;

#pragma include "sre_utils_incl.glsl"

void main(void)
{
    if (vUV.y > 0.0){
        fragColor = mix(horizonColor,skyColor, pow(vUV.y, skyPow));
        fragColor.xyz += vec3(1.0)*pow(max(0.0,dot(vUV, vLightDirection)),2.0)*sunIntensity;
    } else {
        fragColor = mix(horizonColor,groundColor, pow(-vUV.y, groundPow));
    }
    fragColor = toOutput(fragColor);
}