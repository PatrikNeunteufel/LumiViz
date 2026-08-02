// 10 Julia-Drift — Patrik Neunteufel (LumiViz, 2026). Eigenwerk.
// Shadertoy: iChannel0 = Music (512x2: y=0.25 FFT, y=0.75 Waveform).
//
// IDEE: Julia-Menge (z → z² + c); c wandert auf einer Kreisbahn (jede
// Position = andere Julia-Welt) und zuckt mit dem Bass.

// ---- STELLSCHRAUBEN --------------------------------------------------------
const float ZOOM         = 1.4;    // Bildausschnitt
const float BAHN_RADIUS  = 0.7885; // Kreisbahn von c (nahe Kardioide = spannend)
const float BAHN_TEMPO_X = 0.11;   // Wander-Tempo (größer = schnellere Weltwechsel)
const float BAHN_TEMPO_Y = 0.13;
const float BASS_ZUCKEN  = 0.08;   // Stärke der Bass-Störung auf c
const int   ITERATIONEN  = 96;     // Feinheit der Ränder (teurer)
const float FARB_ZYKLEN  = 1.2;    // Farbzyklen über den Iterations-Bereich
const float FARB_DRIFT   = 0.03;
// ----------------------------------------------------------------------------

void mainImage(out vec4 fragColor, in vec2 fragCoord)
{
    vec2 uv = (2.0 * fragCoord - iResolution.xy) / iResolution.y * ZOOM;
    float bass = texture(iChannel0, vec2(0.05, 0.25)).x;

    vec2 c = BAHN_RADIUS * vec2(cos(iTime * BAHN_TEMPO_X), sin(iTime * BAHN_TEMPO_Y));
    c += BASS_ZUCKEN * bass * vec2(cos(iTime * 1.7), sin(iTime * 2.3));

    // Iteration: z² komplex = (x²−y², 2xy)
    vec2 z = uv;
    float m = 0.0;
    for (int i = 0; i < ITERATIONEN; ++i)
    {
        z = vec2(z.x * z.x - z.y * z.y, 2.0 * z.x * z.y) + c;
        if (dot(z, z) > 16.0) break;  // explodiert
        m += 1.0;
    }
    float s = m / float(ITERATIONEN);  // 0 = sofort raus, 1 = Innenbereich
    vec3 col = 0.5 + 0.5 * cos(6.28318 * (s * FARB_ZYKLEN + iTime * FARB_DRIFT) +
                               vec3(0.0, 2.1, 4.2));
    col *= smoothstep(1.0, 0.85, s);  // Innenbereich schwarz
    fragColor = vec4(col, 1.0);
}
