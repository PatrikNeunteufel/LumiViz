// 63 Oszilloskop-Phosphor — Patrik Neunteufel (LumiViz, 2026). Eigenwerk.
// Shadertoy: iChannel0 = Music. TECHNIK: XY-Oszilloskop mit Waveform.
//
// IDEE: Die ECHTE Audio-Waveform (Zeile y=0.75 der Musik-Textur) wird als
// Kurve gezeichnet: für jedes Pixel wird die Welle an mehreren x-Stellen
// gesampelt und der minimale Abstand zur Kurve bestimmt — daraus Strahl +
// Phosphor-Glow auf Gitterschirm. Höhen färben den Strahl weißer.

// ---- STELLSCHRAUBEN --------------------------------------------------------
const int   KURVEN_SAMPLES = 48;  // Abtastung der Kurve (mehr = glatter)
const float AMPLITUDE   = 0.55;  // Darstellungs-Verstärkung der Welle
const float STRAHL_KERN = 0.006;
const float PHOSPHOR    = 0.05;  // Glow-Weite
const float GITTER      = 8.0;   // Rasterlinien
// ----------------------------------------------------------------------------

void mainImage(out vec4 fragColor, in vec2 fragCoord)
{
    vec2 uv = (2.0 * fragCoord - iResolution.xy) / iResolution.y;
    float aspekt = iResolution.x / iResolution.y;
    float treb = texture(iChannel0, vec2(0.70, 0.25)).x;

    // minimaler Abstand Pixel ↔ Waveform-Kurve
    float dMin = 1e9;
    for (int i = 0; i < KURVEN_SAMPLES; ++i)
    {
        float fx = float(i) / float(KURVEN_SAMPLES - 1);       // 0..1
        float x = (fx * 2.0 - 1.0) * aspekt;                    // Bildbreite
        float w = texture(iChannel0, vec2(fx, 0.75)).x - 0.5;   // Waveform ±0.5
        float y = w * 2.0 * AMPLITUDE;
        dMin = min(dMin, length(uv - vec2(x, y)));
    }
    float kern = smoothstep(STRAHL_KERN, STRAHL_KERN * 0.3, dMin);
    float glow = exp(-dMin / PHOSPHOR);

    // Schirm: dunkelgrün mit Raster
    vec3 col = vec3(0.01, 0.03, 0.015);
    vec2 raster = abs(fract(uv * GITTER * 0.5 + 0.5) - 0.5);
    col += smoothstep(0.02, 0.0, min(raster.x, raster.y) / GITTER) *
           vec3(0.0, 0.1, 0.04);
    // Strahl + Phosphor
    vec3 strahl = mix(vec3(0.2, 1.0, 0.4), vec3(0.9, 1.0, 0.9), treb);
    col += glow * strahl * 0.35 + kern * strahl;
    fragColor = vec4(col, 1.0);
}
