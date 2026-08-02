// 26 Truchet-Neon — Patrik Neunteufel (LumiViz, 2026). Eigenwerk.
// Shadertoy: iChannel0 = Music. TRUCHET-KACHELN (verschlungene Neonbahnen).
//
// IDEE: Jede Gitterzelle enthält zwei Viertelkreis-Bögen; ein Hash
// entscheidet die Orientierung — zusammen entsteht ein endloses,
// zusammenhängendes Bahnennetz. Der Abstand zur Bahn wird als Neon-Glow
// gerendert; jede Zelle leuchtet mit "ihrem" FFT-Band. Zwei Skalen
// überlagern sich.

// ---- STELLSCHRAUBEN --------------------------------------------------------
const float ZOOM        = 5.0;    // Kacheln pro Bildhöhe
const float BAHN_DICKE  = 0.03;   // Kernbreite der Neon-Bahn
const float GLOW_WEITE  = 0.10;   // Glow-Halo um die Bahn
const float DRIFT_TEMPO = 0.20;   // das Muster zieht diagonal
const float PULS_TEMPO  = 2.0;    // Lauflicht entlang der Bahnen
const float HELL_AUDIO  = 1.3;
// ----------------------------------------------------------------------------

float n21(vec2 p) { return fract(sin(dot(p, vec2(41.3, 289.1))) * 43758.5453); }
// Abstand zur Truchet-Bahn in einer Zelle (zwei Viertelkreise, r = 0.5)
float truchet(vec2 f, float variante)
{
    if (variante > 0.5) f.x = 1.0 - f.x;  // gespiegelte Orientierung
    float d1 = abs(length(f) - 0.5);              // Bogen um (0,0)
    float d2 = abs(length(f - vec2(1.0)) - 0.5);  // Bogen um (1,1)
    return min(d1, d2);
}
void mainImage(out vec4 fragColor, in vec2 fragCoord)
{
    vec2 uv = (2.0 * fragCoord - iResolution.xy) / iResolution.y * ZOOM;
    uv += iTime * DRIFT_TEMPO * vec2(0.7, 0.4);
    vec3 col = vec3(0.01, 0.01, 0.02);

    // zwei Skalen: groß + halbe Größe versetzt (dichteres Geflecht)
    for (int layer = 0; layer < 2; ++layer)
    {
        vec2 p = uv * (1.0 + float(layer)) + float(layer) * 17.3;
        vec2 id = floor(p);
        vec2 f = fract(p);
        float rnd = n21(id);
        float d = truchet(f, fract(rnd * 2.0));
        // Zellband: der Hash wählt den FFT-Bin
        float band = texture(iChannel0, vec2(0.05 + 0.6 * rnd, 0.25)).x;
        // Neon: harter Kern + weicher Halo
        float kern = smoothstep(BAHN_DICKE, BAHN_DICKE * 0.5, d);
        float halo = smoothstep(GLOW_WEITE, 0.0, d) * 0.4;
        // Lauflicht: Helligkeit wellt entlang der Bahn (über den Winkel)
        float lauf = 0.6 + 0.4 * sin(iTime * PULS_TEMPO + rnd * 6.28318 +
                                     (f.x + f.y) * 6.0);
        vec3 farbe = 0.5 + 0.5 * cos(rnd * 6.28318 + float(layer) * 2.0 +
                                     vec3(0.0, 2.1, 4.2));
        col += (kern + halo) * farbe * lauf * (0.25 + HELL_AUDIO * band) /
               (1.0 + float(layer));
    }
    fragColor = vec4(col, 1.0);
}
