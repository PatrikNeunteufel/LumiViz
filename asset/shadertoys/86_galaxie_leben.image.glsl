// 86 Galaxie-Leben, Image — Patrik Neunteufel (LumiViz, 2026). Eigenwerk.
// "Image": iChannel0 = Buffer A. Arme als Nebelschleier (.g), das Leben
// darauf als Sternen-Funkeln; Kern glüht warm.

void mainImage(out vec4 fragColor, in vec2 fragCoord)
{
    vec2 uv = fragCoord / iResolution.xy;
    vec2 zentriert = (uv * 2.0 - 1.0) * vec2(iResolution.x / iResolution.y, 1.0);
    vec4 z = texture(iChannel0, uv);
    vec3 col = vec3(0.004, 0.004, 0.012);
    col += z.g * vec3(0.10, 0.14, 0.3);                       // Nebel der Arme
    col += z.r * vec3(0.9, 0.95, 1.0);                        // lebende Sterne
    col += exp(-dot(zentriert, zentriert) * 6.0) * vec3(1.0, 0.8, 0.5) * 0.7; // Kern
    fragColor = vec4(col, 1.0);
}
