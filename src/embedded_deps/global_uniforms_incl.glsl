// Per render-pass uniforms
#if __VERSION__ > 100
layout(std140) uniform g_global_uniforms {
#endif
uniform mat4 g_view;
uniform mat4 g_projection;
uniform vec4 g_viewport;
uniform vec4 g_cameraPos;
uniform vec4 g_ambientLight;
uniform vec4 g_lightColorRange[SI_LIGHTS];
uniform vec4 g_lightPosType[SI_LIGHTS];

#if __VERSION__ > 100
};
#endif

// per draw call uniforms
uniform mat4 g_model;
uniform mat3 g_model_it;
uniform mat3 g_model_view_it;
