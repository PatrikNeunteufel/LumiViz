// 45 Leben im Fraktal, Buffer A — Patrik Neunteufel (LumiViz, 2026). Eigenwerk.
// "Buffer A": iChannel0 = Buffer A (SELBST!), iChannel1 = Music.
//
// IDEE: Game of Life × Fraktal: die Conway-Regel läuft nur INNERHALB einer
// Julia-Menge — das Fraktal ist die Petrischale, seine Ränder beschneiden
// die Kolonien. Die Julia-Form wandert langsam: die Schale verformt sich
// und zwingt das Leben in neue Gebiete.

// ---- STELLSCHRAUBEN --------------------------------------------------------
const float ZELLGROESSE = 2.0;
const float SAAT_DICHTE = 0.30;
const float SCHALE_TEMPO = 0.03; // Wandel der Julia-Schale
const int   JULIA_ITER  = 24;
const float BASS_SAAT   = 0.45;
// ----------------------------------------------------------------------------

float n21(vec2 p) { return fract(sin(dot(p, vec2(41.3, 289.1))) * 43758.5453); }
float zelle(vec2 id)
{
    vec2 uv = (id + 0.5) * ZELLGROESSE / iResolution.xy;
    return step(0.5, texture(iChannel0, uv).r);
}
// Petrischale: 1 innerhalb der Julia-Menge
float schale(vec2 uv)
{
    vec2 z = (uv * 2.0 - 1.0) * vec2(iResolution.x / iResolution.y, 1.0) * 1.4;
    vec2 c = 0.75 * vec2(cos(iTime * SCHALE_TEMPO), sin(iTime * SCHALE_TEMPO * 1.3));
    for (int i = 0; i < JULIA_ITER; ++i)
    {
        z = vec2(z.x * z.x - z.y * z.y, 2.0 * z.x * z.y) + c;
        if (dot(z, z) > 4.0) return 0.0;
    }
    return 1.0;
}
void mainImage(out vec4 fragColor, in vec2 fragCoord)
{
    vec2 uv = fragCoord / iResolution.xy;
    vec2 id = floor(fragCoord / ZELLGROESSE);
    float drin = schale(uv);
    if (iFrame < 2)
    {
        fragColor = vec4(step(1.0 - SAAT_DICHTE, n21(id)) * drin, drin, 0.0, 1.0);
        return;
    }
    float summe = 0.0;
    for (int y = -1; y <= 1; ++y)
    for (int x = -1; x <= 1; ++x)
    {
        if (x == 0 && y == 0) continue;
        summe += zelle(id + vec2(float(x), float(y)));
    }
    float war = zelle(id);
    float lebt = (war > 0.5) ? ((summe == 2.0 || summe == 3.0) ? 1.0 : 0.0)
                             : ((summe == 3.0) ? 1.0 : 0.0);
    lebt *= drin;  // außerhalb der Schale stirbt alles

    float bass = texture(iChannel1, vec2(0.05, 0.25)).x;
    if (bass > BASS_SAAT && drin > 0.5 && n21(id + iTime) > 0.97) lebt = 1.0;
    fragColor = vec4(lebt, drin, 0.0, 1.0);  // .g = Schale (für den Image-Pass)
}
