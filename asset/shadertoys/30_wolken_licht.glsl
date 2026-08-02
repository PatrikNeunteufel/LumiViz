// 30 Wolken-Licht — Patrik Neunteufel (LumiViz, 2026). Eigenwerk.
// Shadertoy: iChannel0 = Music. VOLUMETRIK (durchleuchtete Wolkenschicht).
//
// IDEE: Ein echter Volumen-Marsch: entlang des Blickstrahls wird die
// FBM-Dichte aufsummiert; je Sample wird zusätzlich ein kurzer Schritt
// Richtung Sonne gegangen (Selbstabschattung). Der Bass verdichtet die
// Wolken, die Höhen lassen die Sonne gleißen.

// ---- STELLSCHRAUBEN --------------------------------------------------------
const int   SAMPLES      = 24;    // Volumen-Samples (Qualität vs. Kosten)
const float DICHTE_GRUND = 1.2;
const float DICHTE_BASS  = 0.8;
const float WOLKEN_TEMPO = 0.06;  // Zug der Wolken
const vec3  SONNE        = vec3(0.5, 0.35, 0.6); // Richtung (wird normalisiert)
const float STREUUNG     = 0.7;   // wie hell die Sonne durchscheint
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
    for (int i = 0; i < OKTAVEN; ++i) { v += a * vnoise(p); p *= 2.11; a *= 0.5; }
    return v;
}
// Wolkendichte an einem 3D-Punkt (Schicht y = 0..1; 2D-FBM + Höhenprofil)
float dichte(vec3 p, float basis)
{
    float profil = smoothstep(0.0, 0.3, p.y) * smoothstep(1.0, 0.6, p.y);
    float d = fbm(p.xz * 0.8 + iTime * WOLKEN_TEMPO) - 0.45;
    return clamp(d, 0.0, 1.0) * profil * basis;
}
void mainImage(out vec4 fragColor, in vec2 fragCoord)
{
    vec2 uv = (2.0 * fragCoord - iResolution.xy) / iResolution.y;
    float bass = texture(iChannel0, vec2(0.05, 0.25)).x;
    float treb = texture(iChannel0, vec2(0.70, 0.25)).x;
    vec3 sonne = normalize(SONNE);
    float basis = DICHTE_GRUND + DICHTE_BASS * bass;

    vec3 ro = vec3(0.0, 0.2, iTime * 0.3);
    vec3 rd = normalize(vec3(uv.x, uv.y + 0.35, 1.2));

    // Himmel hinter den Wolken
    vec3 col = mix(vec3(0.5, 0.65, 0.9), vec3(0.1, 0.2, 0.45),
                   clamp(rd.y * 1.5, 0.0, 1.0));
    col += pow(max(dot(rd, sonne), 0.0), 128.0) * vec3(1.2, 1.0, 0.7) *
           (1.0 + treb);

    // Volumen-Marsch durch die Schicht y ∈ [0.4, 1.4]
    float transmission = 1.0;
    vec3 wolke = vec3(0.0);
    for (int i = 0; i < SAMPLES; ++i)
    {
        float t = 1.0 + float(i) * 0.35;
        vec3 p = ro + rd * t;
        p.y = (p.y + 0.6);  // Schicht ins Sichtfeld heben
        float d = dichte(p, basis) * 0.30;
        if (d > 0.001)
        {
            // Licht-Sample: ein Schritt Richtung Sonne = Selbstabschattung
            float licht = dichte(p + sonne * 0.5, basis);
            float hell = exp(-licht * 2.5) * STREUUNG + 0.15;
            wolke += transmission * d * vec3(hell);
            transmission *= exp(-d * 1.8);
            if (transmission < 0.05) break;
        }
    }
    col = col * transmission + wolke * vec3(1.0, 0.97, 0.92) * 2.0;
    fragColor = vec4(col, 1.0);
}
