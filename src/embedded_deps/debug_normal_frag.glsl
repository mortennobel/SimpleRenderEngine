#version 140
out vec4 fragColor;
in vec3 vNormal;

#pragma include "sre_utils_incl.glsl"

void main(void)
{
    fragColor = vec4(vNormal*0.5+0.5,1.0);
    fragColor = toOutput(fragColor);
}