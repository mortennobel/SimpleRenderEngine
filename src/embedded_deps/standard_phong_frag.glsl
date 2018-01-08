#version 140
out vec4 fragColor;
in vec3 vNormal;
in vec2 vUV;
in vec3 vWsPos;
uniform vec4 g_cameraPos;

uniform vec4 color;
uniform sampler2D tex;

#pragma include "light-phong.glsl"

void main(void)
{
    vec4 c = color * texture(tex, vUV);

    vec3 l = computeLight(vWsPos, g_cameraPos.xyz);

    fragColor = c * vec4(l, 1.0);
}