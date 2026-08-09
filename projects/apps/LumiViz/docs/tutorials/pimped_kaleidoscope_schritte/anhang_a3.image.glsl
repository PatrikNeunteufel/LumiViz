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

// ---- das Faltwerk ----------------------------------------------------------

// Winkel-Faltung: alle Richtungen in einen Sektor spiegeln
vec2 falteWinkel(vec2 uv, float n)
{
    float sektor = 6.28318 / n;
    float ang = atan(uv.y, uv.x);
    ang = mod(ang, sektor);
    ang = abs(ang - 0.5 * sektor);
    return length(uv) * vec2(cos(ang), sin(ang));
}

// Spiegel-Kachel: die Ebene aus gespiegelten Kopien einer Zelle
vec2 falteKachel(vec2 uv, float dichte)
{
    return abs(fract(uv * dichte) - 0.5) / dichte;
}

// Winkel-Faltung einmal, dann KOPIEN rotierte Kachel-Faltungen per max()
vec3 kaleido(vec2 uv)
{
    vec2 w = falteWinkel(uv, SEKTOREN);

    vec3 acc = vec3(0.0);
    for (int n = 1; n <= KOPIEN; n++) {
        float a = float(n) / float(KOPIEN) * 3.14159;
        vec2 k = falteKachel(R(a) * w, KACHEL);
        acc = max(acc, lesBilinear(uvZuTex(k)));
    }
    return acc;
}

// bandLevel fuer DIESEN Pass (BAND-Makro aus dem Common)
float bandLevel(float lo, float hi) { BAND(iChannel1, lo, hi, 12) return sum; }

// ---- Anzeige + Politur -----------------------------------------------------

void mainImage(out vec4 fragColor, in vec2 fragCoord)
{
    vec2 uv = (fragCoord - 0.5 * iResolution.xy) / iResolution.y;

    vec3 col = kaleido(uv);

    // [4b] Farbrotation ueber die Zeit - hoert auf die Mitten
    // (LumiViz-Anpassung wie in Buffer A: mid-Uniform statt bandLevel)
    float gMid = mid;
    col *= 0.80 + 0.20 * cos(iTime * 0.07 + gMid * 2.0 + vec3(0.0, 2.1, 4.2));

    // Entsaettigung heller Flaechen (lerp auf die Luminanz)
    float l = lum(col);
    col = mix(col, vec3(l), ENTSAETT * smoothstep(0.6, 1.6, l));

    // Tonemapping 1-exp, dann Vignette + Gamma
    col = 1.0 - exp(-col * BELICHTUNG);
    col *= 1.0 - VIGNETTE * dot(uv, uv);
    col = pow(col, vec3(1.0 / 2.2));

    fragColor = vec4(col, 1.0);
}
