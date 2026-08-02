// 17 Wellen-Wasser — Patrik Neunteufel (LumiViz, 2026). Eigenwerk.
// Shadertoy: iChannel0 = Music (512x2: y=0.25 FFT, y=0.75 Waveform).
//
// IDEE: Drei ebene Wellenzüge als Höhenfeld; Fake-Normale aus Nachbarhöhen
// + Lambert/Glanz-Beleuchtung. Mitten treiben Welle 3, Kamera driftet.

// ---- STELLSCHRAUBEN --------------------------------------------------------
const vec2  KAMERA_DRIFT = vec2(0.20, 0.10); // Drift-Richtung × Tempo
const float FREQ_1       = 3.0;   // Wellenfrequenzen (größer = kürzer)
const float FREQ_2       = 4.0;
const float FREQ_3       = 6.0;
const float TEMPO_1      = 1.8;   // Wellen-Tempi
const float TEMPO_2      = 1.4;
const float TEMPO_3      = 2.5;
const float STEILHEIT    = 0.35;  // n.z: kleiner = kontrastreichere Normale
const float GLANZ        = 48.0;  // Glanzpunkt-Schärfe (pow-Exponent)
// ----------------------------------------------------------------------------

float height(vec2 p, float t, float mid)
{
    // dot(p, richtung) = ebene Welle entlang dieser Richtung
    float h = 0.0;
    h += sin(dot(p, vec2(1.0, 0.3)) * FREQ_1 + t * TEMPO_1);
    h += sin(dot(p, vec2(-0.6, 1.0)) * FREQ_2 - t * TEMPO_2);
    h += (0.5 + mid) * sin(dot(p, vec2(0.3, -1.0)) * FREQ_3 + t * TEMPO_3);
    return h * 0.33;  // ~ -1..1
}
void mainImage(out vec4 fragColor, in vec2 fragCoord)
{
    vec2 uv = (2.0 * fragCoord - iResolution.xy) / iResolution.y * 2.0;
    uv += iTime * KAMERA_DRIFT;
    float mid = texture(iChannel0, vec2(0.30, 0.25)).x;
    float t = iTime;
    vec2 e = vec2(0.02, 0.0);
    float h = height(uv, t, mid);
    // Normale aus zentralen Differenzen
    vec3 n = normalize(vec3(height(uv - e.xy, t, mid) - height(uv + e.xy, t, mid),
                            height(uv - e.yx, t, mid) - height(uv + e.yx, t, mid),
                            STEILHEIT));
    vec3 l = normalize(vec3(0.5, 0.6, 0.7));  // Sonne
    float diff = max(dot(n, l), 0.0);
    float spec = pow(max(dot(reflect(-l, n), vec3(0.0, 0.0, 1.0)), 0.0), GLANZ);
    vec3 deep = vec3(0.0, 0.15, 0.30);     // Wellental
    vec3 shallow = vec3(0.1, 0.50, 0.60);  // Wellenberg
    vec3 col = mix(deep, shallow, 0.5 + 0.5 * h) * (0.4 + 0.6 * diff) +
               spec * (0.5 + mid);
    fragColor = vec4(col, 1.0);
}
