// Young particles rise, old ones sink again (alter = age 0..1).
vec2 kraft(vec2 pos, vec2 vel, float alter)
{
    return vec2(0.0, 0.6 * (0.5 - alter));
}
