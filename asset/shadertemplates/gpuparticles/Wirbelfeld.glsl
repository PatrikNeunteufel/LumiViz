// Kreisende Kraft um die Bildmitte.
vec2 kraft(vec2 pos, vec2 vel, float alter)
{
    vec2 d = pos - vec2(0.5);
    return vec2(-d.y, d.x) * 2.0;
}
