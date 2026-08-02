// 73 Magnet-Feld — Patrik Neunteufel (LumiViz, 2026). Eigenwerk.
// Shadertoy: iChannel0 = Music. GLOW: Feldlinien zweier wandernder Pole.
//
// IDEE: Das Potential zweier Ladungen (+/−) wird berechnet; seine
// HÖHENLINIEN (fract-Bänder des Potentials) sind die Feldlinien — sie
// verformen sich live, wenn die Pole umeinander kreisen. Ströme laufen
// als Helligkeit die Linien entlang. Mitten = Polabstand.

// ---- STELLSCHRAUBEN --------------------------------------------------------
const float LINIEN     = 14.0;  // Höhenlinien-Dichte
const float LINIEN_DICKE = 0.10; // relative Breite einer Linie
const float POL_TEMPO  = 0.4;
const float ABSTAND    = 0.45;
const float ABSTAND_MITTEN = 0.25;
const float STROM_TEMPO = 2.0;
// ----------------------------------------------------------------------------

void mainImage(out vec4 fragColor, in vec2 fragCoord)
{
    vec2 uv = (2.0 * fragCoord - iResolution.xy) / iResolution.y;
    float mid = texture(iChannel0, vec2(0.30, 0.25)).x;
    float abst = ABSTAND + ABSTAND_MITTEN * mid;

    // Pole kreisen umeinander
    vec2 plus  = abst * vec2(cos(iTime * POL_TEMPO), sin(iTime * POL_TEMPO));
    vec2 minus = -plus;
    float d1 = length(uv - plus) + 1e-3;
    float d2 = length(uv - minus) + 1e-3;
    // Potential + Feldlinien (fract-Bänder)
    float pot = log(d2 / d1);  // Dipol-Potential (2D)
    float band = abs(fract(pot * LINIEN * 0.15915) - 0.5) * 2.0;  // 0 auf Linie
    float linie = smoothstep(LINIEN_DICKE, LINIEN_DICKE * 0.3, band);
    // Strom: Lauflicht entlang des Potentials
    float strom = pow(0.5 + 0.5 * sin(pot * 8.0 - iTime * STROM_TEMPO), 5.0);

    vec3 col = vec3(0.012, 0.012, 0.03);
    col += linie * vec3(0.15, 0.3, 0.6);
    col += linie * strom * vec3(0.4, 0.8, 1.0) * 1.2;
    // Pole selbst glühen (+ rot, − blau)
    col += exp(-d1 * 14.0) * vec3(1.0, 0.3, 0.2) * 1.5;
    col += exp(-d2 * 14.0) * vec3(0.2, 0.4, 1.0) * 1.5;
    fragColor = vec4(col, 1.0);
}
