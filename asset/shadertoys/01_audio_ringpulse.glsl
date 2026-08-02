// 01 Audio-Ringpulse — Patrik Neunteufel (LumiViz, 2026). Eigenwerk.
// Shadertoy: iChannel0 = Music (512x2: y=0.25 FFT, y=0.75 Waveform).
//
// IDEE: Konzentrische Ringe; der Bass schiebt alle nach außen, die Waveform
// verbeult jeden Ring individuell. Jeder Ring hat seine eigene Dicke.

// ---- STELLSCHRAUBEN --------------------------------------------------------
const int   RING_ANZAHL      = 5;     // Anzahl Ringe
const float RING_BASIS       = 0.22;  // Radius des innersten Rings
const float RING_ABSTAND     = 0.17;  // Abstand der Ringe zueinander
const float RING_DICKE       = 0.030; // Dicke des innersten Rings
const float DICKE_ZUWACHS    = 0.014; // je Ring dicker nach außen
const float BASS_WEITUNG     = 0.15;  // wie stark der Bass alle Ringe aufbläht
const float WELLEN_AUSSCHLAG = 0.15;  // Waveform-Verbeulung (war 0.35)
const float FARB_TEMPO       = 0.7;   // Tempo des Farbdurchlaufs
const float HOEHEN_GLOW      = 0.10;  // Mitten-Aufhellung durch die Höhen
// ----------------------------------------------------------------------------

void mainImage(out vec4 fragColor, in vec2 fragCoord)
{
    // zentrierte Koordinaten: (0,0) = Bildmitte, y-Spanne ±1
    vec2 uv = (2.0 * fragCoord - iResolution.xy) / iResolution.y;
    float d = length(uv);

    // FFT-Bins: 0.05 = Bass, 0.70 = Höhen (x wählt den Bin, Zeile y=0.25)
    float bass = texture(iChannel0, vec2(0.05, 0.25)).x;
    float treb = texture(iChannel0, vec2(0.70, 0.25)).x;

    vec3 col = vec3(0.0);
    for (int i = 0; i < RING_ANZAHL; ++i)
    {
        float fi = float(i);
        // Waveform-Sample abhängig vom Bildradius: jeder Ring bekommt einen
        // anderen Wellen-Ausschnitt (−0.5 zentriert; Stille = 0)
        float wave = texture(iChannel0, vec2(fract(d * 0.5 + fi * 0.13), 0.75)).x - 0.5;
        float r = RING_BASIS + fi * RING_ABSTAND + BASS_WEITUNG * bass +
                  WELLEN_AUSSCHLAG * wave;
        float dicke = RING_DICKE + fi * DICKE_ZUWACHS;
        // 1 exakt auf dem Soll-Radius, 0 ab `dicke` daneben
        float ring = smoothstep(dicke, 0.0, abs(d - r));
        // Cosinus-Palette (3 Phasen à 120°); fi*1.3 = Farbversatz je Ring
        col += ring * (0.5 + 0.5 * cos(iTime * FARB_TEMPO + fi * 1.3 +
                                       vec3(0.0, 2.1, 4.2)));
    }
    col += HOEHEN_GLOW * treb / (0.2 + d);  // Glow, innen am stärksten
    fragColor = vec4(col, 1.0);
}
