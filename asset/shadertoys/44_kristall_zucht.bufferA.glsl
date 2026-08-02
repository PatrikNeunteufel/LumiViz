// 44 Kristall-Zucht, Buffer A — Patrik Neunteufel (LumiViz, 2026). Eigenwerk.
// "Buffer A": iChannel0 = Buffer A (SELBST!), iChannel1 = Music.
//
// IDEE: Kristallwachstums-Automat (DLA-artig): eine Zelle "friert", wenn
// ein Nachbar gefroren ist UND ein Zufallswurf gelingt — vom Keim wachsen
// verästelte Kristallnadeln. .g speichert die GEBURTSZEIT: der Image-Pass
// schimmert die Jahresringe damit an. Höhen beschleunigen das Wachstum.

// ---- STELLSCHRAUBEN --------------------------------------------------------
const float WACHSTUM     = 0.16;  // Grund-Anfrierwahrscheinlichkeit je Frame
const float WACHSTUM_HOEHEN = 0.25; // Höhen-Zuschlag
const float RICHTUNGS_BIAS = 0.5; // 6-zählige Vorzugsrichtungen (0 = isotrop)
// ----------------------------------------------------------------------------

float n21(vec2 p) { return fract(sin(dot(p, vec2(41.3, 289.1))) * 43758.5453); }
float gefroren(vec2 uv) { return step(0.5, texture(iChannel0, uv).r); }
void mainImage(out vec4 fragColor, in vec2 fragCoord)
{
    vec2 uv = fragCoord / iResolution.xy;
    vec2 px = 1.0 / iResolution.xy;
    if (iFrame < 2)
    {
        // Keim in der Mitte
        float keim = step(length((uv - 0.5) * iResolution.xy), 3.0);
        fragColor = vec4(keim, 0.0, 0.0, 1.0);
        return;
    }
    vec4 alt = texture(iChannel0, uv);
    if (alt.r > 0.5) { fragColor = alt; return; }  // einmal Kristall, immer

    // gefrorene Nachbarn? (4er-Nachbarschaft reicht für Nadeln)
    float nachbarn = gefroren(uv + vec2(px.x, 0.0)) + gefroren(uv - vec2(px.x, 0.0)) +
                     gefroren(uv + vec2(0.0, px.y)) + gefroren(uv - vec2(0.0, px.y));
    float treb = texture(iChannel1, vec2(0.70, 0.25)).x;
    float p = WACHSTUM + WACHSTUM_HOEHEN * treb;
    // Richtungs-Bias: 6-zählig um die Mitte (hexagonaler Habitus)
    float winkel = atan(uv.y - 0.5, uv.x - 0.5);
    p *= 1.0 + RICHTUNGS_BIAS * cos(winkel * 6.0);

    float friert = step(0.5, nachbarn) * step(n21(fragCoord + iTime), p);
    // Geburtszeit merken (Sekunden, auf 0..1 gestaucht für die Textur)
    float geburt = friert * fract(iTime * 0.02);
    fragColor = vec4(friert, geburt, 0.0, 1.0);
}
