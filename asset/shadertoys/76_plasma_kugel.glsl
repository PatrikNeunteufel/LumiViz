// 76 Plasma-Kugel — Patrik Neunteufel (LumiViz, 2026). Eigenwerk.
// Shadertoy: iChannel0 = Music. GLOW: Plasmaglobus mit Blitzfäden.
//
// IDEE: Fäden laufen vom Zentrum zur Glaskugel-Innenwand; jeder Faden ist
// ein Winkel-Pfad, den FBM-artiges Wackeln (sin-Summen je Radius) verbiegt.
// Die Fäden wandern langsam und ZUCKEN mit den Höhen; wo sie die Wand
// treffen, entsteht ein heller Fußpunkt.

// ---- STELLSCHRAUBEN --------------------------------------------------------
const int   FAEDEN     = 7;
const float KUGEL      = 0.85;
const float FADEN_GLOW = 0.0012;
const float WACKELN    = 0.35;
const float ZUCKEN_HOEHEN = 0.5;
const float WANDERN    = 0.25;   // Fußpunkte wandern um die Kugel
// ----------------------------------------------------------------------------

float h1(float n) { return fract(sin(n) * 43758.5453); }
void mainImage(out vec4 fragColor, in vec2 fragCoord)
{
    vec2 uv = (2.0 * fragCoord - iResolution.xy) / iResolution.y;
    float treb = texture(iChannel0, vec2(0.70, 0.25)).x;
    float r = length(uv);
    float a = atan(uv.y, uv.x);

    vec3 col = vec3(0.008, 0.006, 0.02);
    // Glaskugel: Rand + leichter Verlauf
    col += smoothstep(0.01, 0.0, abs(r - KUGEL)) * vec3(0.3, 0.4, 0.6);
    col += smoothstep(KUGEL, 0.0, r) * vec3(0.02, 0.01, 0.05);

    if (r < KUGEL)
    {
        for (int i = 0; i < FAEDEN; ++i)
        {
            float fi = float(i);
            // Zielwinkel des Fadens (wandert + zuckt)
            float ziel = fi * 6.28318 / float(FAEDEN) + iTime * WANDERN *
                         (h1(fi * 3.1) - 0.5) * 2.0 +
                         ZUCKEN_HOEHEN * treb * sin(iTime * 17.0 + fi * 5.0);
            // Fadenpfad: Winkel weicht mit dem Radius sinusförmig ab
            float pfad = ziel + WACKELN * (sin(r * 9.0 + iTime * 3.0 + fi * 2.0) *
                                           (0.3 + r)) * (1.0 - r / KUGEL);
            // Winkelabstand (wrap-korrekt über sin)
            float d = abs(sin((a - pfad) * 0.5)) * r * 2.0;
            float faden = FADEN_GLOW / (d * d + 0.0006) * 0.05;
            vec3 farbe = mix(vec3(0.5, 0.3, 1.0), vec3(0.9, 0.5, 1.0),
                             h1(fi * 7.7));
            col += faden * farbe * smoothstep(KUGEL, KUGEL * 0.3, r);
            // Fußpunkt an der Wand
            vec2 fuss = KUGEL * vec2(cos(pfad), sin(pfad));
            col += exp(-dot(uv - fuss, uv - fuss) * 300.0) *
                   vec3(0.8, 0.6, 1.0) * 0.8;
        }
        // Elektroden-Kern
        col += exp(-r * 9.0) * vec3(0.9, 0.7, 1.0) * 1.2;
    }
    fragColor = vec4(col, 1.0);
}
