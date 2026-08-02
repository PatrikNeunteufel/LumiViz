// 51 Mikroben-Tanz — Patrik Neunteufel (LumiViz, 2026). Eigenwerk.
// Shadertoy: iChannel0 = Music. BIOLOGIE: wuselnde Wimpertierchen.
//
// IDEE: Jede Mikrobe ist eine Kette von Kreisen entlang einer schlängelnden
// Mittellinie (sin-Ketten mit Phasenversatz = Schwimmbewegung). Kopf dick,
// Schwanz dünn; ein Wimpernsaum flimmert außen. Die Lautstärke macht das
// Gewusel hektischer.

// ---- STELLSCHRAUBEN --------------------------------------------------------
const int   MIKROBEN    = 7;
const int   GLIEDER     = 10;    // Kettenglieder je Mikrobe
const float GROESSE     = 0.05;  // Kopfradius
const float SCHLAENGELN = 0.35;  // Amplitude der Schwimmwelle
const float TEMPO_VOL   = 1.2;   // Lautstärke-Zuschlag aufs Tempo
// ----------------------------------------------------------------------------

float h1(float n) { return fract(sin(n) * 43758.5453); }
void mainImage(out vec4 fragColor, in vec2 fragCoord)
{
    vec2 uv = (2.0 * fragCoord - iResolution.xy) / iResolution.y;
    float vol = texture(iChannel0, vec2(0.15, 0.25)).x;
    float tempo = 1.0 + TEMPO_VOL * vol;

    // "Nährlösung": leichte Trübung mit Lichtkegel von oben
    vec3 col = vec3(0.02, 0.035, 0.045) * (1.2 - 0.5 * length(uv));

    for (int m = 0; m < MIKROBEN; ++m)
    {
        float fm = float(m);
        // Kopfbahn: geschlossene Wanderkurve je Mikrobe
        float t = iTime * tempo * (0.25 + 0.3 * h1(fm * 3.7)) + fm * 2.2;
        vec2 kopf = vec2(0.85 * sin(t) * (0.5 + 0.5 * h1(fm * 5.1)),
                         0.7 * sin(t * 1.37 + fm));
        vec2 richtung = normalize(vec2(cos(t), 1.37 * cos(t * 1.37 + fm)) + 1e-4);
        vec2 quer = vec2(-richtung.y, richtung.x);
        vec3 farbe = 0.5 + 0.5 * cos(fm * 1.9 + vec3(0.0, 2.1, 4.2));

        for (int g = 0; g < GLIEDER; ++g)
        {
            float fg = float(g) / float(GLIEDER - 1);  // 0 Kopf .. 1 Schwanz
            // Glied sitzt hinter dem Kopf, quer ausgelenkt (laufende Welle)
            vec2 p = kopf - richtung * fg * 0.35 +
                     quer * SCHLAENGELN * 0.2 * sin(fg * 9.0 - iTime * 6.0 * tempo + fm);
            float r = GROESSE * (1.0 - 0.75 * fg);
            float d = length(uv - p);
            // Körper halbtransparent + Wimpernsaum (schmaler Ring flimmert)
            col += smoothstep(r, r * 0.3, d) * farbe * 0.25;
            float saum = smoothstep(0.012, 0.0, abs(d - r)) *
                         (0.5 + 0.5 * sin(atan(uv.y - p.y, uv.x - p.x) * 20.0 +
                                          iTime * 12.0));
            col += saum * farbe * 0.2;
        }
    }
    fragColor = vec4(col, 1.0);
}
