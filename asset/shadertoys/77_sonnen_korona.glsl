// 77 Sonnen-Korona — Patrik Neunteufel (LumiViz, 2026). Eigenwerk.
// Shadertoy: iChannel0 = Music. GLOW: brodelnde Sonne mit Protuberanzen.
//
// IDEE: Die Scheibe brodelt (FBM in Polar, rotierend), die Korona besteht
// aus radialen FBM-Filamenten (Winkel-Noise moduliert die Reichweite).
// Protuberanzen = einzelne Bögen, die mit dem Bass aufsteigen.

// ---- STELLSCHRAUBEN --------------------------------------------------------
const float SONNE_RADIUS = 0.42;
const float KORONA_WEITE = 0.6;   // Reichweite der Filamente
const float BRODELN      = 0.6;
const float BASS_ERUPTION = 0.6;  // Bass-Schwelle für Protuberanzen
const int   OKTAVEN      = 4;
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
    float r = length(uv);
    float a = atan(uv.y, uv.x);
    vec3 col = vec3(0.01, 0.005, 0.01);

    if (r < SONNE_RADIUS)
    {
        // brodelnde Oberfläche: FBM in rotierenden Polarkoordinaten
        float brodel = fbm(vec2(a * 3.0 + iTime * 0.1, r * 8.0 - iTime * BRODELN));
        col = mix(vec3(1.0, 0.45, 0.05), vec3(1.0, 0.9, 0.4), brodel);
        // Randverdunkelung wie bei echten Sternen
        col *= 0.75 + 0.25 * sqrt(1.0 - r / SONNE_RADIUS);
    }
    else
    {
        // Korona-Filamente: Reichweite je Winkel aus FBM
        float reichweite = KORONA_WEITE * (0.4 + 0.6 * fbm(vec2(a * 2.5, iTime * 0.15)));
        float filament = exp(-(r - SONNE_RADIUS) / max(reichweite, 1e-3) * 3.0);
        col += filament * vec3(1.0, 0.6, 0.2) * (0.5 + 0.3 * bass);
        // Protuberanz: ein Bogen bei wanderndem Winkel, zündet mit dem Bass
        float pw = iTime * 0.2;
        float bogen = exp(-pow((a - pw - sin(pw) * 2.0) * 3.0, 2.0)) *
                      exp(-pow((r - SONNE_RADIUS - 0.12 * (0.5 + bass)) * 12.0, 2.0));
        col += bogen * vec3(1.0, 0.5, 0.15) * smoothstep(BASS_ERUPTION, 1.0, bass) * 2.0;
    }
    // Gesamt-Glare
    col += exp(-max(r - SONNE_RADIUS, 0.0) * 5.0) * vec3(0.4, 0.2, 0.05) * 0.5;
    fragColor = vec4(col, 1.0);
}
