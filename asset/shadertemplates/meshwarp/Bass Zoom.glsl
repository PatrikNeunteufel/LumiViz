// A gentle pull towards the centre, stronger with the bass.
vec2 warp(vec2 uv)
{
    return 0.5 + (uv - 0.5) * (0.99 - 0.03 * bass);
}
