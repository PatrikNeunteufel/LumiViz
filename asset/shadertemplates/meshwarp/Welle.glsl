// Waagerechte Welle, die senkrecht durchs Bild laeuft.
vec2 warp(vec2 uv)
{
    uv.x += 0.005 * sin(uv.y * 20.0 + uTime * 2.0);
    return uv;
}
