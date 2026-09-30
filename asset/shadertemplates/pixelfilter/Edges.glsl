// Edges from the neighbour pixels (simplified Sobel).
vec4 farbe(vec2 uv, vec4 src)
{
    vec2 px = 1.0 / uResolution;
    float l = dot(texture(uTex, uv - vec2(px.x, 0.0)).rgb, vec3(0.333));
    float r = dot(texture(uTex, uv + vec2(px.x, 0.0)).rgb, vec3(0.333));
    float d = dot(texture(uTex, uv - vec2(0.0, px.y)).rgb, vec3(0.333));
    float u = dot(texture(uTex, uv + vec2(0.0, px.y)).rgb, vec3(0.333));
    float edge = clamp(length(vec2(r - l, u - d)) * 4.0, 0.0, 1.0);
    return vec4(vec3(edge), src.a);
}
