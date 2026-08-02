// 31 Neon-Stadt 3D — Patrik Neunteufel (LumiViz, 2026). Eigenwerk.
// Shadertoy: iChannel0 = Music. RAYMARCHING (unendliches Stadtraster).
//
// IDEE: Der Raum wird in Blöcke gekachelt (mod); jede Zelle enthält einen
// Quader mit gehashter Höhe = Hochhaus. Die Kamera fliegt eine Straße
// entlang; Fensterbänder leuchten mit dem Spektrum, Kanten glühen neon.

// ---- STELLSCHRAUBEN --------------------------------------------------------
const int   MARSCH      = 90;
const float BLOCK       = 2.0;    // Rastermaß (Straßenbreite inklusive)
const float HAUS_BREITE = 0.55;   // Quaderbreite in der Zelle
const float HOEHE_MAX   = 2.2;    // maximale Hochhaushöhe
const float FLUG_TEMPO  = 0.9;
const float NEON_GLOW   = 1.2;    // Kanten-/Fensterglühen
const float FENSTER_DICHTE = 9.0; // Fensterbänder pro Höheneinheit
// ----------------------------------------------------------------------------

float n21(vec2 p) { return fract(sin(dot(p, vec2(41.3, 289.1))) * 43758.5453); }
float sdBox(vec3 p, vec3 b)
{
    vec3 q = abs(p) - b;
    return length(max(q, 0.0)) + min(max(q.x, max(q.y, q.z)), 0.0);
}
// Szene: gekacheltes Häuserraster; Rückgabe x = Distanz, y = Haus-Zufall
vec2 stadt(vec3 p)
{
    vec2 id = floor(p.xz / BLOCK);
    vec2 lokal = mod(p.xz, BLOCK) - 0.5 * BLOCK;
    float rnd = n21(id);
    float hoehe = 0.3 + HOEHE_MAX * rnd * rnd;   // rnd² = wenige sehr hohe
    float d = sdBox(vec3(lokal.x, p.y - hoehe * 0.5, lokal.y),
                    vec3(HAUS_BREITE, hoehe * 0.5, HAUS_BREITE));
    d = min(d, p.y);  // Boden
    return vec2(d, rnd);
}
void mainImage(out vec4 fragColor, in vec2 fragCoord)
{
    vec2 uv = (2.0 * fragCoord - iResolution.xy) / iResolution.y;
    float bass = texture(iChannel0, vec2(0.05, 0.25)).x;

    // Kamera: mittig auf der Straße (Zellgrenze), leicht schwebend
    vec3 ro = vec3(BLOCK * 0.5, 0.8 + 0.1 * sin(iTime * 0.5),
                   iTime * (FLUG_TEMPO + bass));
    vec3 rd = normalize(vec3(uv.x, uv.y - 0.1, 1.3));

    float t = 0.0;
    vec2 h = vec2(1.0, 0.0);
    int schritte = 0;
    for (int i = 0; i < MARSCH; ++i)
    {
        h = stadt(ro + rd * t);
        schritte = i;
        if (h.x < 0.002 || t > 40.0) break;
        t += h.x;
    }
    // Nachthimmel mit Horizontglühen
    vec3 col = mix(vec3(0.02, 0.01, 0.05), vec3(0.15, 0.05, 0.2),
                   pow(max(1.0 - abs(rd.y), 0.0), 4.0));
    if (h.x < 0.002)
    {
        vec3 p = ro + rd * t;
        float rnd = h.y;
        // Fensterbänder: horizontale Streifen über die Höhe; jedes Haus
        // hört auf "seinen" FFT-Bin
        float band = texture(iChannel0, vec2(0.05 + 0.6 * rnd, 0.25)).x;
        float fenster = step(0.6, fract(p.y * FENSTER_DICHTE)) *
                        step(0.15, p.y);  // Erdgeschoss dunkel
        vec3 neon = 0.5 + 0.5 * cos(rnd * 6.28318 + vec3(0.0, 2.1, 4.2));
        col = vec3(0.02, 0.02, 0.04);                       // Fassade
        col += fenster * neon * band * NEON_GLOW;           // Fenster
        col += smoothstep(0.05, 0.0, p.y) * neon * 0.4;     // Straßen-Reflex
        col = mix(col, vec3(0.02, 0.01, 0.05), smoothstep(15.0, 40.0, t));
    }
    // Streifschuss-Glow an Silhouetten
    col += pow(float(schritte) / float(MARSCH), 3.0) * vec3(0.3, 0.1, 0.5) *
           (0.5 + bass);
    fragColor = vec4(col, 1.0);
}
