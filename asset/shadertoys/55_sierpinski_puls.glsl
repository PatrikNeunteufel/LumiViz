// 55 Sierpinski-Puls — Patrik Neunteufel (LumiViz, 2026). Eigenwerk.
// Shadertoy: iChannel0 = Music. IFS-FRAKTAL mit Neon-Kanten.
//
// IDEE: Das Sierpinski-Dreieck über Faltungen (Spiegel an den drei
// Seitenhalbierenden + Verdopplung). Statt Flächen leuchten die KANTEN
// jeder Tiefenstufe neon; der Bass pumpt Tiefe für Tiefe durch (Lauflicht
// durch die Rekursionsebenen).

// ---- STELLSCHRAUBEN --------------------------------------------------------
const int   TIEFEN      = 7;
const float KANTEN_DICKE = 0.012;
const float PULS_TEMPO  = 1.5;   // Lauflicht durch die Ebenen
const float DREH_DRIFT  = 0.03;
const float ZOOM        = 1.15;
// ----------------------------------------------------------------------------

vec2 rot2(vec2 p, float a) { return vec2(p.x * cos(a) - p.y * sin(a),
                                         p.x * sin(a) + p.y * cos(a)); }
void mainImage(out vec4 fragColor, in vec2 fragCoord)
{
    vec2 uv = (2.0 * fragCoord - iResolution.xy) / iResolution.y * ZOOM;
    float bass = texture(iChannel0, vec2(0.05, 0.25)).x;
    uv = rot2(uv, iTime * DREH_DRIFT);
    uv.y += 0.25;

    vec3 col = vec3(0.01, 0.01, 0.02);
    vec2 p = uv;
    float s = 1.0;
    for (int i = 0; i < TIEFEN; ++i)
    {
        // Sierpinski-Faltung: an x=0 spiegeln, an den ±60°-Achsen spiegeln,
        // dann verdoppeln und nach unten versetzen
        p.x = abs(p.x);
        p = rot2(p, 1.0472);   //  60°
        p.x = abs(p.x);
        p = rot2(p, -1.0472);  // zurück
        p = p * 2.0 - vec2(0.0, 0.5);
        s *= 2.0;
        // Kante dieser Ebene: Abstand zur unteren Dreieckskante (y = −0.5)
        float d = abs(p.y + 0.5) / s;
        float kante = smoothstep(KANTEN_DICKE, KANTEN_DICKE * 0.3, d);
        // Lauflicht: eine Ebene nach der anderen leuchtet auf (Bass hebt alle)
        float puls = pow(0.5 + 0.5 * sin(iTime * PULS_TEMPO - float(i) * 1.1), 4.0);
        vec3 farbe = 0.5 + 0.5 * cos(float(i) * 0.9 + vec3(0.0, 2.1, 4.2));
        col += kante * farbe * (0.15 + puls * 0.8 + bass * 0.4) /
               (1.0 + float(i) * 0.3);
    }
    fragColor = vec4(col, 1.0);
}
