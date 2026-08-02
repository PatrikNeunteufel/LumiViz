// 45 Leben im Fraktal, Image — Patrik Neunteufel (LumiViz, 2026). Eigenwerk.
// "Image": iChannel0 = Buffer A. Die Schale (.g) schimmert dunkelblau,
// das Leben (.r) leuchtet warm darauf.

void mainImage(out vec4 fragColor, in vec2 fragCoord)
{
    vec2 uv = fragCoord / iResolution.xy;
    vec4 z = texture(iChannel0, uv);
    vec3 col = vec3(0.005, 0.005, 0.01);
    col += z.g * vec3(0.03, 0.06, 0.14);          // Petrischale
    col += z.r * vec3(1.0, 0.75, 0.35);           // Leben
    fragColor = vec4(col, 1.0);
}
