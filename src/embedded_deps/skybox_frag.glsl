#version 330
out vec4 fragColor;
in vec3 vUV;

uniform vec4 color;
uniform samplerCube tex;

#pragma include "sre_utils_incl.glsl"

void main(void)
{
    fragColor = color * toLinear(texture(tex, vUV));
    fragColor = toOutput(fragColor);
}