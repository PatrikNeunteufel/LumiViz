// 24 Ozean-Weite — Patrik Neunteufel (LumiViz, 2026). Eigenwerk.
// Shadertoy: iChannel0 = Music. HEIGHTFIELD-RAYMARCHING (3D-Ozean + Himmel).
//
// IDEE: Ein Höhenfeld aus FBM-Wellen wird per Raymarch geschnitten (fester
// Schritt + Verfeinerung), die Normale beleuchtet das Wasser, der Himmel
// mit Sonne spiegelt sich (Fresnel). Bass = Wellengang, Mitten = Chop.

// ---- STELLSCHRAUBEN --------------------------------------------------------
const float WELLEN_HOEHE  = 0.35;  // Grund-Amplitude
const float WELLEN_BASS   = 0.30;  // Bass-Zuschlag auf den Wellengang
const float WELLEN_TEMPO  = 0.9;
const int   OKTAVEN       = 4;     // FBM-Detail der Wellen
const int   MARSCH        = 64;    // Sichtweite/Qualität
const float FLUG_TEMPO    = 1.2;   // Vorwärtsfahrt
const vec3  SONNE         = vec3(0.4, 0.25, 0.6); // Richtung (wird normalisiert)
// ----------------------------------------------------------------------------

float g_amp;
float n21(vec2 p) { return fract(sin(dot(p, vec2(41.3, 289.1))) * 43758.5453); }
float vnoise(vec2 p)
{
    vec2 i = floor(p), f = fract(p);
    f = f * f * (3.0 - 2.0 * f);
    return mix(mix(n21(i), n21(i + vec2(1.0, 0.0)), f.x),
               mix(n21(i + vec2(0.0, 1.0)), n21(i + vec2(1.0, 1.0)), f.x), f.y);
}
// Wellenhöhe am Punkt xz (FBM, zeitbewegt — zwei Richtungen gegeneinander)
float wellen(vec2 p)
{
    float h = 0.0, a = 1.0;
    vec2 q = p;
    for (int i = 0; i < OKTAVEN; ++i)
    {
        h += a * vnoise(q + iTime * WELLEN_TEMPO * vec2(0.6, 0.3) * (1.0 + 0.3 * float(i)));
        q = q * 2.1 + vec2(3.7);
        a *= 0.45;
    }
    return (h - 1.0) * g_amp;  // um 0 zentrieren
}
vec3 himmel(vec3 rd, vec3 sonne)
{
    float grau = pow(max(rd.y, 0.0), 0.5);
    vec3 sky = mix(vec3(0.75, 0.55, 0.35), vec3(0.15, 0.35, 0.6), grau);
    float s = pow(max(dot(rd, sonne), 0.0), 256.0);
    return sky + s * vec3(1.2, 1.0, 0.8);  // Sonnen-Glare
}
void mainImage(out vec4 fragColor, in vec2 fragCoord)
{
    vec2 uv = (2.0 * fragCoord - iResolution.xy) / iResolution.y;
    float bass = texture(iChannel0, vec2(0.05, 0.25)).x;
    g_amp = WELLEN_HOEHE + WELLEN_BASS * bass;
    vec3 sonne = normalize(SONNE);

    vec3 ro = vec3(0.0, 1.2, iTime * FLUG_TEMPO);
    vec3 rd = normalize(vec3(uv.x, uv.y - 0.15, 1.4));

    vec3 col;
    if (rd.y > 0.02)  // Blick über den Horizont: nur Himmel
    {
        col = himmel(rd, sonne);
    }
    else
    {
        // Raymarch gegen das Höhenfeld: fester Schritt, dann halbieren
        float t = 0.0;
        float dt = 0.4;
        float hDiff = 0.0;
        for (int i = 0; i < MARSCH; ++i)
        {
            vec3 p = ro + rd * t;
            hDiff = p.y - wellen(p.xz);
            if (hDiff < 0.01 || t > 40.0) break;
            t += max(dt, hDiff * 0.8);  // adaptiv: hoch überm Wasser = große Schritte
        }
        vec3 p = ro + rd * t;
        // Normale aus Höhen-Differenzen
        vec2 e = vec2(0.05, 0.0);
        vec3 n = normalize(vec3(wellen(p.xz - e.xy) - wellen(p.xz + e.xy),
                                2.0 * e.x,
                                wellen(p.xz - e.yx) - wellen(p.xz + e.yx)));
        float fresnel = pow(1.0 - max(dot(n, -rd), 0.0), 3.0);
        vec3 tiefe = mix(vec3(0.02, 0.12, 0.18), vec3(0.05, 0.3, 0.35),
                         max(dot(n, sonne), 0.0));
        vec3 spiegel = himmel(reflect(rd, n), sonne);
        col = mix(tiefe, spiegel, 0.25 + 0.75 * fresnel);
        col = mix(col, himmel(rd, sonne), smoothstep(20.0, 40.0, t));  // Dunst
    }
    fragColor = vec4(col, 1.0);
}
