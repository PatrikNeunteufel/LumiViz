// 33 Lava-Lampe — Patrik Neunteufel (LumiViz, 2026). Eigenwerk.
// Shadertoy: iChannel0 = Music. 2D-METABALL-FELD mit harter Iso-Kante.
//
// IDEE: Mehrere Blobs steigen und sinken (verschiedene Tempi/Phasen);
// ihre Gauß-Felder summieren sich, eine Iso-Schwelle mit HARTER Kante
// formt die Lava. Innen ein Glut-Gradient, außen Glas-Vignette.
// Der Bass bläht die Blobs, die Lautstärke heizt die Farbe.

// ---- STELLSCHRAUBEN --------------------------------------------------------
const int   BLOBS       = 6;
const float BLOB_GROESSE = 0.16;
const float BASS_BLAEHUNG = 0.08;
const float STEIG_TEMPO = 0.25;   // Auf-/Abstiegs-Tempo
const float ISO_SCHWELLE = 1.0;   // Feldstärke, ab der "Lava" ist
const float ISO_KANTE   = 0.02;   // Kantenbreite: 0.02 = hart
const float GLUT        = 1.3;    // Innenglut-Verstärkung
// ----------------------------------------------------------------------------

float h1(float n) { return fract(sin(n) * 43758.5453); }
void mainImage(out vec4 fragColor, in vec2 fragCoord)
{
    vec2 uv = (2.0 * fragCoord - iResolution.xy) / iResolution.y;
    float bass = texture(iChannel0, vec2(0.05, 0.25)).x;
    float vol  = texture(iChannel0, vec2(0.15, 0.25)).x;

    float feld = 0.0;
    for (int i = 0; i < BLOBS; ++i)
    {
        float fi = float(i);
        // Auf/Ab: sin mit eigenem Tempo/Phase je Blob; x pendelt leicht
        float phase = iTime * STEIG_TEMPO * (0.6 + 0.8 * h1(fi * 3.3)) + fi * 2.4;
        vec2 p = vec2(0.7 * (h1(fi * 7.1) - 0.5) + 0.08 * sin(phase * 1.7),
                      0.75 * sin(phase));
        float r = BLOB_GROESSE * (0.7 + 0.6 * h1(fi * 5.9)) + BASS_BLAEHUNG * bass;
        float d2 = dot(uv - p, uv - p);
        feld += r * r / (d2 + 1e-4);  // Metaball-Beitrag (~1 am Rand r)
    }
    // HARTE Iso-Kante: Lava/kein-Lava ohne Verlauf
    float lava = smoothstep(ISO_SCHWELLE - ISO_KANTE, ISO_SCHWELLE + ISO_KANTE, feld);
    // Innenglut: Feldstärke über der Schwelle = Kern heller/gelber
    float kern = clamp((feld - ISO_SCHWELLE) * 0.5, 0.0, 1.0);
    vec3 lavaFarbe = mix(vec3(0.9, 0.2, 0.05), vec3(1.0, 0.85, 0.3),
                         kern * GLUT * (0.6 + 0.8 * vol));
    // Hintergrund: warmes Lampenglas mit Vignette
    vec3 glas = mix(vec3(0.10, 0.02, 0.05), vec3(0.03, 0.01, 0.02),
                    length(uv));
    vec3 col = mix(glas, lavaFarbe, lava);
    fragColor = vec4(col, 1.0);
}
