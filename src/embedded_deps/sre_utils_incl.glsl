vec4 toLinear(vec4 col){
#ifndef SI_TEX_SAMPLER_SRGB
    return vec4 (
        col.xyz = pow(col.xyz),
        col.w
    );
#else
    return col;
#endif
}

vec4 toOutput(vec4 colorLinear){
#ifndef SI_FRAMEBUFFER_SRGB
    float gamma = 2.2;
    return vec4(pow(colorLinear.xyz,vec3(1.0/gamma)), colorLinear.a); // gamma correction
#else
    return colorLinear;
#endif
}

vec4 toOutput(vec3 colorLinear, float alpha){
#ifndef SI_FRAMEBUFFER_SRGB
    float gamma = 2.2;
    return vec4(pow(colorLinear,vec3(1.0/gamma)), alpha); // gamma correction
#else
    return vec4(colorLinear, alpha);                      // pass through
#endif
}
