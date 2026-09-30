// Der Bass stoesst die Partikel von der Mitte weg.
vec2 kraft(vec2 pos, vec2 vel, float alter)
{
    vec2 d = pos - vec2(0.5);
    return d / (length(d) + 0.001) * bass * 2.0;
}
