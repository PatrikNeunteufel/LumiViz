// 07 Reaktions-Diffusion, Buffer A — Patrik Neunteufel (LumiViz, 2026). Eigenwerk.
// Shadertoy-Tab "Buffer A": iChannel0 = Buffer A (SELBST = Vorframe!),
//                           iChannel1 = Music.
//
// IDEE: Gray-Scott: zwei "Chemikalien" A (.r, Nährstoff) und B (.g, Muster).
// Beide diffundieren (Laplace), A+2B→3B, A wird nachgefüllt (FEED), B
// zerfällt (KILL). Braucht 1–2 Minuten zum Entfalten!

// ---- STELLSCHRAUBEN --------------------------------------------------------
const float FEED       = 0.052; // DIE Musterwahl: 0.055/0.062 Flecken,
const float KILL       = 0.062; // 0.029/0.057 Streifen, 0.026/0.051 Chaos
const float FEED_BASS  = 0.012; // Bass-Modulation von FEED
const float DIFF_A     = 1.0;   // Diffusionsrate A (Verhältnis ~2:1 zu B lassen)
const float DIFF_B     = 0.5;   // Diffusionsrate B
const float SAAT_RADIUS = 0.05; // Start-Fleck in der Mitte
const float INJEKT_STAERKE = 0.5; // Nachsäen bei Bass-Schlägen
const float INJEKT_RADIUS  = 0.02;
// ----------------------------------------------------------------------------

void mainImage(out vec4 fragColor, in vec2 fragCoord)
{
    vec2 uv = fragCoord / iResolution.xy;
    vec2 px = 1.0 / iResolution.xy;

    // Frame 0/1: Startzustand — überall A=1, Keimfleck mit B=1 in der Mitte
    if (iFrame < 2)
    {
        float seed = step(length((uv - 0.5) * vec2(iResolution.x / iResolution.y, 1.0)),
                          SAAT_RADIUS);
        fragColor = vec4(1.0, seed, 0.0, 1.0);
        return;
    }

    vec2 ab = texture(iChannel0, uv).rg;
    // Laplace: 4 Nachbarn − 4×Mitte = Abweichung von der Umgebung (Diffusion)
    vec2 lap = texture(iChannel0, uv + vec2(px.x, 0.0)).rg
             + texture(iChannel0, uv - vec2(px.x, 0.0)).rg
             + texture(iChannel0, uv + vec2(0.0, px.y)).rg
             + texture(iChannel0, uv - vec2(0.0, px.y)).rg
             - 4.0 * ab;

    float bass = texture(iChannel1, vec2(0.05, 0.25)).x;
    float feed = FEED + FEED_BASS * bass;
    float rxn = ab.x * ab.y * ab.y;  // Reaktionsrate A·B²

    float a = ab.x + DIFF_A * lap.x - rxn + feed * (1.0 - ab.x);
    float b = ab.y + DIFF_B * lap.y + rxn - (KILL + feed) * ab.y;

    // Wandernder Injektor: bass² = nur echte Schläge säen nach
    vec2 p = 0.5 + 0.35 * vec2(cos(iTime * 0.7), sin(iTime * 1.1));
    b += INJEKT_STAERKE * smoothstep(INJEKT_RADIUS, 0.0, length(uv - p)) *
         bass * bass;

    fragColor = vec4(clamp(a, 0.0, 1.0), clamp(b, 0.0, 1.0), 0.0, 1.0);
}
