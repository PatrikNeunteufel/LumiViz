// 48 Zellteilung — Patrik Neunteufel (LumiViz, 2026). Eigenwerk.
// Shadertoy: iChannel0 = Music. BIOLOGIE: Metaball-Zellen, die sich teilen.
//
// IDEE: Zellpaare entstehen aus je zwei Metaballs, deren Abstand periodisch
// von 0 (eine Zelle) bis "getrennt" wächst — die Iso-Fläche schnürt sich
// dabei sichtbar ein wie eine Mitose. Membran = schmale Zone um die
// Iso-Schwelle, Kern = zweiter, kleinerer Metaball-Satz.

// ---- STELLSCHRAUBEN --------------------------------------------------------
const int   ZELLEN       = 5;     // Anzahl Zellpaare
const float TEILUNGS_DAUER = 6.0; // Sekunden je Teilungszyklus
const float ZELL_RADIUS  = 0.16;
const float MEMBRAN_DICKE = 0.06;
const float ISO          = 1.0;
const float BASS_PULS    = 0.25;  // Bass lässt das Plasma wabern
// ----------------------------------------------------------------------------

float h1(float n) { return fract(sin(n) * 43758.5453); }
void mainImage(out vec4 fragColor, in vec2 fragCoord)
{
    vec2 uv = (2.0 * fragCoord - iResolution.xy) / iResolution.y;
    float bass = texture(iChannel0, vec2(0.05, 0.25)).x;

    float feld = 0.0;
    float kern = 0.0;
    for (int i = 0; i < ZELLEN; ++i)
    {
        float fi = float(i);
        // Zellzentrum driftet langsam
        vec2 z = 0.65 * vec2(sin(iTime * 0.15 + fi * 2.4) + (h1(fi * 3.1) - 0.5),
                             cos(iTime * 0.11 + fi * 4.1) * 0.8);
        // Teilungsphase 0..1: Abstand der beiden Hälften
        float phase = fract(iTime / TEILUNGS_DAUER + h1(fi * 7.7));
        float trennung = smoothstep(0.3, 0.9, phase) * 0.34;
        vec2 richtung = vec2(cos(fi * 2.1), sin(fi * 2.1));
        float r = ZELL_RADIUS * (1.0 + BASS_PULS * bass);
        vec2 a = z + richtung * trennung;
        vec2 b = z - richtung * trennung;
        feld += r * r / (dot(uv - a, uv - a) + 1e-4);
        feld += r * r / (dot(uv - b, uv - b) + 1e-4);
        // Kerne: kleiner, gleiche Zentren
        float rk = r * 0.35;
        kern += rk * rk / (dot(uv - a, uv - a) + 1e-4);
        kern += rk * rk / (dot(uv - b, uv - b) + 1e-4);
    }
    float zellflaeche = smoothstep(ISO - 0.05, ISO + 0.05, feld);
    // Membran: schmales Band um die Iso-Schwelle
    float membran = exp(-pow((feld - ISO) / (MEMBRAN_DICKE * ISO), 2.0));
    float kernfl = smoothstep(ISO, ISO * 1.4, kern);

    vec3 col = vec3(0.01, 0.02, 0.03);                     // Nährlösung
    col += zellflaeche * vec3(0.10, 0.20, 0.16);           // Zellplasma
    col += membran * vec3(0.35, 0.95, 0.6);                // Membran-Glow
    col += kernfl * vec3(0.9, 0.5, 0.75);                  // Zellkern
    fragColor = vec4(col, 1.0);
}
