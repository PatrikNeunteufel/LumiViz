// 25 Apollonian-Tanz — Patrik Neunteufel (LumiViz, 2026). Eigenwerk.
// Shadertoy: iChannel0 = Music. INVERSIONS-FRAKTAL (Kugel-Inversionen).
//
// IDEE: Wiederholte Kugel-Inversion (p /= dot(p,p)) mit Kachelung erzeugt
// das apollonische Kugelgepäck — unendlich ineinander geschachtelte Kugeln.
// Wir schneiden eine 2D-Ebene hindurch; der Skalenfaktor färbt. Der Bass
// pumpt den Inversionsradius: die Kugeln tanzen.

// ---- STELLSCHRAUBEN --------------------------------------------------------
const int   INVERSIONEN = 7;     // Iterationstiefe (mehr = feinere Kugeln)
const float SKALA       = 1.25;  // Grund-Inversionsstärke
const float SKALA_BASS  = 0.12;  // Bass-Pumpen der Inversion
const float ZOOM        = 1.1;
const float DREH_TEMPO  = 0.10;
const float KANTE       = 0.015; // Kugelrand-Schärfe (klein = hart)
// ----------------------------------------------------------------------------

vec2 rot2(vec2 p, float a) { return vec2(p.x * cos(a) - p.y * sin(a),
                                         p.x * sin(a) + p.y * cos(a)); }
void mainImage(out vec4 fragColor, in vec2 fragCoord)
{
    vec2 uv = (2.0 * fragCoord - iResolution.xy) / iResolution.y * ZOOM;
    float bass = texture(iChannel0, vec2(0.05, 0.25)).x;
    uv = rot2(uv, iTime * DREH_TEMPO);

    float s = SKALA + SKALA_BASS * bass;
    vec3 p = vec3(uv, 0.35 + 0.1 * sin(iTime * 0.2));  // Schnittebene wandert
    float scale = 1.0;   // akkumulierter Skalenfaktor (für Distanz + Farbe)
    float trap = 1e9;
    for (int i = 0; i < INVERSIONEN; ++i)
    {
        // Kacheln auf -1..1, dann Kugel-Inversion am Ursprung
        p = -1.0 + 2.0 * fract(0.5 * p + 0.5);
        float r2 = dot(p, p);
        trap = min(trap, r2);
        float k = s / r2;   // Inversionsfaktor
        p *= k;
        scale *= k;
    }
    // Abstand zur Kugeloberfläche im Original-Raum (durch scale zurück)
    float d = 0.25 * abs(p.y) / scale;
    // harte Kugelränder: alles jenseits KANTE ist schwarz
    float flaeche = 1.0 - smoothstep(0.0, KANTE, d);
    // Farbe: Trap + Skala als Fraktal-Tiefe
    vec3 col = flaeche * (0.5 + 0.5 * cos(log2(scale) * 0.8 + trap * 4.0 +
                                          iTime * 0.3 + vec3(0.0, 2.1, 4.2)));
    // leises Hintergrund-Glimmen statt hartem Schwarz im Nichts
    col += (1.0 - flaeche) * vec3(0.015, 0.01, 0.03);
    fragColor = vec4(col, 1.0);
}
