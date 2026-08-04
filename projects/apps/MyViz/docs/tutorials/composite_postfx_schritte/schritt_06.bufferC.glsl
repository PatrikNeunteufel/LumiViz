// ---- LumiViz-Anpassung (NICHT im Tutorial-Text) ----------------------------
// Die Buffer-Ping-Pongs des Shadertoy-Nodes filtern derzeit NEAREST
// (Qt-FBO-Default, MultiEffectVisualizer::runShadertoy) - shadertoy.com
// liest bilinear. Lesungen dieses Passes mit Zwischenpositionen (Gauss-Taps
// in Promille-Abstaenden, DOF-Gather, gefaltete Koordinaten) laufen deshalb
// ueber ein manuelles bilineares Lesen. Auf shadertoy.com ist das UNNOETIG -
// dort steht im Tutorial schlicht texture().
vec4 lesBilinear0(vec2 st)
{
    vec2 p  = st * iResolution.xy - 0.5;
    vec2 i  = floor(p);
    vec2 fr = p - i;
    vec2 px = 1.0 / iResolution.xy;
    vec2 b  = (i + 0.5) * px;
    vec4 c00 = texture(iChannel0, b);
    vec4 c10 = texture(iChannel0, b + vec2(px.x, 0.0));
    vec4 c01 = texture(iChannel0, b + vec2(0.0, px.y));
    vec4 c11 = texture(iChannel0, b + px);
    return mix(mix(c00, c10, fr.x), mix(c01, c11, fr.x), fr.y);
}
// ---- Ende LumiViz-Anpassung ------------------------------------------------

// ============================================================================
// COMMON - wird jedem Pass vorangestellt. Das SSOT der Kueche:
// ALLE Stellschrauben und geteilten Helfer wohnen hier (Muster:
// Pimped-Kaleidoscope-Tutorial, Schritt 13).
// ============================================================================

// ---- STELLSCHRAUBEN --------------------------------------------------------
// Kuechen-Monitor
const int   ANSICHT   = 0;     // 0 = fertig, 1 = Tiefe, 2 = Bloom-Leitung
// Szene (Buffer A)
const float STIMMUNG  = 0.0;   // 0.0 = dark .. 1.0 = brighter
const float RADIUS    = 6.0;   // Radius des Molochs
const float ZELLE1    = 2.6;   // Kantenlaenge der grossen Platten
const float ZELLE2    = 0.9;   // Raster der Aufbauten + Positionslichter
const float ZELLE3    = 0.32;  // Raster der feinen Rillen
const float PLATTE    = 0.35;  // Hoehenspiel der grossen Platten
const float AUFBAU    = 0.22;  // Hoehe der mittleren Aufbauten
const float FUGE      = 0.07;  // halbe Breite der Panelfugen
const float SCHALE    = 0.30;  // Fugen-Schale unter dem Nennradius
const float RILLE     = 0.02;  // Tiefe der feinen Rillen
const float GLAETTE   = 0.05;  // Kanten-Weiche der smax-Fugen
const float DROSSEL   = 0.5;   // Marsch-Drossel
const float GODRAY    = 1.0;   // volumetrischer Glow der Szene
const float TEMPO     = 1.0;   // Orbit-Tempo
const float NAH       = 8.5;   // Orbit-Radius nah
const float FERN      = 15.0;  // Orbit-Radius fern
const float TIEFE_MAX = 40.0;  // Marsch-Limit ("unendlich weit weg")
// Bloom (Buffer B + C)
const float SCHWELLE  = 0.3;   // Bright-Pass: ab dieser Leuchtdichte "ueberstrahlt"  [Chain-Kalibrierung: Original 0.7, s. make_schritte.py]
const float KNIE      = 0.5;   // weicher Uebergang oberhalb der Schwelle
const int   RAD       = 6;     // Blur-Taps je Seite (Kernel = 2*RAD+1 = 13)
const float SIGMA     = 3.0;   // Gauss-Breite in Tap-Einheiten
const float STREU     = 2.5;   // Tap-Abstand in PROMILLE der Bildhoehe
const float BLOOM_STAERKE = 0.7; // Anteil des Blooms im Endbild
// ----------------------------------------------------------------------------

#define R(a) mat2(cos(a), sin(a), -sin(a), cos(a))

float hash21(vec2 p) { return fract(sin(dot(p, vec2(127.1, 311.7))) * 43758.5453); }
float hash31(vec3 p) { return fract(sin(dot(p, vec3(127.1, 311.7, 74.7))) * 43758.5453); }

float lum(vec3 c) { return dot(c, vec3(0.299, 0.587, 0.114)); }

// zentrierte, hoehen-normierte Koordinaten -> 0..1-Texturkoordinaten
vec2 uvZuTex(vec2 uv)
{
    return uv * vec2(iResolution.y / iResolution.x, 1.0) + 0.5;
}

// ==== Ende Common - ab hier der Pass-eigene Code =========================

// BUFFER C - vertikaler Gauss ueber Buffer B: das fertige Bloom.
// iChannel0 = Buffer B

void mainImage(out vec4 fragColor, in vec2 fragCoord)
{
    vec2 uv = (fragCoord - 0.5 * iResolution.xy) / iResolution.y;

    vec3 acc = vec3(0.0);
    float wsum = 0.0;

    for (int i = -RAD; i <= RAD; i++) {
        float w = exp(-float(i * i) / (2.0 * SIGMA * SIGMA));
        acc  += lesBilinear0(uvZuTex(uv + vec2(0.0, float(i)) * STREU * 0.001)).rgb * w;
        wsum += w;
    }

    fragColor = vec4(acc / wsum, 1.0);
}
