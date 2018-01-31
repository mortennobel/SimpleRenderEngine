#version 140
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