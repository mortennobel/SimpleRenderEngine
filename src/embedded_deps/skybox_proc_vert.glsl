#version 330
in vec3 position;
out vec3 vUV;
out vec3 vLightDirection;

#pragma include "global_uniforms_incl.glsl"

void main(void) {
    vec4 eyespacePos = (g_view * vec4(position, 0.0));
    eyespacePos.w = 1.0;
    gl_Position = g_model * eyespacePos; // model matrix here contains the infinite projection
    vUV = position;
    vLightDirection = vec3(1.0);
    for (int i=0;i<SI_LIGHTS;i++){
        if (g_lightPosType[i].w == 1.0){
            vLightDirection = g_lightPosType[i].xyz;
            break;
        }
    }
}