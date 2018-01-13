uniform vec3 g_ambientLight;
uniform vec4 g_lightPosType[SI_LIGHTS];
uniform vec4 g_lightColorRange[SI_LIGHTS];
uniform float specularity;

vec3 computeLight(vec3 wsPos, vec3 wsCameraPos, vec3 normal){
    vec3 lightColor = vec3(0.0,0.0,0.0);
    for (int i=0;i<SI_LIGHTS;i++){
        bool isDirectional = g_lightPosType[i].w == 0.0;
        bool isPoint       = g_lightPosType[i].w == 1.0;
        vec3 lightDirection;
        float att = 1.0;
        if (isDirectional){
            lightDirection = g_lightPosType[i].xyz;
        } else if (isPoint) {
            vec3 lightVector = g_lightPosType[i].xyz - wsPos;
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
            vec3 H = normalize(lightDirection - normalize(wsPos - wsCameraPos));
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