// 42 Leben-Glut, Image — Patrik Neunteufel (LumiViz, 2026). Eigenwerk.
// "Image": iChannel0 = Buffer A. Lebende Zellen weißgelb, die Glut dahinter
// läuft die Feuer-Rampe hinunter (gelb → rot → dunkel).

// ---- STELLSCHRAUBEN --------------------------------------------------------
const float GLUT_HELL = 1.4;
// ----------------------------------------------------------------------------

void mainImage(out vec4 fragColor, in vec2 fragCoord)
{
    vec2 uv = fragCoord / iResolution.xy;
    vec4 z = texture(iChannel0, uv);
    float g = z.g;  // Glutstufe 0..1
    vec3 col = vec3(g * GLUT_HELL, g * g * GLUT_HELL * 0.7, g * g * g * 0.4);
    if (z.r > 0.5) col = vec3(1.0, 0.95, 0.75);  // lebende Front
    fragColor = vec4(col + vec3(0.015, 0.008, 0.01), 1.0);
}
