// 62 Hologramm-Globus — Patrik Neunteufel (LumiViz, 2026). Eigenwerk.
// Shadertoy: iChannel0 = Music. TECHNIK: Drahtgitter-Globus als Hologramm.
//
// IDEE: Eine Kugel wird als Punktprojektion gerendert: Längen-/Breitenkreise
// entstehen aus der Rückprojektion der Bildkoordinate auf die Kugel (nur
// die VORDERSEITE), dazu Scanlines, Flacker-Störungen und chromatischer
// Versatz — der klassische Sci-Fi-Look. Mitten = Störungsgrad.

// ---- STELLSCHRAUBEN --------------------------------------------------------
const float RADIUS      = 0.65;
const float DREH_TEMPO  = 0.35;
const float GITTER      = 8.0;   // Längen-/Breitenkreise
const float LINIEN_DICKE = 0.03;
const float SCANLINES   = 180.0;
const float STOERUNG    = 0.5;   // Grund-Flackern
const float STOER_MITTEN = 1.0;
// ----------------------------------------------------------------------------

float n21(vec2 p) { return fract(sin(dot(p, vec2(41.3, 289.1))) * 43758.5453); }
// Gitterhelligkeit der Kugel-Vorderseite an Bildposition q (oder 0)
float globus(vec2 q, float drehung)
{
    float r2 = dot(q, q);
    if (r2 > RADIUS * RADIUS) return 0.0;
    // Rückprojektion: z aus der Kugelgleichung, dann Länge/Breite
    float z = sqrt(RADIUS * RADIUS - r2);
    vec3 p = vec3(q, z) / RADIUS;
    float laenge = atan(p.z, p.x) + drehung;
    float breite = asin(clamp(p.y, -1.0, 1.0));
    float g1 = pow(0.5 + 0.5 * cos(laenge * GITTER), 1.0 / LINIEN_DICKE * 0.03);
    float g2 = pow(0.5 + 0.5 * cos(breite * GITTER * 2.0), 1.0 / LINIEN_DICKE * 0.03);
    // Vorderseite heller als Randpartien (z als Beleuchtung)
    return (g1 + g2) * (0.35 + 0.65 * p.z);
}
void mainImage(out vec4 fragColor, in vec2 fragCoord)
{
    vec2 uv = (2.0 * fragCoord - iResolution.xy) / iResolution.y;
    float mid = texture(iChannel0, vec2(0.30, 0.25)).x;
    float drehung = iTime * DREH_TEMPO;

    // Störung: ganze Zeilen springen kurz zur Seite
    float zeile = floor(fragCoord.y / 3.0);
    float stoer = STOERUNG + STOER_MITTEN * mid;
    float ruckel = (n21(vec2(zeile, floor(iTime * 8.0))) - 0.5) *
                   step(0.93, n21(vec2(floor(iTime * 8.0), zeile))) * 0.06 * stoer;
    vec2 q = uv + vec2(ruckel, 0.0);

    // chromatischer Versatz: RGB minimal auseinander
    float g = globus(q, drehung);
    float rKanal = globus(q + vec2(0.004 * stoer, 0.0), drehung);
    float bKanal = globus(q - vec2(0.004 * stoer, 0.0), drehung);
    vec3 col = vec3(rKanal * 0.5, g, bKanal * 1.2) * vec3(0.3, 0.9, 1.0);

    // Scanlines + Flackern
    col *= 0.8 + 0.2 * sin(fragCoord.y * 6.28318 * SCANLINES / iResolution.y);
    col *= 0.9 + 0.1 * sin(iTime * 47.0);
    // Sockel-Lichtkegel
    col += smoothstep(0.9, 0.0, length(uv + vec2(0.0, 0.9))) * vec3(0.05, 0.15, 0.2);
    fragColor = vec4(col, 1.0);
}
