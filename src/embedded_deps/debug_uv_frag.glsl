#version 140
out vec4 fragColor;
in vec4 vUV;

void main(void)
{
    fragColor = vUV;
}