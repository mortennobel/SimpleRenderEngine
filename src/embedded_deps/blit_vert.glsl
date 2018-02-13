#version 140
in vec3 position;
in vec3 normal;
#ifdef S_VERTEX_COLOR
in vec4 color;
out vec4 vColor;
#endif
in vec4 uv;
out vec2 vUV;

uniform mat4 g_model;
uniform mat4 g_view;
uniform mat4 g_projection;

void main(void) {
    gl_Position = g_model * vec4(position,1.0);
    vUV = uv.xy;
}