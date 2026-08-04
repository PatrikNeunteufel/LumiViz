// ---- LumiViz-Anpassung (NICHT im Tutorial-Text) ----------------------------
// Die Buffer-Ping-Pongs des Shadertoy-Nodes filtern derzeit NEAREST
// (Qt-FBO-Default, MultiEffectVisualizer::runShadertoy) - der bilineare
// Tiefpass, den Schritt 6 als Stabilisator des Sharpen voraussetzt (und den
// Shadertoy-Buffer mitbringen), fehlt damit: das System kocht in Pixelgriess
// hoch (exakt die "Probe aufs Exempel" aus Schritt 6). lesBilinear() stellt
// die Shadertoy-Lesesemantik im Shader selbst her. Auf shadertoy.com ist
// diese Funktion UNNOETIG - dort steht im Tutorial schlicht texture().
vec3 lesBilinear(vec2 st)
{
    vec2 p  = st * iResolution.xy - 0.5;
    vec2 i  = floor(p);
    vec2 fr = p - i;
    vec2 px = 1.0 / iResolution.xy;
    vec2 b  = (i + 0.5) * px;
    vec3 c00 = texture(iChannel0, b).rgb;
    vec3 c10 = texture(iChannel0, b + vec2(px.x, 0.0)).rgb;
    vec3 c01 = texture(iChannel0, b + vec2(0.0, px.y)).rgb;
    vec3 c11 = texture(iChannel0, b + px).rgb;
    return mix(mix(c00, c10, fr.x), mix(c01, c11, fr.x), fr.y);
}
// ---- Ende LumiViz-Anpassung ------------------------------------------------

// ============================================================================
// "Pimped Kaleidoscope" - Feedback-Kaleidoskop, Endstand des Tutorials.
// Stil-Vorbild: martin - shader pimped caleidoscope.milk
// (Warp: Sharpen/Decay/Dither -> Buffer A; Comp: Faltwerk/Seeds -> hier).
// Aufbau: Common (dieser Tab) + Buffer A (iChannel0 = Buffer A!) + Image
// (iChannel0 = Buffer A). Braucht keine weiteren iChannels.
// ============================================================================

// ---- STELLSCHRAUBEN --------------------------------------------------------
// Feedback (Buffer A)
const float DECAY      = 0.90;   // Daempfung je Frame (< 1, sonst Explosion!)
const float SHARPEN    = 0.35;   // Unsharp-Mask-Staerke (Preset: 0.35)
const float DITHER     = 0.004;  // Rausch-Saat je Frame
const float ZOOM       = 1.010;  // Grund-Zoom der Lese-UV (> 1 = auswaerts)
const float TEMPO      = 1.0;    // Tempo der unsichtbaren Kamera
// Lichtsaat (Buffer A)
const float SEED_HELL  = 1.0;    // Helligkeit der Seeds
const float BAHN_WEITE = 0.35;   // Radius der Lissajous-Bahn
// Kaleidoskop (Image)
const float SEKTOREN   = 6.0;    // Spiegel-Sektoren der Winkel-Faltung
const float KACHEL     = 1.6;    // Kacheln pro Bildhoehe (Spiegel-Kachel)
const int   KOPIEN     = 3;      // rotierte Kopien ("anz" im Preset)
// Politur (Image)
const float ENTSAETT   = 0.25;   // Entsaettigung heller Flaechen
const float VIGNETTE   = 0.65;   // Randabdunklung
const float BELICHTUNG = 1.6;    // Verstaerkung vor dem 1-exp-Tonemapping
// ----------------------------------------------------------------------------

#define R(a) mat2(cos(a), sin(a), -sin(a), cos(a))

float hash21(vec2 p)
{
    return fract(sin(dot(p, vec2(127.1, 311.7))) * 43758.5453);
}

vec3 palette(float t)
{
    return 0.55 + 0.45 * cos(6.28318 * (t + vec3(0.0, 0.33, 0.67)));
}

float lum(vec3 c) { return dot(c, vec3(0.299, 0.587, 0.114)); }

// zentrierte, hoehen-normierte Koordinaten <-> 0..1-Texturkoordinaten
vec2 uvZuTex(vec2 uv)
{
    return uv * vec2(iResolution.y / iResolution.x, 1.0) + 0.5;
}

// ---- AUDIO -----------------------------------------------------------------
// Bandpegel aus Zeile 0 (FFT) der Music-Textur am uebergebenen Kanal
#define BAND(ch, lo, hi, N)                                              \
    float sum = 0.0;                                                     \
    for (int i = 0; i < N; i++)                                          \
        sum += texture(ch, vec2(mix(lo, hi, (float(i)+0.5)/float(N)),    \
                                0.25)).x;                                \
    sum /= float(N);
// ----------------------------------------------------------------------------

// ==== Ende Common - ab hier der Pass-eigene Code =========================

// ---- Lichtsaat -------------------------------------------------------------

