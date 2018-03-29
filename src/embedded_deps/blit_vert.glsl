#version 330
in vec3 position;
in vec3 normal;
in vec4 uv;
out vec2 vUV;

#pragma include "global_uniforms_incl.glsl"

void main(void) {
    gl_Position = g_model * vec4(position,1.0);
    vUV = uv.xy;
}