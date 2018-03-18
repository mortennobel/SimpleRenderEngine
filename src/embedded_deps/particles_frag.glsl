#version 330
out vec4 fragColor;
in mat3 vUVMat;
in vec3 uvSize;
in vec4 vColor;

#pragma include "global_uniforms_incl.glsl"

uniform sampler2D tex;

#pragma include "sre_utils_incl.glsl"

void main(void)
{
    vec2 uv = (vUVMat * vec3(gl_PointCoord,1.0)).xy;

    if (uv != clamp(uv, uvSize.xy, uvSize.xy + uvSize.zz)){
        discard;
    }
    vec4 c = vColor * toLinear(texture(tex, uv));
    fragColor = c;
    fragColor = toOutput(fragColor);
}