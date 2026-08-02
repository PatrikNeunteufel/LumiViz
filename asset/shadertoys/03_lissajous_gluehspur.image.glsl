// 03 Lissajous-Glühspur, Image — Patrik Neunteufel (LumiViz, 2026). Eigenwerk.
// Shadertoy-Tab "Image": iChannel0 = Buffer A.
//
// IDEE: Buffer anzeigen + billiger horizontaler Glow (versetzte Samples).

// ---- STELLSCHRAUBEN --------------------------------------------------------
const float GLOW_RADIUS = 0.003; // Versatz der Zusatz-Samples
const float GLOW_STAERKE = 0.35;
const float HELLIGKEIT  = 0.8;
// ----------------------------------------------------------------------------

void mainImage(out vec4 fragColor, in vec2 fragCoord)
{
    vec2 uv = fragCoord / iResolution.xy;
    vec3 c = texture(iChannel0, uv).rgb;
    c += GLOW_STAERKE * texture(iChannel0, uv + vec2(GLOW_RADIUS, 0.0)).rgb;
    c += GLOW_STAERKE * texture(iChannel0, uv - vec2(GLOW_RADIUS, 0.0)).rgb;
    fragColor = vec4(c * HELLIGKEIT, 1.0);
}
