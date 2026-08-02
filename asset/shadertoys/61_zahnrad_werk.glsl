// 61 Zahnrad-Werk — Patrik Neunteufel (LumiViz, 2026). Eigenwerk.
// Shadertoy: iChannel0 = Music. TECHNIK: kämmende Zahnräder.
//
// IDEE: Ein Zahnrad-SDF: Kreis + Zahnkranz (cos über den Winkel moduliert
// den Radius) + Nabe + Speichen. Drei Räder kämmen (Drehzahl ∝ 1/Radius,
// gegensinnig); der Bass ist das "Drehmoment" (Tempo aller Räder).

// ---- STELLSCHRAUBEN --------------------------------------------------------
const float TEMPO_GRUND = 0.4;
const float TEMPO_BASS  = 0.8;
const float ZAHN_HOEHE  = 0.045;
const float KANTE       = 0.008;  // Metallkante (klein = hart)
// ----------------------------------------------------------------------------

vec2 rot2(vec2 p, float a) { return vec2(p.x * cos(a) - p.y * sin(a),
                                         p.x * sin(a) + p.y * cos(a)); }
// Zahnrad-Abstandsfeld: Rückgabe < 0 = im Metall
float zahnrad(vec2 p, float radius, float zaehne, float drehung)
{
    p = rot2(p, drehung);
    float a = atan(p.y, p.x);
    float r = length(p);
    float kranz = radius + ZAHN_HOEHE * (0.5 + 0.5 * cos(a * zaehne));  // Zähne
    float d = r - kranz;
    d = max(d, 0.18 * radius / 0.35 * 0.35 - r);  // Nabenloch… vereinfachte Nabe
    // Speichenfenster: 4 Löcher zwischen Nabe und Kranz
    float fenster = min(abs(fract(a * 0.6366 + 0.125) - 0.5) * radius * 3.0,
                        max(r - radius * 0.35, radius * 0.8 - r));
    d = max(d, -fenster + 0.05 * radius);
    return d;
}
void mainImage(out vec4 fragColor, in vec2 fragCoord)
{
    vec2 uv = (2.0 * fragCoord - iResolution.xy) / iResolution.y;
    float bass = texture(iChannel0, vec2(0.05, 0.25)).x;
    float t = iTime * (TEMPO_GRUND + TEMPO_BASS * bass);

    // drei kämmende Räder: Positionen so, dass die Kränze sich berühren
    vec3 col = vec3(0.015, 0.015, 0.025);
    float gesamt = 1e9;
    float glanzWinkel = 0.0;
    // (Radius, Zähne, Position, Richtung)
    for (int i = 0; i < 3; ++i)
    {
        float radius = (i == 0) ? 0.34 : (i == 1) ? 0.22 : 0.27;
        float zaehne = floor(radius * 44.0);
        vec2 pos = (i == 0) ? vec2(-0.35, -0.05)
                 : (i == 1) ? vec2(0.24, 0.28)
                            : vec2(0.33, -0.33);
        float richtung = (i == 1) ? -1.0 : ((i == 2) ? -1.0 : 1.0);
        // Drehzahl ∝ 1/Zähne (Kämm-Bedingung), Phase fein justiert
        float dreh = richtung * t * 12.0 / zaehne + float(i) * 0.3;
        float d = zahnrad(uv - pos, radius, zaehne, dreh);
        if (d < gesamt) { gesamt = d; glanzWinkel = atan(uv.y - pos.y, uv.x - pos.x) + dreh; }
    }
    float metall = smoothstep(KANTE, -KANTE, gesamt);
    // Metall-Look: Grundton + umlaufender Glanzstreifen + Kantenlicht
    vec3 stahl = vec3(0.55, 0.58, 0.65);
    float glanz = pow(0.5 + 0.5 * cos(glanzWinkel * 2.0 + 1.0), 8.0);
    col = mix(col, stahl * (0.35 + 0.5 * glanz), metall);
    col += smoothstep(KANTE, 0.0, abs(gesamt)) * vec3(0.9, 0.85, 0.6) * 0.35; // Kante
    fragColor = vec4(col, 1.0);
}
