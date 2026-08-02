// 16 Laser-Scanner — Patrik Neunteufel (LumiViz, 2026). Eigenwerk.
// Shadertoy: iChannel0 = Music (512x2: y=0.25 FFT, y=0.75 Waveform).
//
// IDEE: Rotierende Strahlen: abs(sin(Winkelabstand)) wird auf dem Strahl 0,
// 1/(d²+eps) macht daraus einen Glow-Strich. Bass = Tempo, Höhen = Gleißen.

// ---- STELLSCHRAUBEN --------------------------------------------------------
const int   STRAHLEN     = 5;
const float TEMPO_GRUND  = 0.5;    // Rotationstempo
const float TEMPO_BASS   = 1.5;    //  … Bass-Zuschlag
const float SCHAERFE     = 0.002;  // kleiner = dünnere, grellere Strahlen
const float HELL_GRUND   = 0.0006; // Strahlhelligkeit
const float HELL_HOEHEN  = 0.0008;
const float MITTE_FREI   = 0.05;   // Strahlen erst ab diesem Radius
const float GLUT_STAERKE = 1.0;    // Bass-Glut im Zentrum
// ----------------------------------------------------------------------------

void mainImage(out vec4 fragColor, in vec2 fragCoord)
{
    vec2 uv = (2.0 * fragCoord - iResolution.xy) / iResolution.y;
    float bass = texture(iChannel0, vec2(0.05, 0.25)).x;
    float treb = texture(iChannel0, vec2(0.70, 0.25)).x;
    float a = atan(uv.y, uv.x);
    float r = length(uv);
    vec3 col = vec3(0.01, 0.01, 0.02);
    for (int i = 0; i < STRAHLEN; ++i)
    {
        float fi = float(i);
        // Winkelabstand zum Strahl i (Tempo gestaffelt, Start versetzt)
        float beam = a - iTime * (TEMPO_GRUND + TEMPO_BASS * bass) *
                             (fi * 0.3 + 0.4) - fi * 2.4;
        float d = abs(sin(beam));  // 0 exakt auf dem Strahl
        col += (HELL_GRUND + HELL_HOEHEN * treb) / (d * d + SCHAERFE) *
               (0.5 + 0.5 * cos(fi * 1.7 + vec3(0.0, 2.1, 4.2)));
    }
    col *= smoothstep(MITTE_FREI, 0.3, r) * smoothstep(1.8, 0.7, r);
    col += GLUT_STAERKE * bass * vec3(0.3, 0.05, 0.1) * smoothstep(0.25, 0.0, r);
    fragColor = vec4(col, 1.0);
}
