// 14 Spektrogramm-Rad, Buffer A — Patrik Neunteufel (LumiViz, 2026). Eigenwerk.
// Shadertoy-Tab "Buffer A": iChannel0 = Buffer A (SELBST = Vorframe!),
//                           iChannel1 = Music.
//
// IDEE: Wasserfall-Speicher: x = Zeit. Bestand rutscht je Frame nach links
// (Vorframe versetzt lesen), rechts wird das aktuelle Spektrum eingespeist.

// ---- STELLSCHRAUBEN --------------------------------------------------------
const float SCROLL_PIXEL    = 2.0;  // Pixel pro Frame (mehr = schnellere Historie)
const float SPEKTRUM_ANTEIL = 0.80; // Frequenzbereich (untere 80% des FFT)
// ----------------------------------------------------------------------------

void mainImage(out vec4 fragColor, in vec2 fragCoord)
{
    vec2 uv = fragCoord / iResolution.xy;
    vec2 px = 1.0 / iResolution.xy;
    if (uv.x > 1.0 - px.x * SCROLL_PIXEL)
    {
        // rechte Spalte: frisches Spektrum (y wählt den Bin)
        float fft = texture(iChannel1, vec2(uv.y * SPEKTRUM_ANTEIL + 0.02, 0.25)).x;
        fragColor = vec4(vec3(fft), 1.0);
    }
    else
    {
        // Vorframe versetzt lesen = Inhalt rutscht nach links
        fragColor = texture(iChannel0, uv + vec2(px.x * SCROLL_PIXEL, 0.0));
    }
}
