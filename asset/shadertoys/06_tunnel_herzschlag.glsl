// 06 Tunnel mit Herzschlag — Patrik Neunteufel (LumiViz, 2026). Eigenwerk.
// Shadertoy: iChannel0 = Music (512x2: y=0.25 FFT, y=0.75 Waveform).
//
// IDEE: Polar-Tunnel: z = TIEFE/r täuscht Tiefe vor (Bildmitte = fern).
// Der Bass gibt Vortrieb; das Spektrum leuchtet als Aura um die Mitte.

// ---- STELLSCHRAUBEN --------------------------------------------------------
const float VORTRIEB_GRUND = 0.40;  // Grund-Fahrgeschwindigkeit (war 0.6)
const float VORTRIEB_BASS  = 0.80;  // Bass-Schub obendrauf (war 1.4)
const float TIEFE          = 0.40;  // Tunnel-Streckung (kleiner = länger)
const float RING_DICHTE    = 6.28318; // Ringe in Fahrtrichtung
const float STREIFEN       = 8.0;   // Anzahl Längsstreifen
const float TWIST          = 0.15;  // Verdrehung der Streifen (war 0.7 — wackelte)
const float ROT_TEMPO      = 0.35;  // Tunnel-Rotation (0 = aus)
const float ROT_BASS       = 0.60;  // Bass beschleunigt die Rotation
const float WECHSEL_SEK    = 4.0;   // alle N Sekunden wechselt die Drehrichtung
                                    // (echte Beat-Flips bräuchten Zustand —
                                    // in LumiViz ginge das über einen Buffer)
const float KANTE          = 0.10;  // Muster-Kante: 0.02 = hart, 0.3 = weich
const float AURA_STAERKE   = 1.0;   // Spektrum-Aura um die Mitte
const float MITTE_DUNKEL   = 0.15;  // Größe des dunklen Fluchtpunkts
// ----------------------------------------------------------------------------

void mainImage(out vec4 fragColor, in vec2 fragCoord)
{
    vec2 uv = (2.0 * fragCoord - iResolution.xy) / iResolution.y;
    float bass = texture(iChannel0, vec2(0.05, 0.25)).x;
    float r = length(uv) + 1e-4;   // nie 0 (Division)
    float a = atan(uv.y, uv.x);
    // Rotation: Richtung wechselt als Rechteckwelle alle WECHSEL_SEK, der
    // Bass beschleunigt; sign(sin(...)) = +1/−1 im Wechsel
    float richtung = sign(sin(iTime * 3.14159 / WECHSEL_SEK) + 1e-4);
    a += richtung * iTime * (ROT_TEMPO + ROT_BASS * bass);
    float drive = iTime * (VORTRIEB_GRUND + VORTRIEB_BASS * bass);
    float z = TIEFE / r + drive;   // Tiefen-Koordinate

    // Wandmuster: Ringe × Längsstreifen; smoothstep um 0.5 macht aus dem
    // weichen Sinus-Produkt eine HARTE Hell/Dunkel-Kante (KANTE = Breite)
    float stripes = 0.5 + 0.5 * sin(z * RING_DICHTE) *
                    sin(a * STREIFEN + drive * TWIST);
    stripes = smoothstep(0.5 - KANTE, 0.5 + KANTE, stripes);
    // Winkel 0..1 wählt den FFT-Bin: das Spektrum liegt einmal um den Tunnel
    float fft = texture(iChannel0, vec2(fract(a / 6.28318), 0.25)).x;

    vec3 col = (0.5 + 0.5 * cos(z + a + vec3(0.0, 2.1, 4.2))) * stripes;
    col += AURA_STAERKE * fft * vec3(0.8, 0.3, 0.6) * smoothstep(1.2, 0.0, r);
    col *= smoothstep(0.0, MITTE_DUNKEL, r);  // Fluchtpunkt dunkel
    fragColor = vec4(col, 1.0);
}
