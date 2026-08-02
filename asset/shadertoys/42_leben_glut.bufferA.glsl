// 42 Leben-Glut, Buffer A — Patrik Neunteufel (LumiViz, 2026). Eigenwerk.
// "Buffer A": iChannel0 = Buffer A (SELBST!), iChannel1 = Music.
//
// IDEE: "Generations"-Ableger von Game of Life: gestorbene Zellen
// verschwinden nicht, sondern durchlaufen GLUT-STUFEN (.g zählt herunter)
// — wie verglühende Asche hinter der lebenden Front. Bass sät nach.

// ---- STELLSCHRAUBEN --------------------------------------------------------
const float ZELLGROESSE = 3.0;
const float GLUT_DAUER  = 0.06;  // Abkühl-Schritt pro Frame (kleiner = länger)
const float SAAT_DICHTE = 0.18;
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
    // Glut: beim Sterben auf 1 setzen, dann pro Frame abkühlen
    float glut = alt.g;
    if (war > 0.5 && lebt < 0.5) glut = 1.0;          // frisch gestorben
    else glut = max(glut - GLUT_DAUER, 0.0);          // abkühlen

    float bass = texture(iChannel1, vec2(0.05, 0.25)).x;
    if (bass > BASS_SAAT)
    {
        vec2 p = 0.5 + 0.35 * vec2(cos(iTime * 1.1), sin(iTime * 0.7));
        if (length(fragCoord / iResolution.xy - p) < 0.05 &&
            n21(id + iTime) > 0.6) lebt = 1.0;
    }
    fragColor = vec4(lebt, glut, 0.0, 1.0);
}
