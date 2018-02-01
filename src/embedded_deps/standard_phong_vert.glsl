#version 140
in vec3 position;
in vec3 normal;
in vec4 uv;
out vec2 vUV;
#if defined(S_TANGENTS) && defined(S_NORMALMAP)
in vec4 tangent;
out mat3 vTBN;
#else
out vec3 vNormal;
#endif
out vec3 vWsPos;
#ifdef S_VERTEX_COLOR
in vec4 color;
out vec4 vColor;
#endif

uniform mat4 g_model;
uniform mat4 g_view;
uniform mat4 g_projection;
uniform mat3 g_model_it;

#pragma include "normalmap_incl.glsl"

void main(void) {
    vec4 wsPos = g_model * vec4(position,1.0);
    gl_Position = g_projection * g_view * wsPos;
#if defined(S_TANGENTS) && defined(S_NORMALMAP)
    vTBN = computeTBN(g_model_it, normal, tangent);
#else
    vNormal = normalize(g_model_it * normal);
#endif
    vUV = uv.xy;
    vWsPos = wsPos.xyz;

#ifdef S_VERTEX_COLOR
    vColor = color;
#endif
}