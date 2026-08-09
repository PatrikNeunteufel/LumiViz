// ============================================================================
// COMMON - wird jedem Pass vorangestellt. Das SSOT der Kueche:
// ALLE Stellschrauben und geteilten Helfer wohnen hier (Muster:
// Pimped-Kaleidoscope-Tutorial, Schritt 13).
// ============================================================================

// ---- STELLSCHRAUBEN --------------------------------------------------------
// Kuechen-Monitor
const int   ANSICHT   = 2;     // 0 = fertig, 1 = Tiefe, 2 = Bloom-Leitung
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

// BUFFER B (v1) - der Bright-Pass: nur das Ueberstrahlende bleibt.
// iChannel0 = Buffer A

void mainImage(out vec4 fragColor, in vec2 fragCoord)
{
    vec2 st = fragCoord / iResolution.xy;

    vec3 c = texture(iChannel0, st).rgb;

    // Schwelle mit weichem Knie: unterhalb SCHWELLE -> 0,
    // ab SCHWELLE + KNIE -> volle Farbe, dazwischen weicher Anstieg
    float faktor = smoothstep(SCHWELLE, SCHWELLE + KNIE, lum(c));

    fragColor = vec4(c * faktor, 1.0);
}
