// 02 Spektrum-Stadt — Patrik Neunteufel (LumiViz, 2026). Eigenwerk.
// Shadertoy: iChannel0 = Music (512x2: y=0.25 FFT, y=0.75 Waveform).
//
// IDEE: Das Spektrum als Skyline (Spaltenhöhe = FFT-Bin), darunter eine
// wabernde Wasser-Spiegelung. Jedes Fenster hat seinen eigenen Zufall:
// Helligkeit, Farbton (warm/kalt), manche sind ganz aus.

// ---- STELLSCHRAUBEN --------------------------------------------------------
const float HORIZONT        = 0.35;  // Höhe der Wasserlinie (0..1 von unten)
const float SPALTEN         = 48.0;  // Anzahl Gebäude
const float HOEHE_MAX       = 0.60;  // maximale Gebäudehöhe
const float VERSTAERKUNG    = 1.8;   // FFT-Verstärkung (mehr Ausschlag)
const float ANHEBUNG        = 0.65;  // pow-Exponent <1 hebt LEISE Bins an
const float SPEKTRUM_ANTEIL = 0.70;  // nur die unteren 70% des FFT nutzen
const float WABERN_STAERKE  = 0.01;  // Wasser-Wabern: Amplitude
const float WABERN_FREQ     = 60.0;  //  … Frequenz
const float WABERN_TEMPO    = 2.0;   //  … Tempo
const float FENSTER_X       = 96.0;  // Fenster-Raster horizontal
const float FENSTER_Y       = 60.0;  //  … vertikal
const float FENSTER_AN      = 0.65;  // Anteil leuchtender Fenster (0.65 = 65%)
const float FLACKER_TEMPO   = 0.15;  // wie oft Fenster an/aus wechseln
// ----------------------------------------------------------------------------

float n21(vec2 p) { return fract(sin(dot(p, vec2(41.3, 289.1))) * 43758.5453); }

void mainImage(out vec4 fragColor, in vec2 fragCoord)
{
    vec2 uv = fragCoord / iResolution.xy;  // 0..1, y=0 unten
    bool water = uv.y < HORIZONT;
    vec2 p = uv;
    if (water)
    {
        // an der Wasserlinie spiegeln + sinusförmig wabern
        p.y = 2.0 * HORIZONT - uv.y +
              WABERN_STAERKE * sin(uv.x * WABERN_FREQ + iTime * WABERN_TEMPO);
    }
    // Spaltenindex quantisieren: eine Spalte teilt sich einen FFT-Bin
    float spalte = floor(p.x * SPALTEN) / SPALTEN;
    float roh = texture(iChannel0, vec2(spalte * SPEKTRUM_ANTEIL + 0.02, 0.25)).x;
    // pow(<1) hebt leise Bins an, VERSTAERKUNG skaliert — mehr Ausschlag
    float fft = clamp(pow(roh, ANHEBUNG) * VERSTAERKUNG, 0.0, 1.0);
    float h = HORIZONT + 0.02 + fft * HOEHE_MAX;  // Dachkante

    vec3 sky = mix(vec3(0.02, 0.03, 0.08), vec3(0.15, 0.05, 0.25), p.y);
    vec3 c = sky;
    if (p.y < h)  // Gebäude
    {
        // Fenster-Zelle bestimmen (Id für den Zufall je Fenster)
        vec2 zelle = vec2(floor(p.x * FENSTER_X), floor(p.y * FENSTER_Y + spalte * 7.0));
        float istFenster = step(0.75, fract(p.x * FENSTER_X)) *
                           step(0.60, fract(p.y * FENSTER_Y + spalte * 7.0));
        // Drei Zufallswerte je Fenster: an/aus, Helligkeit, Farbton.
        // Der an/aus-Zufall wandert langsam mit der Zeit -> Fenster flackern um
        float zufallAn   = n21(zelle + floor(iTime * FLACKER_TEMPO));
        float zufallHell = n21(zelle + 31.7);
        float zufallTon  = n21(zelle + 77.3);
        float an = step(1.0 - FENSTER_AN, zufallAn);       // manche ganz aus
        float hell = 0.35 + 0.65 * zufallHell;             // Helligkeits-Streuung
        // Farbton: warmgelb <-> kaltweißblau je Fenster
        vec3 fensterFarbe = mix(vec3(1.0, 0.75, 0.30), vec3(0.75, 0.85, 1.0),
                                zufallTon);
        c = mix(vec3(0.05, 0.06, 0.10), fensterFarbe,
                istFenster * an * hell * (0.4 + 0.6 * fft));
        // Dachkanten-Glow (oberste 3%)
        c += fft * vec3(0.1, 0.3, 0.5) * smoothstep(h - 0.03, h, p.y);
    }
    if (water) c *= vec3(0.4, 0.5, 0.6);  // Spiegelbild abdunkeln
    fragColor = vec4(c, 1.0);
}
