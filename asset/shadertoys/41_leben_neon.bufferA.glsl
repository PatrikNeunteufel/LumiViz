// 41 Leben-Neon, Buffer A — Patrik Neunteufel (LumiViz, 2026). Eigenwerk.
// "Buffer A": iChannel0 = Buffer A (SELBST!), iChannel1 = Music.
//
// IDEE: Conways Game of Life auf Pixel-Zellen. .r = lebendig (0/1),
// .g = Alter (Frames seit Geburt, für die Farbe im Image-Pass).
// Der Bass sät frische Zellen an wandernder Stelle nach — die Musik
// "füttert" das Leben.

// ---- STELLSCHRAUBEN --------------------------------------------------------
const float ZELLGROESSE = 3.0;   // Pixel pro Zelle (größer = gröber)
const float SAAT_DICHTE = 0.30;  // Startbelegung (Frame 0)
const float BASS_SAAT   = 0.5;   // ab dieser Bass-Stärke wird nachgesät
const float SAAT_RADIUS = 0.06;  // Größe des Nachsä-Flecks
// ----------------------------------------------------------------------------

float n21(vec2 p) { return fract(sin(dot(p, vec2(41.3, 289.1))) * 43758.5453); }
// Zell-Zustand an Zellkoordinate (0/1)
float zelle(vec2 zellId, vec2 gitter)
{
    vec2 uv = (zellId + 0.5) * ZELLGROESSE / iResolution.xy;
    return step(0.5, texture(iChannel0, uv).r);
}
void mainImage(out vec4 fragColor, in vec2 fragCoord)
{
    vec2 gitter = floor(iResolution.xy / ZELLGROESSE);
    vec2 id = floor(fragCoord / ZELLGROESSE);

    if (iFrame < 2)  // Ur-Saat: Zufallsmuster
    {
        float lebend = step(1.0 - SAAT_DICHTE, n21(id));
        fragColor = vec4(lebend, 0.0, 0.0, 1.0);
        return;
    }
    // 8 Nachbarn zählen (Conway-Regel B3/S23)
    float summe = 0.0;
    for (int y = -1; y <= 1; ++y)
    for (int x = -1; x <= 1; ++x)
    {
        if (x == 0 && y == 0) continue;
        summe += zelle(id + vec2(float(x), float(y)), gitter);
    }
    vec4 alt = texture(iChannel0, (id + 0.5) * ZELLGROESSE / iResolution.xy);
    float war = step(0.5, alt.r);
    float lebt = (war > 0.5) ? ((summe == 2.0 || summe == 3.0) ? 1.0 : 0.0)
                             : ((summe == 3.0) ? 1.0 : 0.0);
    float alter = lebt * (alt.g * war + 1.0 / 255.0) * 255.0;  // Frames zählen
    alter = min(alter, 250.0);

    // Bass-Nachsaat: an wandernder Stelle zufällige Zellen beleben
    float bass = texture(iChannel1, vec2(0.05, 0.25)).x;
    if (bass > BASS_SAAT)
    {
        vec2 p = 0.5 + 0.35 * vec2(cos(iTime * 0.9), sin(iTime * 1.3));
        vec2 uv = fragCoord / iResolution.xy;
        if (length(uv - p) < SAAT_RADIUS && n21(id + iTime) > 0.5) lebt = 1.0;
    }
    fragColor = vec4(lebt, alter / 255.0, 0.0, 1.0);
}