vec3 seeds(vec2 uv)
{
    vec3 acc = vec3(0.0);

    // Punkte-Paar: eine Lissajous-Bahn, zwei Lichter punktsymmetrisch
    vec2 pos = vec2(sin(iTime * 0.31), sin(iTime * 0.23))
             * BAHN_WEITE * vec2(1.0, 0.7);
    vec3 farbe = palette(iTime * 0.021);

    vec2 d1 = uv - pos;
    vec2 d2 = uv + pos;
    acc += farbe     * 0.0006 / (0.0004 + dot(d1, d1));
    acc += farbe.bgr * 0.0006 / (0.0004 + dot(d2, d2));

    // Ring: leuchtende Kreislinie mit atmendem Radius
    float r  = 0.26 + 0.10 * sin(iTime * 0.171);
    float dr = length(uv) - r;
    acc += palette(iTime * 0.021 + 0.5) * 0.0012 / (0.0008 + 8.0 * dr * dr);

    return acc * SEED_HELL;
}

// ---- Feedback-Werkzeug -----------------------------------------------------

// kleiner Kreuz-Blur als GetBlur1-Ersatz
vec3 blur4(vec2 st)
{
    vec2 px = 1.0 / iResolution.xy;
    return ( lesBilinear(st + vec2( px.x, 0.0))
           + lesBilinear(st + vec2(-px.x, 0.0))
           + lesBilinear(st + vec2(0.0,  px.y))
           + lesBilinear(st + vec2(0.0, -px.y)) ) * 0.25;
}

// ---- Audio (Anhang A3) -----------------------------------------------------

// bandLevel fuer DIESEN Pass (BAND-Makro aus dem Common setzt den Kanal ein)
float bandLevel(float lo, float hi) { BAND(iChannel1, lo, hi, 12) return sum; }

// [6] Wellenform-Seed: Zeile 1 der Audio-Textur als leuchtende Kurve stempeln.
// Jede x-Position des Bildes liest "ihr" Sample; der 1/d2-Trick aus Schritt 2
// macht aus dem Hoehenabstand zur Kurve eine leuchtende Linie.
vec3 waveSeed(vec2 uv, vec2 fragCoord)
{
    float sx = fragCoord.x / iResolution.x;            // 0..1 quer durchs Bild
    float w  = texture(iChannel1, vec2(sx, 0.75)).x;   // Wellenform-Zeile
    float dy = uv.y - (w - 0.5) * 0.45;                // Abstand zur Kurve
    return palette(iTime * 0.021 + 0.25) * 0.25
         * 0.0016 / (0.0016 + dy * dy * 60.0);
}

// ---- der Kreislauf ---------------------------------------------------------

void mainImage(out vec4 fragColor, in vec2 fragCoord)
{
    vec2 uv = (fragCoord - 0.5 * iResolution.xy) / iResolution.y;
    float zt = iTime * TEMPO;

    // LumiViz-Anpassung (Regel aus Anhang B2, NICHT der Shadertoy-
    // Text): App-Uniforms statt FFT-Absolutpegel. Die dB-FFT-Zeile
    // des Standalone-Testsignals saettigt bei 1.0 (Sonde S67) -
    // Absolut-Schwellen wie smoothstep(0.60, 0.75, ...) koennen
    // dort nie mehr schalten; `beat` ersetzt das handkalibrierte
    // Gate, exakt wie B2 es vorschreibt. Auf shadertoy.com gilt
    // weiter der Tutorial-Text (bandLevel + Schwellen).
    float gBass = bass;
    float gMid  = mid;
    float gTreb = treb;
    float gVol  = vol;
    float gGate = beat;

    // [2] Decay atmet mit der Lautheit (Klammer nach oben ist Pflicht!)
    float decay = min(DECAY + 0.07 * gVol, 0.97);

    // die unsichtbare Kamera: Zentrum-Drift, Dreh-GESCHWINDIGKEIT, Zoom-Atmen
    vec2  zentrum = vec2(sin(zt * 0.043), sin(zt * 0.037)) * 0.10;
    float dreh    = 0.010 * sin(zt * 0.021);
    float zoom    = ZOOM + 0.006 * sin(zt * 0.029);

    // Vorframe transformiert lesen (Frame 0 = Schwarz: der Kaltstart)
    vec2 lese = zentrum + R(-dreh) * ((uv - zentrum) / zoom);
    vec2 st   = uvZuTex(lese);
    vec3 alt  = lesBilinear(st);

    // Unsharp Mask: feine Details verstaerkt zurueckgeben
    // [3] Sharpen mit den Hoehen
    alt += (alt - blur4(st)) * SHARPEN * (0.5 + 1.8 * gTreb);

    // Decay + frische Saat; nie unter Null (Float-Buffer clampen nicht!)
    // [1]+[6] Saat: Beat-Zuendung + Wellenform-Seed
    vec3 saat = seeds(uv) * (1.0 + 2.5 * gGate);
    saat += waveSeed(uv, fragCoord);

    vec3 neu = max(alt * decay + saat, 0.0);

    // Dither-Saat: je Frame anderes Muster, im Mittel leicht positiv
    vec2 dsh = fragCoord + vec2(float(iFrame % 289), float(iFrame % 283)) * 1.618;
    neu += (hash21(dsh) - 0.45) * DITHER;

    fragColor = vec4(max(neu, 0.0), 1.0);
}
