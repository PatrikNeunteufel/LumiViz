// 05 Voronoi-Zellatmung — Patrik Neunteufel (LumiViz, 2026). Eigenwerk.
// Shadertoy: iChannel0 = Music (512x2: y=0.25 FFT, y=0.75 Waveform).
//
// IDEE: Voronoi = "welcher Zellkern ist am nächsten?". Kerne kreisen in
// ihren Gitterzellen; jede Zelle hat eine EIGENE Größe (gewichteter
// Abstand) und glüht mit "ihrem" FFT-Band.

// ---- STELLSCHRAUBEN --------------------------------------------------------
const float ZOOM              = 3.0;   // mehr = mehr, kleinere Zellen
const float WANDER_RADIUS     = 0.40;  // Kreisbahn der Kerne (Bewegungsstärke)
const float WANDER_TEMPO      = 0.25;  // Grundtempo der Kern-Kreise
const float AUDIO_TEMPO       = 2.5;   // Audio-Schub aufs Tempo: laute Bänder
                                       // treiben "ihre" Zellen an (0 = aus)
const float GROESSEN_STREUUNG = 0.90;  // 0 = alle gleich groß, 1 = stark verschieden
const float GLOW_RADIUS       = 0.32;  // Grund-Glühradius je Zelle
const float AUDIO_WEITUNG     = 0.40;  // Radius-Zuwachs bei lautem Band
const float KANTEN_WEICHE     = 0.006; // Kantenbreite: 0.006 = HARTE Kante
                                       // Farbe→Schwarz (größer = Verlauf)
const float HELL_GRUND        = 0.35;  // Grundhelligkeit einer Zelle
const float HELL_AUDIO        = 1.2;   // Audio-Aufhellung
// ----------------------------------------------------------------------------

vec2 hash2(vec2 p)
{
    // 2D-Hash: stabiles Pseudo-Zufalls-Paar 0..1 je Gitterzelle
    p = vec2(dot(p, vec2(127.1, 311.7)), dot(p, vec2(269.5, 183.3)));
    return fract(sin(p) * 43758.5453);
}
void mainImage(out vec4 fragColor, in vec2 fragCoord)
{
    vec2 uv = (2.0 * fragCoord - iResolution.xy) / iResolution.y * ZOOM;
    vec2 g = floor(uv);
    vec2 f = fract(uv);
    float dMin = 8.0;
    vec2 idMin = vec2(0.0);
    // 3x3-Nachbarschaft: der nächste Kern kann im Nachbarfeld liegen
    for (int y = -1; y <= 1; ++y)
    for (int x = -1; x <= 1; ++x)
    {
        vec2 o = vec2(float(x), float(y));
        vec2 h = hash2(g + o);
        // Tempo aus dem Audio: das Band DIESES Kerns schiebt seine Phase an —
        // laute Bänder lassen ihre Zellen zucken/schneller kreisen
        float kernBand = texture(iChannel0, vec2(0.05 + 0.60 * h.x, 0.25)).x;
        float phase = iTime * (WANDER_TEMPO + h.x) + AUDIO_TEMPO * kernBand;
        vec2 c = o + 0.5 + WANDER_RADIUS * sin(phase + 6.28318 * h) - f;
        // GEWICHTETER Abstand: Zellen mit kleinem Gewicht "reichen weiter"
        // und wirken größer — das macht die Größen-Streuung
        float gewicht = 1.0 + GROESSEN_STREUUNG * (h.y - 0.5);
        float d = dot(c, c) * gewicht;
        if (d < dMin) { dMin = d; idMin = h; }
    }
    // Zell-Hash wählt den FFT-Bin: jede Zelle hört ihr eigenes Band
    float band = texture(iChannel0, vec2(0.05 + 0.60 * idMin.x, 0.25)).x;
    float r = GLOW_RADIUS + AUDIO_WEITUNG * band;  // Soll-Glühradius
    // scharfe Kante: nur KANTEN_WEICHE breiter Übergang am Radius
    float glow = 1.0 - smoothstep(r - KANTEN_WEICHE, r, sqrt(dMin));
    vec3 col = glow * (0.5 + 0.5 * cos(6.28318 * idMin.y + iTime * 0.5 +
                                       vec3(0.0, 2.1, 4.2)));
    col *= HELL_GRUND + HELL_AUDIO * band;
    fragColor = vec4(col, 1.0);
}
