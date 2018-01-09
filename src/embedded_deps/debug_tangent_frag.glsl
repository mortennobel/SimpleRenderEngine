#version 140
out vec4 fragColor;
in vec3 vTangent;

void main(void)
{
    fragColor = vec4(vTangent*0.5+0.5,1.0);
}