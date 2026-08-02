// 58 Platinen-Pulse — Patrik Neunteufel (LumiViz, 2026). Eigenwerk.
// Shadertoy: iChannel0 = Music. TECHNIK: Leiterbahnen mit Signal-Läufern.
//
// IDEE: Ein Manhattan-Truchet: je Zelle entweder Gerade oder Knick (Hash).
// Auf den Bahnen laufen Lichtpunkte (Signal-Pulse); Lötpunkte an den
// Zellecken. Grün-goldene Platinen-Optik, Pulse = Spektralbänder.

// ---- STELLSCHRAUBEN --------------------------------------------------------
const float ZOOM       = 7.0;
const float BAHN_DICKE = 0.05;
const float PULS_TEMPO = 1.8;
const float PULS_DICHTE = 3.0;   // Pulse pro Bahn
const float LOET_RADIUS = 0.10;
// ----------------------------------------------------------------------------

float n21(vec2 p) { return fract(sin(dot(p, vec2(41.3, 289.1))) * 43758.5453); }
void mainImage(out vec4 fragColor, in vec2 fragCoord)
{
    vec2 uv = (2.0 * fragCoord - iResolution.xy) / iResolution.y * ZOOM;
    vec2 id = floor(uv);
    vec2 f = fract(uv);
    float rnd = n21(id);

    // Bahnabstand: Gerade (horizontal ODER vertikal) bzw. Diagonal-Knick
    float d;
    float bahnPos;  // Laufkoordinate für den Puls
    if (rnd < 0.35)      { d = abs(f.y - 0.5); bahnPos = f.x; }
    else if (rnd < 0.7)  { d = abs(f.x - 0.5); bahnPos = f.y; }
    else
    {
        // Knick: zwei Viertelkreise wie Truchet
        float v = step(0.5, fract(rnd * 7.0));
        vec2 g = v > 0.5 ? vec2(1.0 - f.x, f.y) : f;
        float d1 = abs(length(g) - 0.5);
        float d2 = abs(length(g - vec2(1.0)) - 0.5);
        d = min(d1, d2);
        bahnPos = (d1 < d2) ? atan(g.y, g.x) * 0.6366 : atan(1.0 - g.y, 1.0 - g.x) * 0.6366;
    }
    float bahn = smoothstep(BAHN_DICKE, BAHN_DICKE * 0.4, d);

    // Signal-Puls: läuft die Bahn entlang; das Zell-Band wählt Tempo-Phase
    float band = texture(iChannel0, vec2(0.05 + 0.6 * rnd, 0.25)).x;
    float puls = pow(0.5 + 0.5 * sin((bahnPos + rnd * 4.0) * 6.28318 * PULS_DICHTE -
                                     iTime * PULS_TEMPO * 6.28318), 12.0);
    // Lötpunkte an den Ecken
    vec2 ecke = f - round(f);
    float loet = smoothstep(LOET_RADIUS, LOET_RADIUS * 0.5, length(ecke));

    vec3 col = vec3(0.02, 0.06, 0.03);                         // Platine
    col += bahn * vec3(0.10, 0.35, 0.12);                      // Kupferbahn (grün maskiert)
    col += bahn * puls * vec3(0.4, 1.0, 0.5) * (0.4 + 1.4 * band);  // Signal
    col += loet * vec3(0.85, 0.7, 0.25);                       // Lötauge
    fragColor = vec4(col, 1.0);
}
