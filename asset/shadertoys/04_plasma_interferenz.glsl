// 04 Plasma-Interferenz — Patrik Neunteufel (LumiViz, 2026). Eigenwerk.
// Shadertoy: iChannel0 = Music (512x2: y=0.25 FFT, y=0.75 Waveform).
//
// IDEE: Summe von vier Sinuswellen (zwei ebene, zwei radiale um wandernde
// Zentren); die Summe läuft durch eine Cosinus-Palette.

// ---- STELLSCHRAUBEN --------------------------------------------------------
const float TEMPO_GRUND   = 0.30;  // Grundtempo (war 0.6 — ruhiger)
const float TEMPO_BASS    = 0.35;  // Bass-Beschleunigung obendrauf (war 0.8)
const float FREQ_WELLE1   = 3.0;   // ebene Welle 1 (größer = feiner)
const float FREQ_WELLE2   = 2.5;   // ebene Welle 2
const float FREQ_RADIAL1  = 4.0;   // Radialwelle um Zentrum 1
const float FREQ_RADIAL2  = 3.0;   // Radialwelle um Zentrum 2 (Basis)
const float FREQ_MITTEN   = 4.0;   // + Mitten-Anteil auf Radialwelle 2
const float KONTRAST      = 2.4;   // Kantenschärfe der Farbübergänge (war 1.2)
const float SCHWARZ_SCHWELLE = 0.35; // unter dieser Helligkeit: SCHWARZ
const float SCHWARZ_KANTE = 0.02;  // Übergangsbreite der Schwarz-Kante
                                   // (0.02 = hart, 0.3 = weicher Verlauf)
const float PALETTEN_DRIFT = 0.25; // Eigenrotation der Farben
const float HELL_GRUND    = 0.6;   // Grundhelligkeit
const float HELL_BASS     = 0.4;   // Bass-Pumpen der Helligkeit
// ----------------------------------------------------------------------------

void mainImage(out vec4 fragColor, in vec2 fragCoord)
{
    vec2 uv = (2.0 * fragCoord - iResolution.xy) / iResolution.y;
    float bass = texture(iChannel0, vec2(0.05, 0.25)).x;
    float mid  = texture(iChannel0, vec2(0.30, 0.25)).x;
    float t = iTime * (TEMPO_GRUND + TEMPO_BASS * bass);

    float v = sin(uv.x * FREQ_WELLE1 + t)
            + sin((uv.y + uv.x) * FREQ_WELLE2 - t * 1.3)
            // Radialwellen: die Zentren wandern selbst auf Kreisbahnen
            + sin(length(uv - vec2(sin(t * 0.7), cos(t * 0.9))) * FREQ_RADIAL1)
            + sin(length(uv + vec2(cos(t * 0.5), sin(t * 0.4))) *
                  (FREQ_RADIAL2 + FREQ_MITTEN * mid));

    // KONTRAST staucht/streckt v vor der Palette: groß = harte Kanten
    vec3 col = 0.5 + 0.5 * cos(v * KONTRAST + iTime * PALETTEN_DRIFT +
                               vec3(0.0, 2.1, 4.2));
    // HARTE Schwarz-Kante: dunkle Bereiche schlagartig auf Schwarz schneiden
    // (kein Verlauf) — die Schwelle wandert mit v, dadurch entstehen scharf
    // umrandete Farbinseln auf Schwarz
    float helligkeit = dot(col, vec3(0.333));
    col *= smoothstep(SCHWARZ_SCHWELLE - SCHWARZ_KANTE,
                      SCHWARZ_SCHWELLE + SCHWARZ_KANTE, helligkeit);
    fragColor = vec4(col * (HELL_GRUND + HELL_BASS * bass), 1.0);
}
