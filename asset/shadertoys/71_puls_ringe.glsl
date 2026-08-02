// 71 Puls-Ringe — Patrik Neunteufel (LumiViz, 2026). Eigenwerk.
// Shadertoy: iChannel0 = Music. GLOW: expandierende Schockwellen.
//
// IDEE: In festen Zeitfenstern startet je ein Ring an gehashter Position
// und expandiert auslaufend (Radius ~ sqrt(t) wie eine echte Welle,
// Helligkeit ~ 1/Radius). Mehrere Fenster überlappen. Der Bass gebiert
// zusätzliche Ringe in der Mitte.

// ---- STELLSCHRAUBEN --------------------------------------------------------
const float RATE       = 0.8;   // Ringe pro Sekunde
const int   FENSTER    = 4;     // parallel lebende Ringe
const float LEBENSDAUER = 2.5;
const float TEMPO      = 0.55;  // Expansionstempo
const float RING_DICKE = 0.015;
const float BASS_RING  = 0.5;   // Bass-Schwelle für den Zentrumsring
// ----------------------------------------------------------------------------

float h1(float n) { return fract(sin(n) * 43758.5453); }
vec3 ring(vec2 uv, vec2 zentrum, float alter, float farbSaat)
{
    if (alter < 0.0 || alter > 1.0) return vec3(0.0);
    float radius = TEMPO * sqrt(alter * LEBENSDAUER);
    float d = abs(length(uv - zentrum) - radius);
    float hell = (1.0 - alter) * (1.0 - alter);
    vec3 farbe = 0.5 + 0.5 * cos(farbSaat * 6.28318 + vec3(0.0, 2.1, 4.2));
    return smoothstep(RING_DICKE, RING_DICKE * 0.2, d) * farbe * hell +
           exp(-d * 25.0) * farbe * hell * 0.4;  // Halo
}
void mainImage(out vec4 fragColor, in vec2 fragCoord)
{
    vec2 uv = (2.0 * fragCoord - iResolution.xy) / iResolution.y;
    float bass = texture(iChannel0, vec2(0.05, 0.25)).x;
    vec3 col = vec3(0.012, 0.01, 0.025);

    for (int k = 0; k < FENSTER; ++k)
    {
        float fk = float(k);
        float fenster = iTime * RATE / float(FENSTER) + fk / float(FENSTER);
        float saat = floor(fenster) * float(FENSTER) + fk;
        float alter = fract(fenster) * float(FENSTER) * RATE * LEBENSDAUER /
                      (RATE * LEBENSDAUER);  // 0..N — clampen unten
        vec2 z = vec2(h1(saat * 3.3) * 1.6 - 0.8, h1(saat * 7.7) * 1.2 - 0.6);
        col += ring(uv, z, fract(fenster) * float(FENSTER) / (RATE * LEBENSDAUER),
                    h1(saat * 11.1));
    }
    // Bass-Ring aus der Mitte (eigenes schnelles Fenster)
    if (bass > BASS_RING)
    {
        float fenster = iTime * 1.5;
        col += ring(uv, vec2(0.0), fract(fenster), 0.1) * (0.5 + bass);
    }
    fragColor = vec4(col, 1.0);
}
