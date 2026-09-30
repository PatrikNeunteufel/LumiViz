// Wenige Farbstufen je Kanal — Plakat-Look.
vec4 farbe(vec2 uv, vec4 src)
{
    const float stufen = 5.0;
    return vec4(floor(src.rgb * stufen + 0.5) / stufen, src.a);
}
