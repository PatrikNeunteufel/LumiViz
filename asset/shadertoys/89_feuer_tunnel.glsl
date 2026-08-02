// 89 Feuer-Tunnel — Patrik Neunteufel (LumiViz, 2026). Eigenwerk.
// Shadertoy: iChannel0 = Music. KOMBI: Tunnel × Feuer — die Tunnelwände
// sind brennendes FBM (Flammen ziehen ENTGEGEN der Fahrtrichtung), die
// Feuer-Rampe färbt; der Fluchtpunkt glüht wie ein Ofen. Bass = Schub.

// ---- STELLSCHRAUBEN --------------------------------------------------------
const float VORTRIEB    = 0.6;
const float VORTRIEB_BASS = 1.2;
const float TIEFE       = 0.4;
const float FLAMMEN_ZUG = 2.0;   // wie schnell die Flammen rückwärts ziehen
const float OFEN_GLUT   = 1.2;
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
    for (int i = 0; i < OKTAVEN; ++i) { v += a * vnoise(p); p *= 2.13; a *= 0.5; }
    return v;
}
void mainImage(out vec4 fragColor, in vec2 fragCoord)
{
    vec2 uv = (2.0 * fragCoord - iResolution.xy) / iResolution.y;
    float bass = texture(iChannel0, vec2(0.05, 0.25)).x;
    float r = length(uv) + 1e-4;
    float a = atan(uv.y, uv.x);
    float z = TIEFE / r + iTime * (VORTRIEB + VORTRIEB_BASS * bass);

    // Wand-Feuer: FBM über (Winkel × Tiefe), zieht nach hinten (+z-Richtung)
    float feuer = fbm(vec2(a * 1.59155 * 4.0, z * 0.8 + iTime * FLAMMEN_ZUG));
    feuer *= 0.6 + 0.7 * bass;
    // Feuer-Rampe + Tiefenstaffelung (fern = dunkler/röter)
    float fern = clamp(r * 1.4, 0.0, 1.0);
    float f = clamp(feuer * (0.5 + fern), 0.0, 1.0);
    vec3 col = vec3(f * 1.5, f * f * 1.05, f * f * f * 0.55);
    // Ofen-Fluchtpunkt: weißglühendes Zentrum
    col += exp(-r * 5.0) * vec3(1.0, 0.75, 0.4) * OFEN_GLUT * (0.7 + 0.6 * bass);
    col *= smoothstep(0.0, 0.05, r);
    fragColor = vec4(col, 1.0);
}
