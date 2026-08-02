// 35 Licht-Fäden, Buffer A — Patrik Neunteufel (LumiViz, 2026). Eigenwerk.
// Shadertoy-Tab "Buffer A": iChannel0 = Buffer A (SELBST = Vorframe!),
//                           iChannel1 = Music.
//
// IDEE: Leuchtpunkte reiten auf einem CURL-NOISE-Feld (divergenzfrei =
// organische Strömungslinien): die Position je Punkt wird über eine
// geschlossene Bahn approximiert, deren Form das Feld verbiegt. Der
// Decay-Trail zieht daraus seidige Lichtfäden ("Lights"-Ästhetik).

// ---- STELLSCHRAUBEN --------------------------------------------------------
const int   FAEDEN       = 24;    // Anzahl Fäden
const float TRAIL        = 0.955; // Fadenlänge (näher an 1 = länger)
const float FELD_STAERKE = 0.45;  // wie stark das Feld die Bahnen verbiegt
const float FELD_GROESSE = 2.2;   // Wirbelgröße
const float TEMPO        = 0.35;
const float PUNKT_GROESSE = 0.006;
const float BASS_ENERGIE = 1.0;   // Bass macht die Fäden heller
// ----------------------------------------------------------------------------

float n21(vec2 p) { return fract(sin(dot(p, vec2(41.3, 289.1))) * 43758.5453); }
float vnoise(vec2 p)
{
    vec2 i = floor(p), f = fract(p);
    f = f * f * (3.0 - 2.0 * f);
    return mix(mix(n21(i), n21(i + vec2(1.0, 0.0)), f.x),
               mix(n21(i + vec2(0.0, 1.0)), n21(i + vec2(1.0, 1.0)), f.x), f.y);
}
vec2 curl(vec2 p)
{
    vec2 e = vec2(0.02, 0.0);
    float dx = vnoise(p + e.xy) - vnoise(p - e.xy);
    float dy = vnoise(p + e.yx) - vnoise(p - e.yx);
    return vec2(dy, -dx) / (2.0 * e.x);
}
float h1(float n) { return fract(sin(n) * 43758.5453); }
void mainImage(out vec4 fragColor, in vec2 fragCoord)
{
    vec2 uv = (2.0 * fragCoord - iResolution.xy) / iResolution.y;
    float bass = texture(iChannel1, vec2(0.05, 0.25)).x;
    vec3 col = texture(iChannel0, fragCoord / iResolution.xy).rgb * TRAIL;

    for (int i = 0; i < FAEDEN; ++i)
    {
        float fi = float(i);
        // Grundbahn: langsame Ellipse je Faden (Hash-Radien/-Phasen) …
        float ph = iTime * TEMPO * (0.5 + h1(fi * 3.7)) + fi * 2.4;
        vec2 p = vec2(0.9 * (h1(fi * 5.1) - 0.5) * 2.0 * cos(ph),
                      0.7 * (h1(fi * 8.3) - 0.5) * 2.0 * sin(ph * 1.3));
        // … vom Curl-Feld verbogen (Feld wandert langsam mit)
        p += FELD_STAERKE * curl(p * FELD_GROESSE + iTime * 0.1);
        float d = length(uv - p);
        vec3 farbe = 0.5 + 0.5 * cos(fi * 0.7 + iTime * 0.2 + vec3(0.0, 2.1, 4.2));
        col += smoothstep(PUNKT_GROESSE, 0.0, d) * farbe *
               (0.5 + BASS_ENERGIE * bass);
    }
    fragColor = vec4(min(col, vec3(2.5)), 1.0);
}
