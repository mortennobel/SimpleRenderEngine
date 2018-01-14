#version 140
out vec4 fragColor;
in vec2 vUV;
#ifdef S_VERTEX_COLOR
in vec4 vColor;
#endif

uniform vec4 color;
uniform sampler2D tex;

void main(void)
{
    fragColor = color * texture(tex, vUV);
#ifdef S_VERTEX_COLOR
    fragColor = fragColor * vColor;
#endif
}