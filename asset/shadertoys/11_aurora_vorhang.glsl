// 11 Aurora-Vorhang — Patrik Neunteufel (LumiViz, 2026). Eigenwerk.
// Shadertoy: iChannel0 = Music (512x2: y=0.25 FFT, y=0.75 Waveform).
//
// IDEE: Leuchtbänder = Gauß-Glocken um Höhenlinien, die per Value-Noise
// wogen und seitlich ziehen. Mitten = Breite, Höhen = Helligkeit. + Sterne.

// ---- STELLSCHRAUBEN --------------------------------------------------------
const int   BAENDER       = 4;     // Anzahl Nordlicht-Bänder
const float WOGEN_TEMPO   = 0.45;  // vertikales Wogen
const float ZUG_TEMPO     = 0.30;  // seitlicher Zug
const float WOGEN_AMP     = 0.20;  // Höhenausschlag der Bänder
const float BREITE_GRUND  = 0.05;  // Bandbreite Grundwert
const float BREITE_FLACKER = 0.10; //  … Noise-Flackern
const float BREITE_MITTEN = 0.08;  //  … Mitten-Verbreiterung
const float HELL_GRUND    = 0.22;
const float HELL_HOEHEN   = 0.30;
const float STERN_DICHTE  = 0.9985; // näher an 1 = weniger Sterne
// ----------------------------------------------------------------------------

float n21(vec2 p) { return fract(sin(dot(p, vec2(41.3, 289.1))) * 43758.5453); }
// Value-Noise: Zufall an Gitterpunkten, dazwischen weich interpoliert
float vnoise(vec2 p)
{
    vec2 i = floor(p);
    vec2 f = fract(p);
    f = f * f * (3.0 - 2.0 * f);
    return mix(mix(n21(i), n21(i + vec2(1.0, 0.0)), f.x),
               mix(n21(i + vec2(0.0, 1.0)), n21(i + vec2(1.0, 1.0)), f.x), f.y);
}
void mainImage(out vec4 fragColor, in vec2 fragCoord)
{
    vec2 uv = fragCoord / iResolution.xy;
    float treb = texture(iChannel0, vec2(0.70, 0.25)).x;
    float mid  = texture(iChannel0, vec2(0.30, 0.25)).x;
    vec3 col = mix(vec3(0.00, 0.01, 0.04), vec3(0.01, 0.00, 0.06), uv.y);
    for (int i = 0; i < BAENDER; ++i)
    {
        float fi = float(i);
        // Höhenlinie: Grundhöhe + Noise-Wogen (x wandert = seitlicher Zug)
        float band = 0.50 + 0.12 * fi +
                     WOGEN_AMP * vnoise(vec2(uv.x * 2.0 - iTime * ZUG_TEMPO + fi * 5.0,
                                             iTime * WOGEN_TEMPO + fi));
        float w = BREITE_GRUND +
                  BREITE_FLACKER * vnoise(vec2(uv.x * 4.0 - iTime * 0.25, fi * 3.0)) +
                  BREITE_MITTEN * mid;
        float glow = exp(-pow((uv.y - band) / w, 2.0));  // Gauß-Glocke
        vec3 tint = mix(vec3(0.1, 0.9, 0.5), vec3(0.5, 0.2, 0.9), fi / 3.0);
        col += glow * tint * (HELL_GRUND + HELL_HOEHEN * treb);
    }
    // Sterne: seltenste Hash-Werte im 2x2-Raster leuchten
    float star = step(STERN_DICHTE, n21(floor(fragCoord / 2.0))) * (0.3 + 0.7 * treb);
    col += star;
    fragColor = vec4(col, 1.0);
}
