// Per render-pass uniforms
#if __VERSION__ > 100
layout(std140) uniform g_global_uniforms {
#endif
#ifdef GL_ES
#ifdef GL_FRAGMENT_PRECISION_HIGH
uniform highp mat4 g_view;
uniform highp mat4 g_projection;
uniform highp vec4 g_viewport;
uniform highp vec4 g_cameraPos;
uniform highp vec4 g_ambientLight;
uniform highp vec4 g_lightColorRange[SI_LIGHTS];
uniform highp vec4 g_lightPosType[SI_LIGHTS];
#else
uniform mediump mat4 g_view;
uniform mediump mat4 g_projection;
uniform mediump vec4 g_viewport;
uniform mediump vec4 g_cameraPos;
uniform mediump vec4 g_ambientLight;
uniform mediump vec4 g_lightColorRange[SI_LIGHTS];
uniform mediump vec4 g_lightPosType[SI_LIGHTS];
#endif
#else
uniform mat4 g_view;
uniform mat4 g_projection;
uniform vec4 g_viewport;
uniform vec4 g_cameraPos;
uniform vec4 g_ambientLight;
uniform vec4 g_lightColorRange[SI_LIGHTS];
uniform vec4 g_lightPosType[SI_LIGHTS];
#endif
#if __VERSION__ > 100
};
#endif
#ifdef GL_ES
// Per draw call uniforms
#ifdef GL_FRAGMENT_PRECISION_HIGH
uniform highp mat4 g_model;
uniform highp mat3 g_model_it;
uniform highp mat3 g_model_view_it;
#else
uniform mediump mat4 g_model;
uniform mediump mat3 g_model_it;
uniform mediump mat3 g_model_view_it;
#endif
#else
uniform mat4 g_model;
uniform mat3 g_model_it;
uniform mat3 g_model_view_it;
#endif