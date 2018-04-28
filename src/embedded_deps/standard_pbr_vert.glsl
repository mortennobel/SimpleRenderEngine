#version 330
in vec3 position;
in vec3 normal;
in vec4 uv;
#if defined(S_TANGENTS) && defined(S_NORMALMAP)
in vec4 tangent;
out mat3 vTBN;
#else
out vec3 vNormal;
#endif
#ifdef S_VERTEX_COLOR
in vec4 vertex_color;
out vec4 vColor;
#endif
out vec2 vUV;
out vec3 vWsPos;
#ifdef S_SHADOW
uniform mat4 shadowViewProjection;
out vec4 vShadowmapCoord;
#endif

#pragma include "global_uniforms_incl.glsl"

#pragma include "normalmap_incl.glsl"

void main(void) {
    vec4 wsPos = g_model * vec4(position,1.0);
    vWsPos = wsPos.xyz;
    gl_Position = g_projection * g_view * wsPos;
#if defined(S_TANGENTS) && defined(S_NORMALMAP)
    vTBN = computeTBN(g_model_it, normal, tangent);
#else
    vNormal = normalize(g_model_it * normal);
#endif
    vUV = uv.xy;
#ifdef S_VERTEX_COLOR
    vColor = vertex_color;
#endif
#ifdef S_SHADOW
    vShadowmapCoord = shadowViewProjection * wsPos;
    vShadowmapCoord.xyz *= 0.5f;
    vShadowmapCoord.xyz += vShadowmapCoord.w * 0.5f;
#endif
}