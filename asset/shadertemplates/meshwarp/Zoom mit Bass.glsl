// Leichter Sog zur Mitte, der Bass verstaerkt ihn.
vec2 warp(vec2 uv)
{
    return 0.5 + (uv - 0.5) * (0.99 - 0.03 * bass);
}
