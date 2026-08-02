// 46 Feuerwerk-Nacht, Image — Patrik Neunteufel (LumiViz, 2026). Eigenwerk.
// "Image": iChannel0 = Buffer A. Nachthimmel + Stadtsilhouette unten,
// darüber die Feuerwerks-Spuren mit Glow.

float n21(vec2 p) { return fract(sin(dot(p, vec2(41.3, 289.1))) * 43758.5453); }
void mainImage(out vec4 fragColor, in vec2 fragCoord)
{
    vec2 uv = fragCoord / iResolution.xy;
    vec3 c = texture(iChannel0, uv).rgb;
    // Glow (4 Taps)
    float r = 0.004;
    c += 0.3 * 0.25 * (texture(iChannel0, uv + vec2(r, 0.0)).rgb +
                       texture(iChannel0, uv - vec2(r, 0.0)).rgb +
                       texture(iChannel0, uv + vec2(0.0, r)).rgb +
                       texture(iChannel0, uv - vec2(0.0, r)).rgb);
    // Nachthimmel + Silhouette
    vec3 himmel = mix(vec3(0.02, 0.02, 0.06), vec3(0.0, 0.0, 0.02), uv.y);
    float dach = 0.06 + 0.05 * n21(vec2(floor(uv.x * 30.0), 0.0));
    if (uv.y < dach) himmel = vec3(0.005);
    fragColor = vec4(himmel + c, 1.0);
}
