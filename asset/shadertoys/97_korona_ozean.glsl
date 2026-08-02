// 97 Korona-Ozean — Patrik Neunteufel (LumiViz, 2026). Eigenwerk.
// Shadertoy: iChannel0 = Music. KOMBI: Sonnen-Korona × Ozean — eine
// brodelnde Sonne versinkt im Meer; die Wellen tragen ihre Spiegelung
// als glitzernde Lichtstraße. Bass = Brodeln, Mitten = Wellengang.

// ---- STELLSCHRAUBEN --------------------------------------------------------
const vec2  SONNE_POS   = vec2(0.0, 0.28);
const float SONNE_RADIUS = 0.30;
const float HORIZONT    = 0.0;
const float WELLEN      = 0.5;
const float LICHTSTRASSE = 0.25;  // Breite der Spiegelstraße
const int   OKTAVEN     = 4;
// ----------------------------------------------------------------------------

float n21(vec2 p) { return fract(sin(dot(p, vec2(41.3, 289.1))) * 43758.5453); }
float vnoise(vec2 p)
{
    vec2 i = floor(p), f = fract(p);
    f = f * f * (3.0 - 2.0 * f);
    return mix(mix(n21(i), n21(i + vec2(1.0, 0.0)), f.x),
               mix(n21(i + vec2(0.0, 1.0)), n21(i + vec2(1.0, 1.0)), f.x), f.y);
}
float fbm(vec2 p)
{
    float v = 0.0, a = 0.5;
    for (int i = 0; i < OKTAVEN; ++i) { v += a * vnoise(p); p *= 2.09; a *= 0.5; }
    return v;
}
void mainImage(out vec4 fragColor, in vec2 fragCoord)
{
    vec2 uv = (2.0 * fragCoord - iResolution.xy) / iResolution.y;
    float bass = texture(iChannel0, vec2(0.05, 0.25)).x;
    float mid  = texture(iChannel0, vec2(0.30, 0.25)).x;
    vec3 col;

    if (uv.y > HORIZONT)
    {
        // Abendhimmel + Sonne mit brodelnder Oberfläche und Korona
        col = mix(vec3(0.85, 0.4, 0.2), vec3(0.2, 0.1, 0.35),
                  clamp(uv.y * 1.6, 0.0, 1.0));
        vec2 d = uv - SONNE_POS;
        float r = length(d);
        if (r < SONNE_RADIUS)
        {
            float a = atan(d.y, d.x);
            float brodel = fbm(vec2(a * 3.0 + iTime * 0.1,
                                    r * 8.0 - iTime * (0.5 + bass)));
            col = mix(vec3(1.0, 0.5, 0.1), vec3(1.0, 0.9, 0.5), brodel);
        }
        else
        {
            col += exp(-(r - SONNE_RADIUS) * 4.0) * vec3(1.0, 0.5, 0.15) *
                   (0.6 + 0.3 * bass);
        }
    }
    else
    {
        // Meer: Streifenwellen + Lichtstraße unter der Sonne
        float tiefe = -uv.y;
        float welle = fbm(vec2(uv.x * 6.0, tiefe * 18.0 - iTime * (1.0 + WELLEN * mid)));
        col = mix(vec3(0.06, 0.08, 0.16), vec3(0.15, 0.12, 0.25), welle * 0.6);
        // Lichtstraße: schmaler x-Korridor unter der Sonne, Wellen brechen ihn
        float strasse = exp(-pow((uv.x - SONNE_POS.x) / (LICHTSTRASSE * (0.5 + tiefe)), 2.0));
        float glitzer = smoothstep(0.45, 0.75, welle);
        col += strasse * (0.35 + 0.65 * glitzer) * vec3(1.0, 0.55, 0.2) *
               (0.6 + 0.4 * bass) * smoothstep(1.0, 0.0, tiefe);
    }
    fragColor = vec4(col, 1.0);
}
