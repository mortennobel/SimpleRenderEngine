#version 330
out vec4 fragColor;
in vec2 vUV;

uniform sampler2D tex;

#pragma include "sre_utils_incl.glsl"

void main(void)
{
    fragColor = toLinear(texture(tex, vUV));
    fragColor = toOutput(fragColor);
}