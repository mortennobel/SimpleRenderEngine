#if __VERSION__ > 100
//layout(std140) uniform g_global_uniforms {
#endif
#ifdef GL_ES
uniform precision highp mat4 g_view;
uniform precision highp mat4 g_projection;
uniform precision highp vec4 g_viewport;
uniform precision highp vec4 g_cameraPos;
uniform precision highp vec4 g_ambientLight;
uniform precision highp vec4 g_lightColorRange[SI_LIGHTS];
uniform precision highp vec4 g_lightPosType[SI_LIGHTS];
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
//};
#endif