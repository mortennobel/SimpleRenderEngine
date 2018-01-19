#version 140
out vec4 fragColor;
in vec3 vTangent;

void main(void)
{
    fragColor = vec4(vTangent*0.5+0.5,1.0);
#ifndef SI_FRAMEBUFFER_SRGB
    float gamma = 2.2;
    fragColor = vec4(pow(fragColor.xyz,vec3(1.0/gamma)), fragColor.a); // gamma correction
#endif
}