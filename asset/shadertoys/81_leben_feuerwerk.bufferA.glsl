// 81 Leben-Feuerwerk, Buffer A — Patrik Neunteufel (LumiViz, 2026). Eigenwerk.
// "Buffer A": iChannel0 = SELBST, iChannel1 = Music. KOMBI: Game of Life ×
// Feuerwerk — sterbende Zellen ZÜNDEN: sie schreiben einen Glow in den
// .b-Kanal, der wie ein Funken-Trail verglüht und verweht (leicht nach oben
// gelesen = Funken steigen).

// ---- STELLSCHRAUBEN --------------------------------------------------------
const float ZELLGROESSE = 3.0;
const float SAAT_DICHTE = 0.25;
const float FUNKEN_GLUT = 0.94;  // Verglüh-Faktor des Trails
const float AUFTRIEB    = 1.5;   // Pixel/Frame, die der Glow aufsteigt
const float BASS_SAAT   = 0.4;
// ----------------------------------------------------------------------------

float n21(vec2 p) { return fract(sin(dot(p, vec2(41.3, 289.1))) * 43758.5453); }
float zelle(vec2 id)
{
    vec2 uv = (id + 0.5) * ZELLGROESSE / iResolution.xy;
    return step(0.5, texture(iChannel0, uv).r);
}
void mainImage(out vec4 fragColor, in vec2 fragCoord)
{
    vec2 uv = fragCoord / iResolution.xy;
    vec2 id = floor(fragCoord / ZELLGROESSE);
    if (iFrame < 2)
    {
        fragColor = vec4(step(1.0 - SAAT_DICHTE, n21(id)), 0.0, 0.0, 1.0);
        return;
    }
    float summe = 0.0;
    for (int y = -1; y <= 1; ++y)
    for (int x = -1; x <= 1; ++x)
    {
        if (x == 0 && y == 0) continue;
        summe += zelle(id + vec2(float(x), float(y)));
    }
    vec4 alt = texture(iChannel0, (id + 0.5) * ZELLGROESSE / iResolution.xy);
    float war = step(0.5, alt.r);
    float lebt = (war > 0.5) ? ((summe == 2.0 || summe == 3.0) ? 1.0 : 0.0)
                             : ((summe == 3.0) ? 1.0 : 0.0);
    // Funken-Kanal: von UNTEN lesen (steigt auf) + verglühen; Zündung beim Tod
    float funken = texture(iChannel0,
                           uv - vec2(0.0, AUFTRIEB / iResolution.y)).b * FUNKEN_GLUT;
    if (war > 0.5 && lebt < 0.5) funken = 1.0;  // Todes-Zündung

    float bass = texture(iChannel1, vec2(0.05, 0.25)).x;
    if (bass > BASS_SAAT && n21(id + iTime) > 0.985) lebt = 1.0;
    fragColor = vec4(lebt, 0.0, min(funken, 1.5), 1.0);
}
