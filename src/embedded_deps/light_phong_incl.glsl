uniform vec3 g_ambientLight;
in vec4 vLightDir[SI_LIGHTS];
#ifdef GL_ES
uniform highp vec4 g_lightColorRange[SI_LIGHTS];
#else
uniform vec4 g_lightColorRange[SI_LIGHTS];
#endif
uniform vec4 specularity;

vec3 computeLight(vec3 wsPos, vec3 wsCameraPos, vec3 normal, out vec3 specularityOut){
    specularityOut = vec3(0.0, 0.0, 0.0);
    vec3 lightColor = vec3(0.0,0.0,0.0);
    for (int i=0;i<SI_LIGHTS;i++){
        float att = vLightDir[i].w;
        if (att <= 0.0){
            continue;
        }
        vec3 lightDirection = normalize(vLightDir[i].xyz);
        // diffuse light
        float thisDiffuse = max(0.0,dot(lightDirection, normal));
        if (thisDiffuse > 0.0){
            lightColor += (att * thisDiffuse) * g_lightColorRange[i].xyz;
        }

        // specular light
        if (specularity.a > 0.0){
            vec3 H = normalize(lightDirection + normalize(wsCameraPos - wsPos));
            float nDotHV = dot(normal, H);
            if (nDotHV > 0.0){
                float pf = pow(nDotHV, specularity.a);
                specularityOut += specularity.rgb * pf * att; // white specular highlights
            }
        }
    }
    lightColor = max(g_ambientLight.xyz, lightColor);

    return lightColor;
}