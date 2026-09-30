// Few colour steps per channel — poster look.
vec4 farbe(vec2 uv, vec4 src)
{
    const float steps = 5.0;
    return vec4(floor(src.rgb * steps + 0.5) / steps, src.a);
}
