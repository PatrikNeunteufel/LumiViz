// 87 Wellen-Kristall — Patrik Neunteufel (LumiViz, 2026). Eigenwerk.
// Shadertoy: iChannel0 = Music. KOMBI: Wasser × Kristall — das Wellen-
// Höhenfeld wird FACETTIERT (Normale auf ein grobes Raster quantisiert):
// gefrorenes, kristallines Wasser, dessen Facetten einzeln blitzen.

// ---- STELLSCHRAUBEN --------------------------------------------------------
const float FACETTEN   = 14.0;  // Quantisierungs-Raster der Normalen
const float WELLEN_TEMPO = 1.0;
const float GLANZ      = 32.0;
const float MITTEN_CHOP = 0.6;
// ----------------------------------------------------------------------------

float height(vec2 p, float t, float mid)
{
    float h = sin(dot(p, vec2(1.0, 0.4)) * 3.0 + t * 1.2);
    h += sin(dot(p, vec2(-0.5, 1.0)) * 4.5 - t * 0.9);
    h += (0.4 + MITTEN_CHOP * mid) * sin(dot(p, vec2(0.3, -1.0)) * 7.0 + t * 1.9);
    return h * 0.33;
}
void mainImage(out vec4 fragColor, in vec2 fragCoord)
{
    vec2 uv = (2.0 * fragCoord - iResolution.xy) / iResolution.y * 2.0;
    float mid = texture(iChannel0, vec2(0.30, 0.25)).x;
    float t = iTime * WELLEN_TEMPO;

    vec2 e = vec2(0.03, 0.0);
    float h = height(uv, t, mid);
    vec3 n = normalize(vec3(height(uv - e.xy, t, mid) - height(uv + e.xy, t, mid),
                            height(uv - e.yx, t, mid) - height(uv + e.yx, t, mid),
                            0.4));
    // DIE Kristallisierung: Normale aufs Raster runden = flache Facetten
    n = normalize(floor(n * FACETTEN) / FACETTEN + 1e-4);

    vec3 l = normalize(vec3(0.5, 0.6, 0.7));
    float diff = max(dot(n, l), 0.0);
    float spec = pow(max(dot(reflect(-l, n), vec3(0.0, 0.0, 1.0)), 0.0), GLANZ);
    // Eisfarben: Facetten-Ton aus der Normalen-Richtung (jede Facette anders)
    vec3 eis = 0.5 + 0.5 * cos(n.x * 5.0 + n.y * 7.0 + vec3(4.0, 4.6, 5.2));
    vec3 col = mix(vec3(0.02, 0.08, 0.15), eis * vec3(0.4, 0.7, 0.9), diff);
    col += spec * vec3(1.0, 1.0, 0.95);
    fragColor = vec4(col, 1.0);
}
