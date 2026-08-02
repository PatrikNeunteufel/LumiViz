// 100 Grand Finale, Image — Patrik Neunteufel (LumiViz, 2026). Eigenwerk.
// "Image": iChannel0 = Buffer A. Sternenhimmel + Feuerwerk oben, darunter
// die wellige Ozean-Spiegelung des Buffers.

// ---- STELLSCHRAUBEN --------------------------------------------------------
const float HORIZONT   = -0.35;  // wie im Buffer
const float WELLEN     = 0.012;  // Spiegel-Verzerrung
const float SPIEGEL    = 0.45;   // Spiegelhelligkeit
// ----------------------------------------------------------------------------

float n21(vec2 p) { return fract(sin(dot(p, vec2(41.3, 289.1))) * 43758.5453); }
void mainImage(out vec4 fragColor, in vec2 fragCoord)
{
    vec2 uv01 = fragCoord / iResolution.xy;
    vec2 uv = (2.0 * fragCoord - iResolution.xy) / iResolution.y;
    vec3 col;
    if (uv.y > HORIZONT)
    {
        col = texture(iChannel0, uv01).rgb;
        col += vec3(0.006, 0.006, 0.018);
        // Sterne (nur über dem Horizont)
        col += step(0.998, n21(floor(uv * 90.0))) * 0.4;
    }
    else
    {
        // Spiegelung: an der Horizontlinie klappen + Wellen-Wabern
        float tiefe = HORIZONT - uv.y;
        float ySpiegel = HORIZONT + tiefe;
        vec2 quelle = vec2(uv.x + WELLEN * sin(uv.y * 60.0 + iTime * 2.5),
                           ySpiegel);
        // zurück in 0..1-Koordinaten des Buffers
        vec2 q01 = (quelle * iResolution.y + iResolution.xy) / (2.0 * iResolution.xy);
        col = texture(iChannel0, q01).rgb * SPIEGEL * (1.0 - tiefe * 0.6);
        col += vec3(0.01, 0.02, 0.04);  // Wasser-Grundton
    }
    fragColor = vec4(col, 1.0);
}
