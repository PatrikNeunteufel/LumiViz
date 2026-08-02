// 78 Glühwürmchen — Patrik Neunteufel (LumiViz, 2026). Eigenwerk.
// Shadertoy: iChannel0 = Music. GLOW: nächtliche Wiese voller Leuchtkäfer.
//
// IDEE: Punkte auf sanften Zufallsbahnen; jedes Würmchen BLINKT mit eigener
// Phase (weiches An/Aus über pow(sin)) — mehr Musik = mehr Käfer leuchten
// gleichzeitig. Unten Gras-Silhouetten (gefaltete sin-Spitzen), oben Mond.

// ---- STELLSCHRAUBEN --------------------------------------------------------
const int   KAEFER     = 40;
const float BLINK_TEMPO = 0.8;
const float GROESSE    = 0.006;
const float VOL_MEHR   = 2.0;   // Lautstärke weckt mehr Käfer
const float GRAS_HOEHE = 0.25;
// ----------------------------------------------------------------------------

float h1(float n) { return fract(sin(n) * 43758.5453); }
void mainImage(out vec4 fragColor, in vec2 fragCoord)
{
    vec2 uv = (2.0 * fragCoord - iResolution.xy) / iResolution.y;
    float vol = texture(iChannel0, vec2(0.15, 0.25)).x;

    // Nacht + Mond
    vec3 col = mix(vec3(0.01, 0.015, 0.04), vec3(0.0, 0.005, 0.02), uv.y + 0.5);
    vec2 mond = vec2(0.75, 0.55);
    col += exp(-dot(uv - mond, uv - mond) * 40.0) * vec3(0.8, 0.85, 0.9) * 0.5;
    col += exp(-dot(uv - mond, uv - mond) * 6.0) * vec3(0.3, 0.35, 0.45) * 0.25;

    // Käfer
    for (int i = 0; i < KAEFER; ++i)
    {
        float fi = float(i);
        // Bahn: langsame Lissajous-Drift je Käfer
        vec2 p = vec2(1.5 * (h1(fi * 3.1) - 0.5) * 2.0 +
                          0.1 * sin(iTime * (0.3 + h1(fi) * 0.4) + fi),
                      -0.7 + 1.1 * h1(fi * 7.7) +
                          0.06 * sin(iTime * (0.4 + h1(fi * 5.0) * 0.5) + fi * 2.0));
        // Blinken: weiche Pulse, Phase je Käfer; Schwelle sinkt mit `vol`
        float blink = pow(0.5 + 0.5 * sin(iTime * BLINK_TEMPO * 6.28318 *
                                          (0.3 + 0.7 * h1(fi * 9.1)) + fi * 2.7),
                          8.0);
        blink *= step(1.0 - 0.25 - VOL_MEHR * vol * 0.25, h1(fi * 11.3) + blink);
        float d = length(uv - p);
        vec3 leuchten = mix(vec3(0.7, 1.0, 0.25), vec3(1.0, 0.9, 0.4), h1(fi));
        col += smoothstep(GROESSE, 0.0, d) * leuchten * blink;
        col += exp(-d * 90.0) * leuchten * blink * 0.5;  // Halo
    }
    // Gras-Silhouette: gefaltete sin-Spitzen, zwei Ebenen
    for (int g = 0; g < 2; ++g)
    {
        float fg = float(g);
        float kante = -0.75 + GRAS_HOEHE *
                      abs(sin(uv.x * (14.0 + fg * 9.0) + fg * 2.0 +
                              0.15 * sin(iTime * 0.7 + fg)));
        if (uv.y < kante) col *= 0.15 + 0.1 * fg;
    }
    fragColor = vec4(col, 1.0);
}
