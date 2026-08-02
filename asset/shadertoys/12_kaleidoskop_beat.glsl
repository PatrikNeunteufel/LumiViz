// 12 Kaleidoskop-Beat — Patrik Neunteufel (LumiViz, 2026). Eigenwerk.
// Shadertoy: iChannel0 = Music (512x2: y=0.25 FFT, y=0.75 Waveform).
//
// IDEE: Winkel in Segmente falten (mod + abs) = Spiegel-Kaleidoskop; ein
// Wellenmuster füllt die gefalteten Koordinaten. Bass schaltet die
// Segmentzahl, das Spektrum färbt die Radien.

// ---- STELLSCHRAUBEN --------------------------------------------------------
const float SEGMENTE_BASIS = 6.0;  // Spiegelzahl ohne Bass
const float SEGMENTE_BASS  = 6.0;  // Bass-Stufen: floor(bass*…)*2 dazu
const float MUSTER_FREQ1   = 9.0;  // Frequenzen des Innenmusters
const float MUSTER_FREQ2   = 7.0;
const float MUSTER_FREQ3   = 5.0;
const float MUSTER_TEMPO   = 1.4;  // Tempo der ersten Welle
const float FARB_KONTRAST  = 1.5;  // Härte der Farbwechsel
const float HELL_GRUND     = 0.35;
const float HELL_AUDIO     = 1.3;
// ----------------------------------------------------------------------------

void mainImage(out vec4 fragColor, in vec2 fragCoord)
{
    vec2 uv = (2.0 * fragCoord - iResolution.xy) / iResolution.y;
    float bass = texture(iChannel0, vec2(0.05, 0.25)).x;
    float seg = SEGMENTE_BASIS + floor(bass * SEGMENTE_BASS) * 2.0;
    float a = atan(uv.y, uv.x);
    float r = length(uv);
    // DIE Kaleidoskop-Zeile: Winkel in ein Segment falten und spiegeln
    a = abs(mod(a, 6.28318 / seg) - 3.14159 / seg);
    vec2 p = vec2(cos(a), sin(a)) * r;
    float v = sin(p.x * MUSTER_FREQ1 - iTime * MUSTER_TEMPO) +
              sin(p.y * MUSTER_FREQ2 + iTime) +
              sin((p.x + p.y) * MUSTER_FREQ3 + iTime * 0.6);
    float fft = texture(iChannel0, vec2(fract(r * 0.6), 0.25)).x;
    vec3 col = 0.5 + 0.5 * cos(v * FARB_KONTRAST + r * 3.0 + vec3(0.0, 2.1, 4.2));
    col *= HELL_GRUND + HELL_AUDIO * fft;
    col *= smoothstep(1.6, 0.4, r);  // außen ausblenden
    fragColor = vec4(col, 1.0);
}
