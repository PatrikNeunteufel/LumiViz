// 29 Fluss-Feld, Buffer A — Patrik Neunteufel (LumiViz, 2026). Eigenwerk.
// Shadertoy-Tab "Buffer A": iChannel0 = Buffer A (SELBST = Vorframe!),
//                           iChannel1 = Music.
//
// IDEE: FARB-ADVEKTION (Mini-Fluid): jedes Pixel liest das Vorframe an der
// Stelle, von der die Strömung es "herträgt" (uv − v·dt). Das Strömungsfeld
// ist der CURL eines Value-Noise (divergenzfrei = wirbelig, ohne Quellen).
// Wandernde Farbquellen tropfen Tinte hinein; der Bass rührt kräftiger.

// ---- STELLSCHRAUBEN --------------------------------------------------------
const float STROEMUNG    = 0.0035; // Advektions-Schrittweite (Fließtempo)
const float BASS_RUEHREN = 2.0;    // Bass verstärkt die Strömung
const float WIRBEL_GROESSE = 3.0;  // Noise-Skala des Feldes (mehr = kleinere Wirbel)
const float FELD_WANDEL  = 0.10;   // wie schnell sich das Wirbelfeld ändert
const float VERBLASSEN   = 0.995;  // Tinte hält lang (0.98 = schneller weg)
const int   QUELLEN      = 3;      // Anzahl Tintenquellen
const float QUELL_GROESSE = 0.02;
// ----------------------------------------------------------------------------

float n21(vec2 p) { return fract(sin(dot(p, vec2(41.3, 289.1))) * 43758.5453); }
float vnoise(vec2 p)
{
    vec2 i = floor(p), f = fract(p);
    f = f * f * (3.0 - 2.0 * f);
    return mix(mix(n21(i), n21(i + vec2(1.0, 0.0)), f.x),
               mix(n21(i + vec2(0.0, 1.0)), n21(i + vec2(1.0, 1.0)), f.x), f.y);
}
// Curl des Noise: (dN/dy, −dN/dx) — divergenzfreies Wirbelfeld
vec2 curl(vec2 p)
{
    vec2 e = vec2(0.01, 0.0);
    float dx = vnoise(p + e.xy) - vnoise(p - e.xy);
    float dy = vnoise(p + e.yx) - vnoise(p - e.yx);
    return vec2(dy, -dx) / (2.0 * e.x);
}
void mainImage(out vec4 fragColor, in vec2 fragCoord)
{
    vec2 uv = fragCoord / iResolution.xy;
    float bass = texture(iChannel1, vec2(0.05, 0.25)).x;

    // Strömung am Pixel (Feld wandert langsam durch die 3. Noise-Achse
    // — hier als Zeitversatz in den Koordinaten)
    vec2 v = curl(uv * WIRBEL_GROESSE + iTime * FELD_WANDEL);
    // ADVEKTION: Vorframe dort lesen, wo die Strömung herkommt
    vec2 quelle = uv - v * STROEMUNG * (1.0 + BASS_RUEHREN * bass);
    vec3 col = texture(iChannel0, quelle).rgb * VERBLASSEN;

    // Tintenquellen: wandern auf Lissajous-Bahnen, jede mit eigener Farbe
    for (int i = 0; i < QUELLEN; ++i)
    {
        float fi = float(i);
        vec2 p = 0.5 + 0.35 * vec2(cos(iTime * (0.3 + fi * 0.17) + fi * 2.1),
                                   sin(iTime * (0.4 + fi * 0.13) + fi * 4.2));
        float d = length((uv - p) * vec2(iResolution.x / iResolution.y, 1.0));
        vec3 tinte = 0.5 + 0.5 * cos(fi * 2.1 + iTime * 0.2 + vec3(0.0, 2.1, 4.2));
        col = max(col, tinte * smoothstep(QUELL_GROESSE, 0.0, d));
    }
    fragColor = vec4(col, 1.0);
}
