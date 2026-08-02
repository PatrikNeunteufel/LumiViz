// 59 Daten-Regen — Patrik Neunteufel (LumiViz, 2026). Eigenwerk.
// Shadertoy: iChannel0 = Music. TECHNIK: fallende Zeichenkolonnen.
//
// IDEE: Spalten aus Blockglyphen (Zufallsbitmuster je Rasterzelle, das
// periodisch wechselt) fallen mit verschiedenen Tempi; der Kopf jeder
// Kolonne leuchtet hell, dahinter verblasst die Spur. Der Bass lässt
// die Kolonnen schneller stürzen.

// ---- STELLSCHRAUBEN --------------------------------------------------------
const float SPALTEN     = 40.0;
const float ZEILEN      = 30.0;
const float FALL_GRUND  = 0.25;
const float FALL_BASS   = 0.6;
const float SPUR_LAENGE = 0.35;  // sichtbare Spur hinter dem Kopf (0..1)
const float GLYPH_WECHSEL = 3.0; // Musterwechsel pro Sekunde
// ----------------------------------------------------------------------------

float n21(vec2 p) { return fract(sin(dot(p, vec2(41.3, 289.1))) * 43758.5453); }
void mainImage(out vec4 fragColor, in vec2 fragCoord)
{
    vec2 uv = fragCoord / iResolution.xy;
    float bass = texture(iChannel0, vec2(0.05, 0.25)).x;

    float spalte = floor(uv.x * SPALTEN);
    float srnd = n21(vec2(spalte, 7.0));
    // Kopfposition der Kolonne: fällt endlos (wrappt), Tempo je Spalte
    float kopf = 1.0 - fract(iTime * (FALL_GRUND + FALL_BASS * bass) *
                             (0.5 + srnd) + srnd * 5.0);
    // Abstand HINTER dem Kopf (nach oben), mit Wrap
    float dy = fract(kopf - uv.y);
    float spur = (dy < SPUR_LAENGE) ? 1.0 - dy / SPUR_LAENGE : 0.0;

    // Glyphe: 3x3-Bitmuster je Rasterzelle, wechselt im Takt
    vec2 zelle = vec2(spalte, floor(uv.y * ZEILEN));
    vec2 sub = floor(fract(vec2(uv.x * SPALTEN, uv.y * ZEILEN)) * 3.0);
    float bit = step(0.5, n21(zelle + sub * 0.37 +
                              floor(iTime * GLYPH_WECHSEL) * 0.13));
    // Rand der Zelle frei lassen (Glyphen trennen)
    vec2 fz = fract(vec2(uv.x * SPALTEN, uv.y * ZEILEN));
    float rand = step(0.08, fz.x) * step(fz.x, 0.92) *
                 step(0.08, fz.y) * step(fz.y, 0.92);

    float hell = spur * bit * rand;
    vec3 col = hell * vec3(0.2, 1.0, 0.4) * (0.3 + spur);
    // Kopf-Glyphe weiß aufblitzen
    col += step(0.965, spur) * bit * rand * vec3(0.8, 1.0, 0.85);
    col += vec3(0.0, 0.015, 0.005);
    fragColor = vec4(col, 1.0);
}
