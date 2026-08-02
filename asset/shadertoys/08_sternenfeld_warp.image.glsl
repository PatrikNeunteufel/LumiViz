// 08 Sternenfeld-Warp, Image — Patrik Neunteufel (LumiViz, 2026). Eigenwerk.
// Shadertoy-Tab "Image": iChannel0 = Buffer A. Anzeige + Vignette.

// ---- STELLSCHRAUBEN --------------------------------------------------------
const float VIGNETTE = 0.5;  // Randabdunklung (0 = aus)
// ----------------------------------------------------------------------------

void mainImage(out vec4 fragColor, in vec2 fragCoord)
{
    vec2 uv = fragCoord / iResolution.xy;
    vec3 c = texture(iChannel0, uv).rgb;
    c *= 1.0 - VIGNETTE * pow(length(uv - 0.5), 2.0);
    fragColor = vec4(c, 1.0);
}
