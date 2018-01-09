#version 140
in vec3 position;
in vec4 tangent;
out vec3 vTangent;

uniform mat4 g_model;
uniform mat4 g_view;
uniform mat4 g_projection;

void main(void) {
    gl_Position = g_projection * g_view * g_model * vec4(position,1.0);
    vTangent = vTangent.xyz;
}