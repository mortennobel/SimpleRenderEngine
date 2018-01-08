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
out vec2 vUV;
out vec3 vLightDir[S_LIGHTS];
out vec3 vWsPos;

uniform mat4 g_model;
uniform mat4 g_view;
uniform mat4 g_projection;
uniform mat3 g_model_it;
uniform vec4 g_lightPosType[S_LIGHTS];

void main(void) {
    vec4 wsPos = g_model * vec4(position,1.0);
    vWsPos = wsPos.xyz;
    gl_Position = g_projection * g_view * wsPos;
#if defined(S_TANGENTS) && defined(S_NORMALMAP)
    vec3 wsNormal = normalize(g_model_it * normal);
    vec3 wsTangent = normalize(g_model_it * tangent.xyz);
    vec3 wsBitangent = cross(wsNormal, wsTangent) * tangent.w;
    vTBN = mat3(wsTangent, wsBitangent, wsNormal);
#else
    vNormal = normalize(g_model_it * normal);
#endif
    vUV = uv.xy;

    for (int i=0;i<S_LIGHTS;i++){
        bool isDirectional = g_lightPosType[i].w == 0.0;
        bool isPoint       = g_lightPosType[i].w == 1.0;
        vec3 lightDirection;
        float att = 1.0;
        if (isDirectional){
            vLightDir[i] = g_lightPosType[i].xyz;
        } else if (isPoint) {
            vLightDir[i] = normalize(g_lightPosType[i].xyz - vWsPos);
        }
    }
}