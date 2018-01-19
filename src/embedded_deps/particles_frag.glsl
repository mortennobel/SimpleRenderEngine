#version 140
out vec4 fragColor;
in mat3 vUVMat;
in vec3 uvSize;
in vec4 vColor;
#ifdef GL_ES
uniform precision highp vec4 g_viewport;
else
uniform vec4 g_viewport;
#endif


uniform sampler2D tex;

void main(void)
{
    vec2 uv = (vUVMat * vec3(gl_PointCoord,1.0)).xy;

    if (uv != clamp(uv, uvSize.xy, uvSize.xy + uvSize.zz)){
        discard;
    }
    vec4 c = vColor * texture(tex, uv);
    fragColor = c;
#ifndef SI_FRAMEBUFFER_SRGB
    float gamma = 2.2;
    fragColor = vec4(pow(fragColor.xyz,vec3(1.0/gamma)), fragColor.a); // gamma correction
#endif

}