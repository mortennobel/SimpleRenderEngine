#version 330
out vec4 fragColor;
in vec3 vTangent;

#pragma include "sre_utils_incl.glsl"

void main(void)
{
    fragColor = vec4(vTangent*0.5+0.5,1.0);
    fragColor = toOutput(fragColor);
}