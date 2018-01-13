#version 140
out vec4 fragColor;
#if defined(S_TANGENTS) && defined(S_NORMALMAP)
in mat3 vTBN;
#else
in vec3 vNormal;
#endif
in vec2 vUV;
in vec3 vWsPos;
uniform vec4 g_cameraPos;
#ifdef S_NORMALMAP
uniform sampler2D normalTex;
uniform float normalScale;
#endif


uniform vec4 color;
uniform sampler2D tex;

#pragma include "light_phong_incl.glsl"
#pragma include "normalmap_incl.glsl"

void main(void)
{
    vec4 c = color * texture(tex, vUV);
    vec3 normal = getNormal();
    vec3 l = computeLight(vWsPos, g_cameraPos.xyz, normal);

    fragColor = c * vec4(l, 1.0);
}