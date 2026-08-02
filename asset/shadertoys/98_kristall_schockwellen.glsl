// 98 Kristall-Schockwellen — Patrik Neunteufel (LumiViz, 2026). Eigenwerk.
// Shadertoy: iChannel0 = Music. KOMBI: Puls-Ringe × Facetten — die
// Schockwellen laufen durch ein Voronoi-Facettenfeld: jede Facette blitzt
// GESCHLOSSEN auf, wenn die Wellenfront ihr Zentrum passiert (kristallines
// Aufleuchten statt glattem Ring). Bass startet Zentrums-Wellen.

// ---- STELLSCHRAUBEN --------------------------------------------------------
const float FACETTEN   = 5.0;
const float RATE       = 0.5;    // Wellen pro Sekunde
const float TEMPO      = 0.5;    // Expansions-Tempo
const float BLITZ_DAUER = 0.35;  // wie lange eine Facette nachleuchtet
const float BASS_WELLE = 0.5;
// ----------------------------------------------------------------------------

vec2 hash2(vec2 p)
{
    p = vec2(dot(p, vec2(127.1, 311.7)), dot(p, vec2(269.5, 183.3)));
    return fract(sin(p) * 43758.5453);
}
void mainImage(out vec4 fragColor, in vec2 fragCoord)
{
    vec2 uv = (2.0 * fragCoord - iResolution.xy) / iResolution.y;
    float bass = texture(iChannel0, vec2(0.05, 0.25)).x;

    // Facette dieses Pixels: Kernposition + Id
    vec2 g = floor(uv * FACETTEN);
    vec2 f = fract(uv * FACETTEN);
    float dMin = 8.0;
    vec2 id = vec2(0.0);
    vec2 kern = vec2(0.0);
    for (int y = -1; y <= 1; ++y)
    for (int x = -1; x <= 1; ++x)
    {
        vec2 o = vec2(float(x), float(y));
        vec2 h = hash2(g + o);
        vec2 c = o + h - f;
        float d = dot(c, c);
        if (d < dMin) { dMin = d; id = h; kern = (g + o + h) / FACETTEN; }
    }
    float kernR = length(kern);  // Abstand des FACETTEN-ZENTRUMS zur Mitte

    vec3 col = vec3(0.012, 0.012, 0.03);
    // Facetten-Grundton (dunkel, leicht variiert) + Kanten
    float kante = smoothstep(0.0, 0.03, sqrt(dMin));
    col += vec3(0.03, 0.04, 0.07) * (0.5 + id.y) * kante;

    // Wellen (2 Fenster + Bass-Welle): Front-Radius vs. kernR
    for (int k = 0; k < 3; ++k)
    {
        float fk = float(k);
        float rate = (k == 2) ? 1.4 : RATE;
        if (k == 2 && bass < BASS_WELLE) continue;
        float fenster = iTime * rate + fk * 0.37;
        float alterT = fract(fenster) / rate;   // Sekunden seit Start
        float front = TEMPO * alterT * ((k == 2) ? 1.6 : 1.0);
        // Facette blitzt, wenn die Front gerade ihr Zentrum passiert hat
        float seit = front - kernR;
        if (seit > 0.0 && seit < BLITZ_DAUER)
        {
            float blitz = 1.0 - seit / BLITZ_DAUER;
            vec3 farbe = 0.5 + 0.5 * cos(id.x * 6.28318 + fk * 2.0 +
                                         vec3(0.0, 2.1, 4.2));
            col += blitz * blitz * farbe * kante * 1.4;
        }
    }
    fragColor = vec4(col, 1.0);
}
