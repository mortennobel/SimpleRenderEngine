#version 140
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
in vec4 color;
out vec4 vColor;
#endif
out vec2 vUV;
out vec4 vLightDir[SI_LIGHTS];
out vec3 vWsPos;

uniform mat4 g_model;
uniform mat4 g_view;
uniform mat4 g_projection;
uniform mat3 g_model_it;

#ifdef GL_ES
uniform highp vec4 g_lightColorRange[SI_LIGHTS];
uniform highp vec4 g_lightPosType[SI_LIGHTS];
#else
uniform vec4 g_lightColorRange[SI_LIGHTS];
uniform vec4 g_lightPosType[SI_LIGHTS];
#endif

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

    for (int i=0;i<SI_LIGHTS;i++){
        bool isDirectional = g_lightPosType[i].w == 0.0;
        bool isPoint       = g_lightPosType[i].w == 1.0;
        float att = 1.0;
        if (isDirectional){
            vLightDir[i] = vec4(g_lightPosType[i].xyz, 1.0);
        } else if (isPoint) {
            vec3 dir = g_lightPosType[i].xyz - vWsPos;
            float dirLength = length(dir);
            float att = 0.0;
            if (g_lightColorRange[i].w == 0.0){
                att = 1.0;
            } else if (dirLength < g_lightColorRange[i].w) {
                att = pow(1.0 - (dirLength / g_lightColorRange[i].w), 1.5); // non physical range based attenuation
            }
            vLightDir[i] = vec4(dir / dirLength, att);
        } else {
            vLightDir[i] = vec4(0.0, 0.0, 0.0, 0.0);
        }
    }
#ifdef S_VERTEX_COLOR
    vColor = color;
#endif
}