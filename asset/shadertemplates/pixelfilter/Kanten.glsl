// Kanten aus den Nachbarpixeln (Sobel, vereinfacht).
vec4 farbe(vec2 uv, vec4 src)
{
    vec2 px = 1.0 / uResolution;
    float l = dot(texture(uTex, uv - vec2(px.x, 0.0)).rgb, vec3(0.333));
    float r = dot(texture(uTex, uv + vec2(px.x, 0.0)).rgb, vec3(0.333));
    float u = dot(texture(uTex, uv - vec2(0.0, px.y)).rgb, vec3(0.333));
    float o = dot(texture(uTex, uv + vec2(0.0, px.y)).rgb, vec3(0.333));
    float kante = clamp(length(vec2(r - l, o - u)) * 4.0, 0.0, 1.0);
    return vec4(vec3(kante), src.a);
}
