#version 330
in vec3 position;
out vec3 vUV;

uniform mat4 infProjection;
uniform mat4 g_model;

void main(void) {
    gl_Position = infProjection * (g_model * vec4(position, 1.0));
    vUV = position;
}