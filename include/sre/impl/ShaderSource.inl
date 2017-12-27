#include <map>
#include <utility>
#include <string>

std::map<std::string, std::string> builtInShaderSource  {
        std::make_pair<std::string,std::string>("light-phong.glsl",R"(
uniform vec3 g_ambientLight;
uniform vec4 g_lightPosType[SCENE_LIGHTS];
uniform vec4 g_lightColorRange[SCENE_LIGHTS];
uniform float specularity;

vec3 computeLight(){
    vec3 lightColor = vec3(0.0,0.0,0.0);
    vec3 normal = normalize(vNormal);
    for (int i=0;i<SCENE_LIGHTS;i++){
        bool isDirectional = g_lightPosType[i].w == 0.0;
        bool isPoint       = g_lightPosType[i].w == 1.0;
        vec3 lightDirection;
        float att = 1.0;
        if (isDirectional){
            lightDirection = g_lightPosType[i].xyz;
        } else if (isPoint) {
            vec3 lightVector = g_lightPosType[i].xyz - vEyePos;
            float lightVectorLength = length(lightVector);
            float lightRange = g_lightColorRange[i].w;
            lightDirection = lightVector / lightVectorLength; // compute normalized lightDirection (using length)
            if (lightRange <= 0.0){
                att = 1.0;
            } else if (lightVectorLength >= lightRange){
                att = 0.0;
            } else {
                att = pow(1.0 - lightVectorLength / lightRange,1.5); // non physical range based attenuation
            }
        } else {
            continue;
        }

        // diffuse light
        float thisDiffuse = max(0.0,dot(lightDirection, normal));
        if (thisDiffuse > 0.0){
            lightColor += (att * thisDiffuse) * g_lightColorRange[i].xyz;
        }

        // specular light
        if (specularity > 0.0){
            vec3 H = normalize(lightDirection - normalize(vEyePos));
            float nDotHV = dot(normal, H);
            if (nDotHV > 0.0){
                float pf = pow(nDotHV, specularity);
                lightColor += vec3(att * pf); // white specular highlights
            }
        }
    }
    lightColor = max(g_ambientLight.xyz, lightColor);

    return lightColor;
}
)"),
        std::make_pair<std::string,std::string>("standard_vert.glsl",R"(#version 140
in vec3 position;
in vec3 normal;
in vec4 uv;
out vec3 vNormal;
out vec2 vUV;
out vec3 vEyePos;

uniform mat4 g_model;
uniform mat4 g_view;
uniform mat4 g_projection;
uniform mat3 g_normal;

void main(void) {
    vec4 eyePos = g_view * g_model * vec4(position,1.0);
    gl_Position = g_projection * eyePos;
    vNormal = normalize(g_normal * normal);
    vUV = uv.xy;
    vEyePos = eyePos.xyz;
}
)"),
        std::make_pair<std::string,std::string>("standard_frag.glsl",R"(#version 140
out vec4 fragColor;
in vec3 vNormal;
in vec2 vUV;
in vec3 vEyePos;

uniform vec4 color;
uniform sampler2D tex;

#pragma include "light-phong.glsl"

void main(void)
{
    vec4 c = color * texture(tex, vUV);

    vec3 l = computeLight();

    fragColor = c * vec4(l, 1.0);
}
        )"),
        std::make_pair<std::string,std::string>("unlit_vert.glsl",R"(#version 140
in vec3 position;
in vec3 normal;
in vec4 uv;
out vec2 vUV;

uniform mat4 g_model;
uniform mat4 g_view;
uniform mat4 g_projection;

void main(void) {
    gl_Position = g_projection * g_view * g_model * vec4(position,1.0);
    vUV = uv.xy;
}
)"),
        std::make_pair<std::string,std::string>("unlit_frag.glsl", R"(#version 140
out vec4 fragColor;
in vec2 vUV;

uniform vec4 color;
uniform sampler2D tex;

void main(void)
{
    fragColor = color * texture(tex, vUV);
}
)"),
        std::make_pair<std::string,std::string>("sprite_vert.glsl",R"(#version 140
in vec3 position;
in vec4 uv;
in vec4 color;
out vec2 vUV;
out vec4 vColor;

uniform mat4 g_model;
uniform mat4 g_view;
uniform mat4 g_projection;

void main(void) {
    gl_Position = g_projection * g_view * g_model * vec4(position,1.0);
    vUV = uv.xy;
    vColor = color;
}
        )"),
        std::make_pair<std::string,std::string>("sprite_frag.glsl", R"(#version 140
out vec4 fragColor;
in vec2 vUV;
in vec4 vColor;

uniform sampler2D tex;

void main(void)
{
    fragColor = vColor * texture(tex, vUV);
}
        )"),
        std::make_pair<std::string,std::string>("particles_vert.glsl", R"(#version 140
in vec3 position;
in float particleSize;
in vec4 uv;
in vec4 color;
out mat3 vUVMat;
out vec4 vColor;
out vec3 uvSize;

uniform mat4 g_model;
uniform mat4 g_view;
uniform mat4 g_projection;
uniform vec4 g_viewport;

mat3 translate(vec2 p){
 return mat3(1.0,0.0,0.0,0.0,1.0,0.0,p.x,p.y,1.0);
}

mat3 rotate(float rad){
  float s = sin(rad);
  float c = cos(rad);
 return mat3(c,s,0.0,-s,c,0.0,0.0,0.0,1.0);
}

mat3 scale(float s){
  return mat3(s,0.0,0.0,0.0,s,0.0,0.0,0.0,1.0);
}

void main(void) {
    vec4 pos = vec4( position, 1.0);
    vec4 eyeSpacePos = g_view * g_model * pos;
    gl_Position = g_projection * eyeSpacePos;
    if (g_projection[2][3] != 0.0){ // if perspective projection
        gl_PointSize = (g_viewport.y / 600.0) * particleSize * 1.0 / -eyeSpacePos.z;
    } else {
        gl_PointSize = particleSize*(g_viewport.y / 600.0);
    }

    vUVMat = translate(uv.xy)*scale(uv.z) * translate(vec2(0.5,0.5))*rotate(uv.w) * translate(vec2(-0.5,-0.5));
    vColor = color;
    uvSize = uv.xyz;
}
)"),
        std::make_pair<std::string,std::string>("particles_frag.glsl", R"(#version 140
out vec4 fragColor;
in mat3 vUVMat;
in vec3 uvSize;
in vec4 vColor;
#ifdef GL_ES
uniform precision highp vec4 g_viewport;
else
uniform vec4 g_viewport;
#endif


uniform sampler2D tex;

void main(void)
{
    vec2 uv = (vUVMat * vec3(gl_PointCoord,1.0)).xy;

    if (uv != clamp(uv, uvSize.xy, uvSize.xy + uvSize.zz)){
        discard;
    }
    vec4 c = vColor * texture(tex, uv);
    fragColor = c;
}
)"),
        std::make_pair<std::string,std::string>("debug_uv_vert.glsl", R"(#version 140
in vec3 position;
in vec4 uv;
out vec2 vUV;

uniform mat4 g_model;
uniform mat4 g_view;
uniform mat4 g_projection;

void main(void) {
    gl_Position = g_projection * g_view * g_model * vec4(position,1.0);
    vUV = uv.xy;
}
)"),
        std::make_pair<std::string,std::string>("debug_uv_frag.glsl", R"(#version 140
in vec2 vUV;
out vec4 fragColor;

void main(void)
{
    fragColor = vec4(vUV,0.0,1.0);
}
)"),
        std::make_pair<std::string,std::string>("debug_normal_vert.glsl", R"(#version 140
in vec3 position;
in vec3 normal;
out vec3 vNormal;

uniform mat4 g_model;
uniform mat4 g_view;
uniform mat4 g_projection;
uniform mat3 g_normal;

void main(void) {
    gl_Position = g_projection * g_view * g_model * vec4(position,1.0);
    vNormal = g_normal * normal;
}
)"),
        std::make_pair<std::string,std::string>("debug_normal_frag.glsl", R"(#version 140
out vec4 fragColor;
in vec3 vNormal;

void main(void)
{
    fragColor = vec4(vNormal*0.5+0.5,1.0);
}
)")
};
