// 44 Kristall-Zucht, Image — Patrik Neunteufel (LumiViz, 2026). Eigenwerk.
// "Image": iChannel0 = Buffer A. Die Geburtszeit (.g) wird als
// SCHIMMERNDE Interferenzfarbe gerendert — ein Lichtband wandert über die
// Wachstumsringe; dazu Eis-Glow um die Nadeln.

// ---- STELLSCHRAUBEN --------------------------------------------------------
const float SCHIMMER_TEMPO = 1.2; // wandnerndes Lichtband
const float GLOW_RADIUS    = 0.003;
// ----------------------------------------------------------------------------

void mainImage(out vec4 fragColor, in vec2 fragCoord)
{
    vec2 uv = fragCoord / iResolution.xy;
    vec4 z = texture(iChannel0, uv);
    vec3 col = vec3(0.01, 0.015, 0.03);
    if (z.r > 0.5)
    {
        // Interferenz über die Geburtszeit + wanderndes Schimmer-Band
        float ring = z.g * 40.0;
        float schimmer = 0.6 + 0.4 * sin(ring * 6.28318 - iTime * SCHIMMER_TEMPO);
        col = (0.5 + 0.5 * cos(ring + vec3(0.0, 2.1, 4.2))) *
              mix(vec3(0.5, 0.8, 1.0), vec3(1.0), schimmer) * 0.9;
    }
    // Eisglow um die Nadeln (4 Nachbar-Samples)
    float halo = texture(iChannel0, uv + vec2(GLOW_RADIUS, 0.0)).r +
                 texture(iChannel0, uv - vec2(GLOW_RADIUS, 0.0)).r +
                 texture(iChannel0, uv + vec2(0.0, GLOW_RADIUS)).r +
                 texture(iChannel0, uv - vec2(0.0, GLOW_RADIUS)).r;
    col += halo * 0.08 * vec3(0.4, 0.7, 1.0);
    fragColor = vec4(col, 1.0);
}
