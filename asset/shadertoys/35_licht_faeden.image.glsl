// 35 Licht-Fäden, Image — Patrik Neunteufel (LumiViz, 2026). Eigenwerk.
// Shadertoy-Tab "Image": iChannel0 = Buffer A. Anzeige + Bloom-Andeutung.

// ---- STELLSCHRAUBEN --------------------------------------------------------
const float BLOOM_RADIUS = 0.004;
const float BLOOM_STAERKE = 0.5;
// ----------------------------------------------------------------------------

void mainImage(out vec4 fragColor, in vec2 fragCoord)
{
    vec2 uv = fragCoord / iResolution.xy;
    vec3 c = texture(iChannel0, uv).rgb;
    // 4-Tap-Bloom: weiche Aura um die Fäden
    c += BLOOM_STAERKE * 0.25 *
         (texture(iChannel0, uv + vec2(BLOOM_RADIUS, 0.0)).rgb +
          texture(iChannel0, uv - vec2(BLOOM_RADIUS, 0.0)).rgb +
          texture(iChannel0, uv + vec2(0.0, BLOOM_RADIUS)).rgb +
          texture(iChannel0, uv - vec2(0.0, BLOOM_RADIUS)).rgb);
    fragColor = vec4(c * 0.85, 1.0);
}
