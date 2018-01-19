#version 140
out vec4 fragColor;
in vec4 vUV;

void main(void)
{
    fragColor = vUV;
#ifndef SI_FRAMEBUFFER_SRGB
    float gamma = 2.2;
    fragColor = vec4(pow(fragColor.xyz,vec3(1.0/gamma)), fragColor.a); // gamma correction
#endif
}