// 86 Galaxie-Leben, Buffer A — Patrik Neunteufel (LumiViz, 2026). Eigenwerk.
// "Buffer A": iChannel0 = SELBST, iChannel1 = Music. KOMBI: Galaxie × Life —
// Game of Life lebt NUR auf den Spiralarmen einer rotierenden Galaxie
// (Sterne entstehen und vergehen entlang der Arme).

// ---- STELLSCHRAUBEN --------------------------------------------------------
const float ZELLGROESSE = 2.0;
const float SPIRAL_ARME = 2.0;
const float SPIRAL_ENGE = 3.5;
const float DREH_TEMPO  = 0.02;
const float ARM_BREITE  = 0.35;  // wie breit die lebbare Zone der Arme ist
const float SAAT_DICHTE = 0.3;
// ----------------------------------------------------------------------------

float n21(vec2 p) { return fract(sin(dot(p, vec2(41.3, 289.1))) * 43758.5453); }
float zelle(vec2 id)
{
    vec2 uv = (id + 0.5) * ZELLGROESSE / iResolution.xy;
    return step(0.5, texture(iChannel0, uv).r);
}
// Spiralarm-Maske 0..1 an Bildposition
float arm(vec2 uv01)
{
    vec2 p = (uv01 * 2.0 - 1.0) * vec2(iResolution.x / iResolution.y, 1.0);
    float r = length(p) + 1e-3;
    float a = atan(p.y, p.x) - iTime * DREH_TEMPO;
    float w = 0.5 + 0.5 * cos(a * SPIRAL_ARME + SPIRAL_ENGE * log(r));
    return smoothstep(1.0 - ARM_BREITE, 1.0, w) * smoothstep(1.2, 0.3, r);
}
void mainImage(out vec4 fragColor, in vec2 fragCoord)
{
    vec2 uv = fragCoord / iResolution.xy;
    vec2 id = floor(fragCoord / ZELLGROESSE);
    float zone = arm(uv);
    if (iFrame < 2)
    {
        fragColor = vec4(step(1.0 - SAAT_DICHTE, n21(id)) * step(0.5, zone),
                         zone, 0.0, 1.0);
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
    lebt *= step(0.35, zone);  // außerhalb der Arme stirbt alles
    // Arme wandern → an frisch überstrichenen Rändern nachsäen
    if (zone > 0.5 && n21(id + floor(iTime * 2.0)) > 0.992) lebt = 1.0;
    fragColor = vec4(lebt, zone, 0.0, 1.0);
}
