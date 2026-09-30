// A horizontal ripple running vertically through the image.
vec2 warp(vec2 uv)
{
    uv.x += 0.005 * sin(uv.y * 20.0 + uTime * 2.0);
    return uv;
}
