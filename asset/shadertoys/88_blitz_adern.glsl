// 88 Blitz-Adern — Patrik Neunteufel (LumiViz, 2026). Eigenwerk.
// Shadertoy: iChannel0 = Music. KOMBI: KIFS-Kristall × Blitz — das
// Adergeflecht liegt dunkel da; in Zeitfenstern SCHIESST ein Stromstoß
// durchs Netz (die Adern flammen nacheinander auf, Tiefe für Tiefe).

// ---- STELLSCHRAUBEN --------------------------------------------------------
const int   FALTUNGEN   = 7;
const float SKALA       = 1.35;
const float WINKEL      = 0.62;
const float ADER_DICKE  = 0.012;
const float STOSS_RATE  = 0.7;   // Stromstöße pro Sekunde
const float BASS_ZUENDET = 0.35; // ab dieser Bass-Stärke voller Stoß
// ----------------------------------------------------------------------------

vec2 rot2(vec2 p, float a) { return vec2(p.x * cos(a) - p.y * sin(a),
                                         p.x * sin(a) + p.y * cos(a)); }
void mainImage(out vec4 fragColor, in vec2 fragCoord)
{
    vec2 uv = (2.0 * fragCoord - iResolution.xy) / iResolution.y * 1.3;
    float bass = texture(iChannel0, vec2(0.05, 0.25)).x;

    // Stromstoß-Fenster: `front` läuft je Fenster von 0..FALTUNGEN durch
    float fenster = iTime * STOSS_RATE;
    float front = fract(fenster) * float(FALTUNGEN + 2);
    float staerke = smoothstep(BASS_ZUENDET, BASS_ZUENDET + 0.15, bass) * 0.7 + 0.3;

    vec2 p = uv;
    float s = 1.0;
    vec3 col = vec3(0.008, 0.008, 0.02);
    for (int i = 0; i < FALTUNGEN; ++i)
    {
        p = abs(p);
        p = rot2(p, WINKEL + 0.03 * sin(iTime * 0.2));
        p = p * SKALA - vec2(0.35, 0.15) * SKALA;
        s *= SKALA;
        float d = abs(p.y) / s;
        float ader = smoothstep(ADER_DICKE, ADER_DICKE * 0.3, d);
        // Grundzustand: dunkles Geflecht
        col += ader * vec3(0.05, 0.06, 0.10);
        // Stromfront: die Tiefe |i − front| < 1 flammt blauweiß auf
        float blitz = exp(-pow(float(i) - front, 2.0) * 1.2) * staerke;
        col += ader * blitz * vec3(0.6, 0.75, 1.0) * 1.8;
        col += exp(-d * 40.0) * blitz * vec3(0.3, 0.4, 0.8) * 0.5;  // Korona
    }
    fragColor = vec4(col, 1.0);
}
