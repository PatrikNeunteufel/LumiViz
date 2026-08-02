// 91 Sternen-Feuerwerk, Image — Patrik Neunteufel (LumiViz, 2026). Eigenwerk.
// "Image": iChannel0 = Buffer A. Anzeige + Vignette.

void mainImage(out vec4 fragColor, in vec2 fragCoord)
{
    vec2 uv = fragCoord / iResolution.xy;
    vec3 c = texture(iChannel0, uv).rgb;
    c *= 1.0 - 0.4 * pow(length(uv - 0.5) * 1.3, 2.0);
    fragColor = vec4(c + vec3(0.004, 0.004, 0.012), 1.0);
}
