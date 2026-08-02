// 94 Hex-Datenstrom — Patrik Neunteufel (LumiViz, 2026). Eigenwerk.
// Shadertoy: iChannel0 = Music. KOMBI: Hexgitter × Daten-Regen — die
// Datenkolonnen fallen durch ein Sechseck-Raster: Hex-Zellen leuchten auf,
// wenn eine Kolonne sie durchquert (Kopf hell, Spur verblasst).

// ---- STELLSCHRAUBEN --------------------------------------------------------
const float ZOOM       = 6.0;
const float FALL_GRUND = 0.30;
const float FALL_BASS  = 0.5;
const float SPUR       = 0.45;   // Spurlänge (0..1 der Bildhöhe)
const float FUGE       = 0.04;
// ----------------------------------------------------------------------------

float hexDist(vec2 p)
{
    p = abs(p);
    return max(dot(p, vec2(0.8660254, 0.5)), p.x);
}
vec4 hexCoords(vec2 p)
{
    vec2 r = vec2(1.0, 1.7320508);
    vec2 h = r * 0.5;
    vec2 a = mod(p, r) - h;
    vec2 b = mod(p - h, r) - h;
    vec2 g = dot(a, a) < dot(b, b) ? a : b;
    return vec4(g, p - g);
}
float n21(vec2 p) { return fract(sin(dot(p, vec2(41.3, 289.1))) * 43758.5453); }
void mainImage(out vec4 fragColor, in vec2 fragCoord)
{
    vec2 uv = (2.0 * fragCoord - iResolution.xy) / iResolution.y * ZOOM;
    float bass = texture(iChannel0, vec2(0.05, 0.25)).x;
    vec4 h = hexCoords(uv);
    vec2 id = h.zw;
    float rnd = n21(vec2(id.x, 0.0));  // Spalten-Zufall (gleiche x-Spalte)

    // Kolonnen-Kopf in dieser Spalte (Weltkoordinate y, fällt endlos)
    float spanne = ZOOM * 2.4;  // sichtbare y-Spanne (großzügig)
    float kopf = spanne * 0.5 - fract(iTime * (FALL_GRUND + FALL_BASS * bass) *
                                      (0.5 + rnd) + rnd * 7.0) * spanne;
    float dy = kopf - id.y;  // Abstand des Zellzentrums HINTER dem Kopf
    float spur = (dy > 0.0 && dy < SPUR * spanne) ? 1.0 - dy / (SPUR * spanne) : 0.0;

    float d = hexDist(h.xy);
    float zelle = smoothstep(0.5, 0.5 - FUGE, d);
    // Zell-Innenmuster: Bitflackern wie Glyphen
    float bit = step(0.35, n21(id + floor(iTime * 4.0)));

    vec3 col = vec3(0.004, 0.012, 0.006);
    col += zelle * spur * bit * vec3(0.2, 1.0, 0.45) * (0.3 + spur);
    col += zelle * step(0.92, spur) * vec3(0.8, 1.0, 0.85);  // Kopfzelle weiß
    col += smoothstep(0.5, 0.47, d) * smoothstep(0.44, 0.47, d) *
           vec3(0.0, 0.15, 0.05);  // Zellrahmen schwach
    fragColor = vec4(col, 1.0);
}
