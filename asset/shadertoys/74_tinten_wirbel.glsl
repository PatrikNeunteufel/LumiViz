// 74 Tinten-Wirbel — Patrik Neunteufel (LumiViz, 2026). Eigenwerk.
// Shadertoy: iChannel0 = Music. GLOW: Domain-Warping (iq-Technik).
//
// IDEE: FBM, dessen KOORDINATEN selbst von FBM verschoben werden
// (fbm(p + fbm(p + fbm(p)))) — daraus entstehen die typischen fließenden
// Tintenschlieren. Zwei Warp-Ebenen färben getrennt; die Lautstärke rührt
// das Feld, Glanzlichter sitzen auf den Schlieren-Kämmen.

// ---- STELLSCHRAUBEN --------------------------------------------------------
const float SKALA      = 2.2;
const float WARP_1     = 1.6;   // Stärke der ersten Verschiebung
const float WARP_2     = 1.2;   //  … zweiten
const float FLIESSEN   = 0.10;  // Grund-Fließtempo
const float RUEHREN_VOL = 0.25;
const int   OKTAVEN    = 5;
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
    for (int i = 0; i < OKTAVEN; ++i) { v += a * vnoise(p); p *= 2.03; a *= 0.5; }
    return v;
}
void mainImage(out vec4 fragColor, in vec2 fragCoord)
{
    vec2 uv = (2.0 * fragCoord - iResolution.xy) / iResolution.y * SKALA;
    float vol = texture(iChannel0, vec2(0.15, 0.25)).x;
    float t = iTime * (FLIESSEN + RUEHREN_VOL * vol);

    // Domain-Warping in zwei Stufen (q und w sind die Zwischenfelder)
    vec2 q = vec2(fbm(uv + vec2(0.0, 0.0) + t * vec2(0.3, 0.2)),
                  fbm(uv + vec2(5.2, 1.3) - t * vec2(0.2, 0.3)));
    vec2 w = vec2(fbm(uv + WARP_1 * q + vec2(1.7, 9.2) + t * vec2(0.15, 0.1)),
                  fbm(uv + WARP_1 * q + vec2(8.3, 2.8) - t * vec2(0.1, 0.15)));
    float f = fbm(uv + WARP_2 * w);

    // Färbung: die Zwischenfelder q/w mischen die Tinten
    vec3 tinte1 = vec3(0.08, 0.12, 0.35);
    vec3 tinte2 = vec3(0.55, 0.10, 0.35);
    vec3 tinte3 = vec3(0.05, 0.45, 0.45);
    vec3 col = mix(tinte1, tinte2, clamp(q.x * 1.6, 0.0, 1.0));
    col = mix(col, tinte3, clamp(w.y * 1.4, 0.0, 1.0));
    col *= 0.4 + 1.4 * f;
    // Glanz auf den Kämmen (hohe f-Werte gleißen)
    col += smoothstep(0.62, 0.75, f) * vec3(0.9, 0.85, 0.7) * (0.3 + 0.5 * vol);
    fragColor = vec4(col, 1.0);
}
