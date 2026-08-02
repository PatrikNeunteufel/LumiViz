// 34 Wurmloch-Flug — Patrik Neunteufel (LumiViz, 2026). Eigenwerk.
// Shadertoy: iChannel0 = Music. VERZERRTER TUNNEL (FBM-Wände + Torsion).
//
// IDEE: Ein Tunnel, dessen Radius über Winkel und Tiefe von FBM verformt
// wird — die Wand ist organisch statt rund. Die ganze Röhre verdreht sich
// mit der Tiefe (Torsion), Tiefen-Nebel verschluckt das Ende. Bass = Sog,
// Höhen = Funkeln der Wandadern.

// ---- STELLSCHRAUBEN --------------------------------------------------------
const float SOG_GRUND    = 0.8;   // Vortrieb
const float SOG_BASS     = 1.4;
const float TORSION      = 0.35;  // Verdrehung pro Tiefeneinheit
const float WAND_WELLEN  = 1.2;   // Stärke der FBM-Wandverformung
const float ADER_SCHAERFE = 3.0;  // Kontrast der Wandadern
const float NEBEL        = 0.35;  // Tiefen-Nebel (größer = kürzerer Blick)
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
    for (int i = 0; i < OKTAVEN; ++i) { v += a * vnoise(p); p *= 2.05; a *= 0.5; }
    return v;
}
void mainImage(out vec4 fragColor, in vec2 fragCoord)
{
    vec2 uv = (2.0 * fragCoord - iResolution.xy) / iResolution.y;
    float bass = texture(iChannel0, vec2(0.05, 0.25)).x;
    float treb = texture(iChannel0, vec2(0.70, 0.25)).x;

    float r = length(uv) + 1e-4;
    float a = atan(uv.y, uv.x);
    float z = 1.0 / r + iTime * (SOG_GRUND + SOG_BASS * bass);  // Tiefe
    a += z * TORSION;  // Torsion: die Röhre verdreht sich mit der Tiefe

    // Wandkoordinaten (Winkel × Tiefe); FBM verformt den effektiven Radius
    vec2 wand = vec2(a * 1.59155, z * 0.5);  // 1/(2π)·10 ≈ nahtlos genug
    float form = fbm(wand);
    // Adern: zweites, feineres FBM, kontrastverstärkt
    float adern = pow(fbm(wand * 3.0 + 5.0), ADER_SCHAERFE) * 3.0;

    // Farbe: Tiefe + Winkel durch die Palette, Adern glühen türkis-violett
    vec3 col = (0.5 + 0.5 * cos(z * 0.9 + a + vec3(0.0, 2.1, 4.2))) *
               (0.35 + 0.65 * form);
    col += adern * mix(vec3(0.1, 0.9, 0.8), vec3(0.7, 0.3, 1.0),
                       0.5 + 0.5 * sin(z * 0.5)) * (0.4 + 0.8 * treb);
    // Tiefen-Nebel: 1/r wächst zur Mitte — dort verschwindet der Tunnel
    col *= exp(-NEBEL / r * 0.35);
    col *= smoothstep(0.0, 0.1, r);  // Zentrum sauber schwarz
    fragColor = vec4(col, 1.0);
}
