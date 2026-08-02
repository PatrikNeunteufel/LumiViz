// 70 Laser-Raum — Patrik Neunteufel (LumiViz, 2026). Eigenwerk.
// Shadertoy: iChannel0 = Music. GLOW: perspektivisches Lasergitter im Nebel.
//
// IDEE: Ein Boden- und Deckengitter in Ein-Punkt-Perspektive (Projektion
// y/z, x/z), die Gitterlinien glühen; Nebel dämpft mit der Tiefe. Die Bahn
// scrollt vorwärts (Bass = Tempo), Vertikallaser wischen quer (Höhen).

// ---- STELLSCHRAUBEN --------------------------------------------------------
const float TEMPO_GRUND = 1.2;
const float TEMPO_BASS  = 2.0;
const float GITTER      = 2.0;   // Linienabstand
const float LINIEN_GLOW = 0.02;
const float RAUM_HOEHE  = 0.8;
const float NEBEL       = 0.12;
const int   WISCHER     = 3;     // Quer-Laser
// ----------------------------------------------------------------------------

void mainImage(out vec4 fragColor, in vec2 fragCoord)
{
    vec2 uv = (2.0 * fragCoord - iResolution.xy) / iResolution.y;
    float bass = texture(iChannel0, vec2(0.05, 0.25)).x;
    float treb = texture(iChannel0, vec2(0.70, 0.25)).x;
    float fahrt = iTime * (TEMPO_GRUND + TEMPO_BASS * bass);

    vec3 col = vec3(0.01, 0.005, 0.02);
    // Boden (uv.y < 0) und Decke (uv.y > 0) als projizierte Ebenen
    for (int seite = 0; seite < 2; ++seite)
    {
        float vorzeichen = (seite == 0) ? -1.0 : 1.0;
        float y = uv.y * vorzeichen;
        if (y < -0.02) continue;
        float z = RAUM_HOEHE / max(y, 0.02);        // Tiefe aus der Projektion
        float x = uv.x * z;                          // Weltkoordinate quer
        // Gitter: Quer- und Längslinien im Weltmaß
        float quer = abs(fract((z + fahrt) / GITTER) - 0.5);
        float laengs = abs(fract(x / GITTER) - 0.5);
        float linie = LINIEN_GLOW / (quer * quer + 0.002) +
                      LINIEN_GLOW / (laengs * laengs + 0.002);
        vec3 farbe = (seite == 0) ? vec3(0.9, 0.2, 0.8) : vec3(0.2, 0.6, 1.0);
        col += linie * 0.02 * farbe * exp(-z * NEBEL);
    }
    // Quer-Wischer: vertikale Laser wandern durchs Bild
    for (int i = 0; i < WISCHER; ++i)
    {
        float fi = float(i);
        float x = sin(iTime * (0.7 + fi * 0.23) + fi * 2.1) * 0.9;
        float d = abs(uv.x - x);
        col += (0.0008 + 0.0016 * treb) / (d * d + 0.001) * 0.15 *
               (0.5 + 0.5 * cos(fi * 2.0 + vec3(0.0, 2.1, 4.2)));
    }
    fragColor = vec4(col, 1.0);
}
