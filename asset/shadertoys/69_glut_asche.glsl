// 69 Glut-Asche — Patrik Neunteufel (LumiViz, 2026). Eigenwerk.
// Shadertoy: iChannel0 = Music. GLOW: aufsteigende Glutpartikel + Hitzeflimmern.
//
// IDEE: Partikel steigen mit Seitenwind auf (endlos wrappend), glühen beim
// Aufsteigen aus; das ganze Bild flimmert leicht (Hitze-Verzerrung über
// sin-Versatz). Unten glimmt ein Glutbett, das mit dem Bass atmet.

// ---- STELLSCHRAUBEN --------------------------------------------------------
const int   PARTIKEL   = 70;
const float STEIGZEIT  = 4.0;   // Sekunden bis oben
const float WIND       = 0.15;
const float GROESSE    = 0.008;
const float FLIMMERN   = 0.004; // Hitze-Verzerrung
const float GLUTBETT   = 1.0;   // Helligkeit des Bodens
// ----------------------------------------------------------------------------

float h1(float n) { return fract(sin(n) * 43758.5453); }
void mainImage(out vec4 fragColor, in vec2 fragCoord)
{
    vec2 uv = (2.0 * fragCoord - iResolution.xy) / iResolution.y;
    float bass = texture(iChannel0, vec2(0.05, 0.25)).x;
    // Hitzeflimmern: Bildkoordinate leicht wellenverzerrt
    uv.x += FLIMMERN * sin(uv.y * 40.0 + iTime * 7.0);

    vec3 col = vec3(0.015, 0.008, 0.008);
    // Glutbett unten: FBM-artiges Glimmen aus zwei sin-Schichten
    float bett = smoothstep(-0.7, -1.0, uv.y) *
                 (0.6 + 0.4 * sin(uv.x * 13.0 + iTime * 1.5) *
                              sin(uv.x * 29.0 - iTime * 2.3));
    col += bett * GLUTBETT * (0.7 + 0.6 * bass) * vec3(1.0, 0.35, 0.08);

    for (int i = 0; i < PARTIKEL; ++i)
    {
        float fi = float(i);
        float t = fract(iTime / STEIGZEIT + h1(fi * 3.1));
        float x = (h1(fi * 7.7) * 2.0 - 1.0) +
                  WIND * sin(t * 6.0 + fi) * t;      // Seitenwind wächst mit Höhe
        float y = -1.0 + 2.2 * t;
        float glut = (1.0 - t) * (1.0 - t);          // ausglühen
        float d = length(uv - vec2(x, y));
        vec3 farbe = mix(vec3(1.0, 0.25, 0.05), vec3(1.0, 0.85, 0.45), glut);
        col += smoothstep(GROESSE * (0.5 + glut), 0.0, d) * farbe * glut *
               (0.6 + 0.8 * bass);
    }
    fragColor = vec4(col, 1.0);
}
