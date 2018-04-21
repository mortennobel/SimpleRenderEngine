#ifdef SI_VERTEX
mat3 computeTBN(mat3 g_model_it, vec3 normal, vec4 tangent){
    vec3 wsNormal = normalize(g_model_it * normal);
    vec3 wsTangent = normalize(g_model_it * tangent.xyz);
    vec3 wsBitangent = cross(wsNormal, wsTangent) * tangent.w;
    return mat3(wsTangent, wsBitangent, wsNormal);
}
#endif

#ifdef SI_FRAGMENT

// Find the normal for this fragment, pulling either from a predefined normal map
// or from the interpolated mesh normal and tangent attributes.
vec3 getNormal()
{
#ifdef S_NORMALMAP
    // Retrieve the tangent space matrix
#ifndef S_TANGENTS
    vec3 pos_dx = dFdx(vWsPos);
    vec3 pos_dy = dFdy(vWsPos);
    vec3 tex_dx = dFdx(vec3(vUV, 0.0));
    vec3 tex_dy = dFdy(vec3(vUV, 0.0));
    vec3 t = (tex_dy.t * pos_dx - tex_dx.t * pos_dy) / (tex_dx.s * tex_dy.t - tex_dy.s * tex_dx.t);

    vec3 ng = normalize(vNormal);

    t = normalize(t - ng * dot(ng, t));
    vec3 b = normalize(cross(ng, t));
    mat3 tbn = mat3(t, b, ng);
#else // S_TANGENTS
    mat3 tbn = vTBN;
#endif

    vec3 n = texture(normalTex, vUV).rgb;
    n = normalize(tbn * ((2.0 * n - 1.0) * vec3(normalScale, normalScale, 1.0)));
#else
    vec3 n = normalize(vNormal);
#endif

#ifdef S_TWO_SIDED
    if (!gl_FrontFacing){
        return -n;
    }
#endif
    return n;
}
#endif