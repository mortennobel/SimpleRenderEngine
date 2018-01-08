#version 140
in vec3 position;
in vec3 normal;
in vec4 uv;
out vec3 vNormal;
out vec2 vUV;
out vec3 vWsPos;

uniform mat4 g_model;
uniform mat4 g_view;
uniform mat4 g_projection;
uniform mat3 g_model_it;

void main(void) {
    vec4 wsPos = g_model * vec4(position,1.0);
    gl_Position = g_projection * g_view * wsPos;
    vNormal = normalize(g_model_it * normal);
    vUV = uv.xy;
    vWsPos = vWsPos.xyz;
}