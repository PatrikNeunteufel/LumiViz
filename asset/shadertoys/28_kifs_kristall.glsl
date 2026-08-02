// 28 KIFS-Kristall — Patrik Neunteufel (LumiViz, 2026). Eigenwerk.
// Shadertoy: iChannel0 = Music. KIFS (Kaleidoscopic Iterated Function System).
//
// IDEE: Die Ebene wird wiederholt gefaltet (abs), rotiert und skaliert —
// aus einer einzigen Linie entsteht ein kristallines Adergeflecht. Der
// minimale Abstand zu einer Achse über alle Iterationen wird als glühende
// Ader gerendert. Mitten drehen den Faltwinkel: der Kristall morpht.

// ---- STELLSCHRAUBEN --------------------------------------------------------
const int   FALTUNGEN    = 7;     // Iterationstiefe
const float SKALA        = 1.35;  // Streckung je Faltung
const float WINKEL_GRUND = 0.60;  // Faltwinkel (DIE Form-Stellschraube)
const float WINKEL_MITTEN = 0.25; // Mitten verdrehen den Winkel (Morphing)
const float WINKEL_DRIFT = 0.05;  // langsame Eigenbewegung
const float ADER_DICKE   = 0.012; // Kernbreite der Adern
const float ADER_GLOW    = 0.08;  // Halo
const float ZOOM         = 1.3;
// ----------------------------------------------------------------------------

vec2 rot2(vec2 p, float a) { return vec2(p.x * cos(a) - p.y * sin(a),
                                         p.x * sin(a) + p.y * cos(a)); }
void mainImage(out vec4 fragColor, in vec2 fragCoord)
{
    vec2 uv = (2.0 * fragCoord - iResolution.xy) / iResolution.y * ZOOM;
    float mid = texture(iChannel0, vec2(0.30, 0.25)).x;
    float treb = texture(iChannel0, vec2(0.70, 0.25)).x;
    float winkel = WINKEL_GRUND + WINKEL_MITTEN * mid + iTime * WINKEL_DRIFT;

    vec2 p = uv;
    float dMin = 1e9;
    float tiefe = 0.0;  // Iteration, in der der Minimalabstand fiel (Farbe)
    float s = 1.0;
    for (int i = 0; i < FALTUNGEN; ++i)
    {
        p = abs(p);              // Faltung an beiden Achsen
        p = rot2(p, winkel);     // verdrehen
        p = p * SKALA - vec2(0.35, 0.15) * SKALA;  // strecken + versetzen
        s *= SKALA;
        // Abstand zur x-Achse dieser Iterationsstufe (im Original-Maßstab)
        float d = abs(p.y) / s;
        if (d < dMin) { dMin = d; tiefe = float(i); }
    }
    // Adern: harter Kern + Halo; Höhen lassen sie gleißen
    float kern = smoothstep(ADER_DICKE, ADER_DICKE * 0.4, dMin);
    float halo = smoothstep(ADER_GLOW, 0.0, dMin) * 0.35;
    vec3 farbe = 0.5 + 0.5 * cos(tiefe * 0.9 + iTime * 0.25 + vec3(0.0, 2.1, 4.2));
    vec3 col = (kern * (0.8 + 0.6 * treb) + halo) * farbe;
    col += vec3(0.01, 0.012, 0.02);  // Grundschimmer
    fragColor = vec4(col, 1.0);
}
