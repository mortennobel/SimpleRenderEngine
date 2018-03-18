#version 140
out vec4 fragColor;
#if defined(S_TANGENTS) && defined(S_NORMALMAP)
in mat3 vTBN;
#else
in vec3 vNormal;
#endif
in vec2 vUV;
in vec3 vWsPos;
#ifdef S_NORMALMAP
uniform sampler2D normalTex;
uniform float normalScale;
#endif
#ifdef S_VERTEX_COLOR
in vec4 vColor;
#endif

uniform vec4 color;
uniform sampler2D tex;

#pragma include "global_uniforms_incl.glsl"
#pragma include "light_incl.glsl"
#pragma include "normalmap_incl.glsl"
#pragma include "sre_utils_incl.glsl"

void main()
{
    vec4 c = color * toLinear(texture(tex, vUV));
#ifdef S_VERTEX_COLOR
    c = c * vColor;
#endif
    vec3 normal = getNormal();
    vec3 specularLight = vec3(0.0,0.0,0.0);
    vec3 l = computeLightBlinnPhong(vWsPos, g_cameraPos.xyz, normal, specularLight);

    fragColor = c * vec4(l, 1.0) + vec4(specularLight,0);
    fragColor = toOutput(fragColor);
}