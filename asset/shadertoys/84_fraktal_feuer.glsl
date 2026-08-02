// 84 Fraktal-Feuer — Patrik Neunteufel (LumiViz, 2026). Eigenwerk.
// Shadertoy: iChannel0 = Music. KOMBI: Julia-Fraktal × Feuer — die
// Fraktal-Silhouette BRENNT: FBM-Flammen ziehen aus dem Julia-Rand nach
// oben, der Rand selbst glüht als Glutlinie. Bass facht an.

// ---- STELLSCHRAUBEN --------------------------------------------------------
const float ZOOM       = 1.5;
const int   JULIA_ITER = 48;
const float FLAMMEN_HOEHE = 0.35; // wie weit die Flammen überm Rand züngeln
const float ZUG_TEMPO  = 1.4;
const int   OKTAVEN    = 4;
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
// glatter Julia-"Abstand": 0 innen, wächst außen
float julia(vec2 uv)
{
    vec2 z = uv;
    vec2 c = vec2(-0.62, 0.43) + 0.02 * vec2(sin(iTime * 0.1), cos(iTime * 0.13));
    float m = 0.0;
    for (int i = 0; i < JULIA_ITER; ++i)
    {
        z = vec2(z.x * z.x - z.y * z.y, 2.0 * z.x * z.y) + c;
        if (dot(z, z) > 16.0) break;
        m += 1.0;
    }
    return 1.0 - m / float(JULIA_ITER);  // 0 = tief innen, 1 = weit außen
}
void mainImage(out vec4 fragColor, in vec2 fragCoord)
{
    vec2 uv = (2.0 * fragCoord - iResolution.xy) / iResolution.y * ZOOM;
    float bass = texture(iChannel0, vec2(0.05, 0.25)).x;

    float rand = julia(uv);
    // Flammen: die Julia-Randnähe wird mit aufsteigendem FBM moduliert —
    // Flammen "wachsen" aus dem Rand nach oben
    float flammenZone = julia(uv + vec2(0.0, -FLAMMEN_HOEHE *
                                        fbm(vec2(uv.x * 4.0,
                                                 uv.y * 2.0 - iTime * ZUG_TEMPO))));
    float f = clamp((1.0 - flammenZone * 2.2) * (1.1 + 1.2 * bass), 0.0, 1.0);
    vec3 col = vec3(f * 1.5, f * f * 1.1, f * f * f * 0.6);  // Feuer-Rampe
    // Kernkörper: dunkel verkohlt mit glühendem Rand
    float koerper = smoothstep(0.55, 0.5, rand);
    col = mix(col, vec3(0.03, 0.02, 0.02), koerper);
    col += smoothstep(0.06, 0.0, abs(rand - 0.5)) *
           vec3(1.0, 0.5, 0.1) * (0.7 + bass);  // Glutlinie am Rand
    fragColor = vec4(col, 1.0);
}
