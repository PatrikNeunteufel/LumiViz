// 20 Moiré-Scheiben — Patrik Neunteufel (LumiViz, 2026). Eigenwerk.
// Shadertoy: iChannel0 = Music (512x2: y=0.25 FFT, y=0.75 Waveform).
//
// IDEE: Zwei Ringsysteme um kreisende Zentren; leicht verstimmte Frequenzen
// interferieren zu wandernden Moiré-Schlieren.

// ---- STELLSCHRAUBEN --------------------------------------------------------
const float ZENTRUM_RADIUS = 0.30;  // Kreisbahn der beiden Zentren
const float TEMPO_1X       = 0.90;  // Umlauftempi Zentrum 1 (x/y verschieden
const float TEMPO_1Y       = 0.75;  //  = elliptische Bahn)
const float TEMPO_2X       = 0.65;
const float TEMPO_2Y       = 0.85;
const float RING_DICHTE    = 24.0;  // Ringe pro Einheit
const float DICHTE_MITTEN  = 18.0;  //  … Mitten-Zuschlag
const float VERSTIMMUNG    = 1.04;  // Frequenzverhältnis der Systeme (DIE
const float VERSTIMM_BASS  = 0.08;  //  Moiré-Stellschraube) + Bass-Anteil
const float FARB_KONTRAST  = 2.0;
// ----------------------------------------------------------------------------

void mainImage(out vec4 fragColor, in vec2 fragCoord)
{
    vec2 uv = (2.0 * fragCoord - iResolution.xy) / iResolution.y;
    float bass = texture(iChannel0, vec2(0.05, 0.25)).x;
    float mid  = texture(iChannel0, vec2(0.30, 0.25)).x;
    vec2 c1 =  ZENTRUM_RADIUS * vec2(cos(iTime * TEMPO_1X), sin(iTime * TEMPO_1Y));
    vec2 c2 = -ZENTRUM_RADIUS * vec2(cos(iTime * TEMPO_2X), sin(iTime * TEMPO_2Y));
    float f = RING_DICHTE + DICHTE_MITTEN * mid;
    float r1 = sin(length(uv - c1) * f);
    float r2 = sin(length(uv - c2) * f * (VERSTIMMUNG + VERSTIMM_BASS * bass));
    float m = r1 * r2;  // Interferenz: gleichphasig hell, gegenphasig dunkel
    vec3 col = (0.5 + 0.5 * cos(m * FARB_KONTRAST + iTime * 0.4 +
                                vec3(0.0, 2.1, 4.2))) *
               smoothstep(-0.2, 0.7, m);
    col *= smoothstep(1.7, 0.5, length(uv));
    fragColor = vec4(col, 1.0);
}
