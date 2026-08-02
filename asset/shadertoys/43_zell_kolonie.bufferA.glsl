// 43 Zell-Kolonie, Buffer A — Patrik Neunteufel (LumiViz, 2026). Eigenwerk.
// "Buffer A": iChannel0 = Buffer A (SELBST!), iChannel1 = Music.
//
// IDEE: "Larger than Life"-Ableger: die Nachbarschaft ist ein 5×5-Feld,
// Geburt/Überleben in BÄNDERN statt exakten Zahlen — statt Pixelgewimmel
// entstehen organische, amöbenhafte Kolonien mit glatten Rändern.

// ---- STELLSCHRAUBEN --------------------------------------------------------
const float ZELLGROESSE = 2.0;
const float GEBURT_MIN  = 7.5;   // Nachbarsumme-Band für Geburt (von 24)
const float GEBURT_MAX  = 10.5;
const float LEBEN_MIN   = 5.5;   // Band fürs Überleben
const float LEBEN_MAX   = 12.5;
const float SAAT_DICHTE = 0.42;
const float BASS_SAAT   = 0.45;
// ----------------------------------------------------------------------------

float n21(vec2 p) { return fract(sin(dot(p, vec2(41.3, 289.1))) * 43758.5453); }
float zelle(vec2 id)
{
    vec2 uv = (id + 0.5) * ZELLGROESSE / iResolution.xy;
    return step(0.5, texture(iChannel0, uv).r);
}
void mainImage(out vec4 fragColor, in vec2 fragCoord)
{
    vec2 id = floor(fragCoord / ZELLGROESSE);
    if (iFrame < 2)
    {
        fragColor = vec4(step(1.0 - SAAT_DICHTE, n21(id)), 0.0, 0.0, 1.0);
        return;
    }
    // 5×5-Nachbarschaft (ohne Zentrum): 24 Nachbarn
    float summe = 0.0;
    for (int y = -2; y <= 2; ++y)
    for (int x = -2; x <= 2; ++x)
    {
        if (x == 0 && y == 0) continue;
        summe += zelle(id + vec2(float(x), float(y)));
    }
    float war = zelle(id);
    float lebt = (war > 0.5)
        ? ((summe > LEBEN_MIN && summe < LEBEN_MAX) ? 1.0 : 0.0)
        : ((summe > GEBURT_MIN && summe < GEBURT_MAX) ? 1.0 : 0.0);

    // weiches Dichtefeld mitschreiben (.g): 24er-Summe normiert — der
    // Image-Pass rendert damit glatte Membranen statt harter Pixel
    float dichte = summe / 24.0;

    float bass = texture(iChannel1, vec2(0.05, 0.25)).x;
    if (bass > BASS_SAAT)
    {
        vec2 p = 0.5 + 0.3 * vec2(sin(iTime * 0.8), cos(iTime * 1.2));
        if (length(fragCoord / iResolution.xy - p) < 0.05 &&
            n21(id + iTime) > 0.5) lebt = 1.0;
    }
    fragColor = vec4(lebt, dichte, 0.0, 1.0);
}
