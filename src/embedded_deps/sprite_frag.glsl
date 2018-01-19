#version 140
out vec4 fragColor;
in vec2 vUV;
in vec4 vColor;

uniform sampler2D tex;

void main(void)
{
    fragColor = vColor * texture(tex, vUV);
#ifndef SI_FRAMEBUFFER_SRGB
    float gamma = 2.2;
    fragColor = vec4(pow(fragColor.xyz,vec3(1.0/gamma)), fragColor.a); // gamma correction
#endif
}