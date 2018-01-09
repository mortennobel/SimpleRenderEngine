#version 140
out vec4 fragColor;
in vec2 vUV;
in vec4 vColor;

uniform sampler2D tex;

void main(void)
{
    fragColor = vColor * texture(tex, vUV);
}