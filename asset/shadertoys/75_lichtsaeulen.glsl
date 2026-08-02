// 75 Lichtsäulen — Patrik Neunteufel (LumiViz, 2026). Eigenwerk.
// Shadertoy: iChannel0 = Music. GLOW: Spektrum als Säulen-Skyline aus Licht.
//
// IDEE: Vertikale Lichtsäulen (1/d²-Glow um Spaltenmitten), deren Höhe das
// FFT-Spektrum ist; Spitzen glühen heißer, ein Boden spiegelt weich.
// Wie Scheinwerfer einer nächtlichen Bühne — pure Glow-Ästhetik.

// ---- STELLSCHRAUBEN --------------------------------------------------------
const float SAEULEN    = 24.0;
const float GLOW       = 0.0006;
const float HOEHE      = 1.1;
const float ANHEBUNG   = 0.6;   // pow < 1 hebt leise Bins
const float SPIEGEL    = 0.35;  // Boden-Reflex
// ----------------------------------------------------------------------------

void mainImage(out vec4 fragColor, in vec2 fragCoord)
{
    vec2 uv = (2.0 * fragCoord - iResolution.xy) / iResolution.y;
    vec3 col = vec3(0.008, 0.008, 0.02);
    float boden = -0.55;

    for (int i = 0; i < int(SAEULEN); ++i)
    {
        float fi = (float(i) + 0.5) / SAEULEN;
        float x = (fi * 2.0 - 1.0) * 1.5;
        float fft = pow(texture(iChannel0, vec2(fi * 0.7 + 0.02, 0.25)).x, ANHEBUNG);
        float top = boden + fft * HOEHE;
        // Abstand zur Säulen-Strecke (vertikal, von boden bis top)
        vec2 p = vec2(x, clamp(uv.y, boden, top));
        float d = length(uv - p);
        vec3 farbe = 0.5 + 0.5 * cos(fi * 4.0 + iTime * 0.3 + vec3(0.0, 2.1, 4.2));
        col += GLOW / (d * d + 0.0005) * farbe * 0.12 * (0.3 + fft);
        // heiße Spitze
        float ds = length(uv - vec2(x, top));
        col += GLOW * 2.0 / (ds * ds + 0.0008) * vec3(1.0, 0.95, 0.9) * 0.08 * fft;
        // Bodenreflex: gespiegelte, gedämpfte Säule
        vec2 ps = vec2(x, clamp(uv.y, 2.0 * boden - top, boden));
        float dr = length(uv - ps);
        col += GLOW / (dr * dr + 0.001) * farbe * 0.12 * SPIEGEL * (0.3 + fft);
    }
    // Bodenlinie
    col += smoothstep(0.006, 0.0, abs(uv.y - boden)) * vec3(0.15, 0.15, 0.25);
    fragColor = vec4(col, 1.0);
}
