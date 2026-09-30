// Rotation that gets stronger towards the centre.
vec2 warp(vec2 uv)
{
    vec2 d = uv - 0.5;
    float a = 0.02 * (1.0 - length(d) * 1.4);
    float c = cos(a);
    float s = sin(a);
    return 0.5 + vec2(c * d.x - s * d.y, s * d.x + c * d.y);
}
