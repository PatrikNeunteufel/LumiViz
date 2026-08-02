// 38 Dünen-Dämmerung — Patrik Neunteufel (LumiViz, 2026). Eigenwerk.
// Shadertoy: iChannel0 = Music. TERRAIN-RAYMARCHING (Wüste im Abendlicht).
//
// IDEE: Ein Höhenfeld aus überlagerten, scharf gefalteten Sinuswellen
// (abs(sin) = Dünenkämme) + FBM-Ripples wird geraymarcht; Sonnenstand tief,
// warme Palette, langer Schattenwurf über dot(n, sonne). Die Mitten lassen
// den Sand "flimmern" (Ripple-Amplitude).

// ---- STELLSCHRAUBEN --------------------------------------------------------
const float DUENEN_HOEHE = 0.5;
const float RIPPLE       = 0.03;  // kleine Sandwellen
const float RIPPLE_MITTEN = 0.04; // Audio-Zuschlag auf die Ripples
const float FLUG_TEMPO   = 0.5;
const int   MARSCH       = 72;
const vec3  SONNE        = vec3(-0.55, 0.12, 0.3); // tiefer Abendstand
// ----------------------------------------------------------------------------

float g_ripple;
float n21(vec2 p) { return fract(sin(dot(p, vec2(41.3, 289.1))) * 43758.5453); }
float vnoise(vec2 p)
{
    vec2 i = floor(p), f = fract(p);
    f = f * f * (3.0 - 2.0 * f);
    return mix(mix(n21(i), n21(i + vec2(1.0, 0.0)), f.x),
               mix(n21(i + vec2(0.0, 1.0)), n21(i + vec2(1.0, 1.0)), f.x), f.y);
}
float duenen(vec2 p)
{
    // Dünenkämme: abs(sin) ist an der Spitze SCHARF — genau wie echte Kämme;
    // zwei Richtungen überlagert, Grundwellen leicht gedreht
    float h = DUENEN_HOEHE * (1.0 - abs(sin(p.x * 0.5 + sin(p.y * 0.3) * 0.8)));
    h += 0.5 * DUENEN_HOEHE * (1.0 - abs(sin(dot(p, vec2(0.35, 0.25)) + 1.7)));
    h += g_ripple * vnoise(p * 8.0);  // feine Sand-Ripples
    return h;
}
vec3 himmel(vec3 rd, vec3 sonne)
{
    vec3 sky = mix(vec3(0.9, 0.5, 0.25), vec3(0.25, 0.2, 0.45),
                   clamp(rd.y * 2.0, 0.0, 1.0));
    sky += pow(max(dot(rd, sonne), 0.0), 64.0) * vec3(1.3, 0.9, 0.5);
    return sky;
}
void mainImage(out vec4 fragColor, in vec2 fragCoord)
{
    vec2 uv = (2.0 * fragCoord - iResolution.xy) / iResolution.y;
    float mid = texture(iChannel0, vec2(0.30, 0.25)).x;
    g_ripple = RIPPLE + RIPPLE_MITTEN * mid;
    vec3 sonne = normalize(SONNE);

    vec3 ro = vec3(0.0, 1.1, iTime * FLUG_TEMPO);
    vec3 rd = normalize(vec3(uv.x, uv.y - 0.12, 1.4));

    vec3 col;
    if (rd.y > 0.05) { col = himmel(rd, sonne); }
    else
    {
        float t = 0.0;
        float hDiff = 0.0;
        for (int i = 0; i < MARSCH; ++i)
        {
            vec3 p = ro + rd * t;
            hDiff = p.y - duenen(p.xz);
            if (hDiff < 0.01 || t > 35.0) break;
            t += max(0.15, hDiff * 0.7);
        }
        vec3 p = ro + rd * t;
        vec2 e = vec2(0.05, 0.0);
        vec3 n = normalize(vec3(duenen(p.xz - e.xy) - duenen(p.xz + e.xy),
                                2.0 * e.x,
                                duenen(p.xz - e.yx) - duenen(p.xz + e.yx)));
        float licht = max(dot(n, sonne), 0.0);
        // Sand: Licht-/Schattenseite der Dünen deutlich getrennt
        vec3 sand = mix(vec3(0.35, 0.18, 0.15), vec3(1.0, 0.65, 0.35),
                        pow(licht, 0.7));
        col = mix(sand, himmel(rd, sonne), smoothstep(15.0, 35.0, t));  // Dunst
    }
    fragColor = vec4(col, 1.0);
}
