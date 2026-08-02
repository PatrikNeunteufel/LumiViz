// 92 Magnet-Neuronen — Patrik Neunteufel (LumiViz, 2026). Eigenwerk.
// Shadertoy: iChannel0 = Music. KOMBI: Magnetfeld × Neuronen — DREI Pole
// bilden ein Feldliniennetz, auf dem NEURONALE Pulse feuern (Lauflichter
// die Linien entlang, Feuerrate aus den Höhen); die Pole sind die Somata.

// ---- STELLSCHRAUBEN --------------------------------------------------------
const float LINIEN     = 12.0;
const float LINIEN_DICKE = 0.09;
const float POL_TEMPO  = 0.3;
const float PULS_TEMPO = 2.5;
const float PULS_HOEHEN = 3.0;
// ----------------------------------------------------------------------------

void mainImage(out vec4 fragColor, in vec2 fragCoord)
{
    vec2 uv = (2.0 * fragCoord - iResolution.xy) / iResolution.y;
    float treb = texture(iChannel0, vec2(0.70, 0.25)).x;

    // drei kreisende Pole mit Ladungen +, −, +
    vec2 p1 = 0.55 * vec2(cos(iTime * POL_TEMPO), sin(iTime * POL_TEMPO));
    vec2 p2 = 0.55 * vec2(cos(iTime * POL_TEMPO + 2.094), sin(iTime * POL_TEMPO + 2.094));
    vec2 p3 = 0.55 * vec2(cos(iTime * POL_TEMPO + 4.189), sin(iTime * POL_TEMPO + 4.189));
    float d1 = length(uv - p1) + 1e-3;
    float d2 = length(uv - p2) + 1e-3;
    float d3 = length(uv - p3) + 1e-3;
    float pot = log(d2) - 0.5 * log(d1) - 0.5 * log(d3);  // Potential

    float band = abs(fract(pot * LINIEN * 0.15915) - 0.5) * 2.0;
    float linie = smoothstep(LINIEN_DICKE, LINIEN_DICKE * 0.3, band);
    // Neuronen-Puls: läuft das Potential entlang, Rate aus den Höhen
    float puls = pow(0.5 + 0.5 * sin(pot * 7.0 - iTime *
                                     (PULS_TEMPO + PULS_HOEHEN * treb)), 8.0);
    vec3 col = vec3(0.01, 0.012, 0.028);
    col += linie * vec3(0.10, 0.18, 0.30);
    col += linie * puls * vec3(0.5, 0.9, 1.0) * 1.4;
    // Somata: die Pole glühen und pulsieren
    float atem = 0.7 + 0.3 * sin(iTime * 2.0);
    col += exp(-d1 * 12.0) * vec3(1.0, 0.4, 0.6) * atem;
    col += exp(-d2 * 12.0) * vec3(0.4, 0.6, 1.0) * atem;
    col += exp(-d3 * 12.0) * vec3(0.5, 1.0, 0.6) * atem;
    fragColor = vec4(col, 1.0);
}
