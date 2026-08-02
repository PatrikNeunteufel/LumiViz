// 90 Opal-Atmung — Patrik Neunteufel (LumiViz, 2026). Eigenwerk.
// Shadertoy: iChannel0 = Music. KOMBI: Opal × Voronoi-Atmung — die
// Farbpatches ATMEN mit "ihrem" Spektralband (Kern-Hash wählt den Bin):
// laute Bänder blähen ihre Patches und drehen deren Farbspiel schneller.

// ---- STELLSCHRAUBEN --------------------------------------------------------
const float PATCH_DICHTE = 3.0;
const float ATMUNG       = 0.5;   // Audio-Blähung der Patches
const float SPIEL_TEMPO  = 1.5;   // Audio-Drehung des Farbspiels
const float MILCH        = 0.4;
// ----------------------------------------------------------------------------

vec2 hash2(vec2 p)
{
    p = vec2(dot(p, vec2(127.1, 311.7)), dot(p, vec2(269.5, 183.3)));
    return fract(sin(p) * 43758.5453);
}
void mainImage(out vec4 fragColor, in vec2 fragCoord)
{
    vec2 uv = (2.0 * fragCoord - iResolution.xy) / iResolution.y;
    vec2 p = uv * PATCH_DICHTE;
    vec2 g = floor(p);
    vec2 f = fract(p);
    float d1 = 8.0, d2 = 8.0;
    vec2 id = vec2(0.0);
    for (int y = -1; y <= 1; ++y)
    for (int x = -1; x <= 1; ++x)
    {
        vec2 o = vec2(float(x), float(y));
        vec2 h = hash2(g + o);
        // Patch-Gewicht atmet mit dem Band des Kerns
        float band = texture(iChannel0, vec2(0.05 + 0.6 * h.x, 0.25)).x;
        vec2 c = o + h - f;
        float d = dot(c, c) / (1.0 + ATMUNG * band);  // laut = Patch reicht weiter
        if (d < d1) { d2 = d1; d1 = d; id = h; }
        else if (d < d2) d2 = d;
    }
    float band = texture(iChannel0, vec2(0.05 + 0.6 * id.x, 0.25)).x;
    float weich = smoothstep(0.0, 0.22, d2 - d1);
    // Farbspiel: Grundphase je Patch, dreht mit dem Band-Pegel
    vec3 spiel = 0.5 + 0.5 * cos(id.x * 18.0 + iTime * SPIEL_TEMPO * band * 3.0 +
                                 id.y * 3.0 + vec3(0.0, 2.1, 4.2));
    vec3 col = vec3(0.85, 0.88, 0.92) * MILCH + spiel * weich * (0.4 + 1.1 * band);
    col *= smoothstep(1.35, 0.7, length(uv * vec2(0.85, 1.15)));  // Stein-Oval
    fragColor = vec4(col, 1.0);
}
