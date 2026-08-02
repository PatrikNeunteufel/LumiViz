// 19 Orbit-Partikel, Buffer A — Patrik Neunteufel (LumiViz, 2026). Eigenwerk.
// Shadertoy-Tab "Buffer A": iChannel0 = Buffer A (SELBST = Vorframe!),
//                           iChannel1 = Music.
//
// IDEE: Glühpunkte auf elliptischen Bahnen (alles gehasht je Partikel);
// der Decay-Trail zieht die Schweife. Bass weitet die Bahnen.

// ---- STELLSCHRAUBEN --------------------------------------------------------
const int   PARTIKEL     = 48;
const float TRAIL        = 0.93;  // näher an 1 = längere Schweife
const float BAHN_MIN     = 0.25;  // kleinster Bahnradius
const float BAHN_STREUUNG = 0.65; // Radius-Streuung
const float BASS_WEITUNG = 0.20;  // Bass weitet alle Bahnen
const float TEMPO_MIN    = 0.3;   // langsamster Umlauf
const float PUNKT_GROESSE = 0.012;
const float PUNKT_HOEHEN = 0.010; // Höhen vergrößern die Punkte
// ----------------------------------------------------------------------------

float h1(float n) { return fract(sin(n) * 43758.5453); }
void mainImage(out vec4 fragColor, in vec2 fragCoord)
{
    vec2 uv = (2.0 * fragCoord - iResolution.xy) / iResolution.y;
    float bass = texture(iChannel1, vec2(0.05, 0.25)).x;
    float treb = texture(iChannel1, vec2(0.70, 0.25)).x;
    vec3 col = texture(iChannel0, fragCoord / iResolution.xy).rgb * TRAIL;
    for (int i = 0; i < PARTIKEL; ++i)
    {
        float fi = float(i);
        float rad = BAHN_MIN + BAHN_STREUUNG * h1(fi * 3.1) + BASS_WEITUNG * bass;
        // Drehrichtung per Hash-Vorzeichen
        float speed = (TEMPO_MIN + h1(fi * 7.7)) * (h1(fi) > 0.5 ? 1.0 : -1.0);
        float ang = iTime * speed + fi * 2.4;
        // Ellipse: y um Hash-Faktor gestaucht
        vec2 p = rad * vec2(cos(ang), sin(ang) * (0.6 + 0.4 * h1(fi * 5.3)));
        float d = length(uv - p);
        col += smoothstep(PUNKT_GROESSE + PUNKT_HOEHEN * treb, 0.0, d) *
               (0.5 + 0.5 * cos(fi * 0.8 + vec3(0.0, 2.1, 4.2)));
    }
    fragColor = vec4(min(col, vec3(2.0)), 1.0);  // Deckel gegen Aufschaukeln
}
