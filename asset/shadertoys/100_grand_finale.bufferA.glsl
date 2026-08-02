// 100 Grand Finale, Buffer A — Patrik Neunteufel (LumiViz, 2026). Eigenwerk.
// "Buffer A": iChannel0 = SELBST, iChannel1 = Music. KOMBI-FINALE:
// Feuerwerk ÜBER einer Ozean-Spiegelung MIT Sternenhimmel — das Abschluss-
// Stück des 100er-Vorrats. Trail für die Raketen, Wellen spiegeln unten.

// ---- STELLSCHRAUBEN --------------------------------------------------------
const float TRAIL      = 0.90;
const float RAKETEN    = 0.8;    // Starts pro Sekunde
const int   FUNKEN     = 36;
const float HORIZONT   = -0.35;
const float BASS_EXTRA = 0.45;
// ----------------------------------------------------------------------------

float h1(float n) { return fract(sin(n) * 43758.5453); }
vec3 ereignis(vec2 uv, float seed, float t)
{
    vec3 col = vec3(0.0);
    vec2 start = vec2(-0.8 + 1.6 * h1(seed * 7.7), HORIZONT);
    vec2 ziel  = vec2(start.x + 0.2 * (h1(seed * 3.1) - 0.5),
                      0.25 + 0.4 * h1(seed * 9.3));
    float auf = 0.8;
    if (t < auf)
    {
        vec2 p = mix(start, ziel, t / auf);
        col += smoothstep(0.006, 0.0, length(uv - p)) * vec3(1.0, 0.85, 0.6);
    }
    else if (t < auf + 2.0)
    {
        float e = t - auf;
        float glut = exp(-e * 1.8);
        vec3 farbe = 0.5 + 0.5 * cos(seed * 41.0 + vec3(0.0, 2.1, 4.2));
        for (int i = 0; i < FUNKEN; ++i)
        {
            float fi = float(i);
            float w = 6.28318 * fi / float(FUNKEN) + h1(seed + fi) * 0.25;
            vec2 p = ziel + vec2(cos(w), sin(w)) * (0.3 + 0.15 * h1(seed * 13.0 + fi)) * e -
                     vec2(0.0, 0.12 * e * e);
            col += smoothstep(0.0045, 0.0, length(uv - p)) * farbe * glut;
        }
    }
    return col;
}
void mainImage(out vec4 fragColor, in vec2 fragCoord)
{
    vec2 uv = (2.0 * fragCoord - iResolution.xy) / iResolution.y;
    float bass = texture(iChannel1, vec2(0.05, 0.25)).x;
    vec3 col = texture(iChannel0, fragCoord / iResolution.xy).rgb * TRAIL;

    for (int k = 0; k < 2; ++k)
    {
        float fenster = iTime * RAKETEN * 0.5 + float(k) * 0.5;
        col += ereignis(uv, floor(fenster) * 2.0 + float(k),
                        fract(fenster) / (RAKETEN * 0.5));
    }
    if (bass > BASS_EXTRA)
    {
        float fenster = iTime * 1.4;
        col += ereignis(uv, floor(fenster) * 7.0 + 2.3, fract(fenster) / 1.4) * 1.2;
    }
    fragColor = vec4(min(col, vec3(2.5)), 1.0);
}
