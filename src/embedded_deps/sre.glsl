vec4 gammaToLinear(vec4 colGamma){
    return vec4 (
        colGamma.xyz = pow(colGamma.xyz),
        colGamma.w
    );
}

vec4 linearToGamma(vec4 colorLinear){
    float gamma = 2.2;
    return vec4(pow(colorLinear.xyz,vec3(1.0/gamma)), colorLinear.a); // gamma correction
}

vec4 linearToGamma3(vec3 colorLinear, float alpha){
    float gamma = 2.2;
    return vec4(pow(colorLinear,vec3(1.0/gamma)), alpha); // gamma correction
}
