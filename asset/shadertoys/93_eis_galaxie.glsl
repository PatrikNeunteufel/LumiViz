// 93 Eis-Galaxie — Patrik Neunteufel (LumiViz, 2026). Eigenwerk.
// Shadertoy: iChannel0 = Music. KOMBI: Schneeflocke × Galaxie — 6-fach
// gefaltete SPIRALARME: eine Galaxie mit Kristallsymmetrie, die Arme
// schimmern wie Eis (wanderndes Lichtband). Höhen = Glitzerstaub.

// ---- STELLSCHRAUBEN --------------------------------------------------------
const float SPIRAL_ENGE = 4.0;
const float DREH_TEMPO  = 0.06;
const float ARM_SCHAERFE = 3.0;   // Kontrast der Arme
const float SCHIMMER_TEMPO = 1.2;
const float GLITZER     = 0.996;
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
    for (int i = 0; i < OKTAVEN; ++i) { v += a * vnoise(p); p *= 2.07; a *= 0.5; }
    return v;
}
void mainImage(out vec4 fragColor, in vec2 fragCoord)
{
    vec2 uv = (2.0 * fragCoord - iResolution.xy) / iResolution.y;
    float treb = texture(iChannel0, vec2(0.70, 0.25)).x;
    float r = length(uv) + 1e-4;
    float a = atan(uv.y, uv.x) - iTime * DREH_TEMPO;
    // 6-fach-Faltung des Winkels (Schneeflocken-Symmetrie)
    a = abs(mod(a, 1.0472) - 0.5236);

    // Spiralarm in der gefalteten Welt
    float arm = pow(0.5 + 0.5 * cos(a * 6.0 + SPIRAL_ENGE * log(r)), ARM_SCHAERFE);
    float nebel = fbm(vec2(a * 4.0, r * 5.0 - iTime * 0.05)) * arm;
    nebel *= smoothstep(1.3, 0.15, r);
    // Eis-Schimmer: radiales Lichtband über die Arme
    float schimmer = 0.6 + 0.4 * sin(r * 12.0 - iTime * SCHIMMER_TEMPO);
    vec3 col = vec3(0.005, 0.008, 0.02);
    col += nebel * mix(vec3(0.3, 0.55, 0.9), vec3(0.8, 0.9, 1.0), schimmer) * 1.5;
    col += exp(-r * 5.0) * vec3(0.85, 0.95, 1.0) * 0.8;  // Eiskern
    // Glitzerstaub auf den Armen
    float glitzer = step(GLITZER, n21(floor(uv * 110.0) + floor(iTime * 5.0)));
    col += glitzer * nebel * (0.5 + treb) * 2.0;
    fragColor = vec4(col, 1.0);
}
