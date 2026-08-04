// ============================================================================
// "Composite: PostFX" - der Juggernaut-Moloch hinter einer Multipass-
// Nachbearbeitungs-Kette. Endstand des Tutorials (Schritt 13).
// Szene: Juggernaut-Shader-Tutorial (kondensiert). Faltungen: Pimped-
// Kaleidoscope-Tutorial. Bloom-Kette: das Pendant zu Milkdrops GetBlur1/2/3.
// Verdrahtung: Buffer A liest SICH SELBST (iChannel0), Buffer B liest A,
// Buffer C liest B, Image liest A (iChannel0) und C (iChannel1).
// ============================================================================

// ---- STELLSCHRAUBEN --------------------------------------------------------
// Kuechen-Monitor
const int   ANSICHT   = 0;     // 0 = fertig, 1 = Tiefe, 2 = Bloom-Leitung
// Szene (Buffer A)
const float STIMMUNG  = 0.0;   // 0.0 = dark .. 1.0 = brighter (wirkt bis in die Post!)
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
const float TIEFE_MAX = 40.0;  // Marsch-Limit = "unendlich" im Tiefenkanal
// Temporal (Buffer A)
const float NACHZIEH  = 0.35;  // Vorframe-Anteil (0.0 = aus; nie >= 1.0!)
// Bloom (Buffer B + C)
const float SCHWELLE  = 0.3;   // Bright-Pass: ab dieser Leuchtdichte "ueberstrahlt"  [Chain-Kalibrierung: Original 0.7, s. make_schritte.py]
const float KNIE      = 0.5;   // weicher Uebergang oberhalb der Schwelle
const int   RAD       = 6;     // Blur-Taps je Seite (Kernel = 2*RAD+1 = 13)
const float SIGMA     = 3.0;   // Gauss-Breite in Tap-Einheiten
const float STREU     = 2.5;   // Tap-Abstand in Promille der Bildhoehe
const float BLOOM_STAERKE = 0.7; // Anteil des Blooms im Endbild
// Depth of Field (Image)
const float FOKUS     = 6.0;   // Fokus-Distanz in Welteinheiten
const float FOKUS_HUB = 3.0;   // Pendel-Amplitude der Fokus-Uhr (0.0 = statisch)
const float BLENDE    = 0.012; // Blendenweite: max. Zerstreuungskreis (uv-Einheiten)
const float COC_MAX   = 0.015; // Sicherheitsdeckel des Zerstreuungskreises
// Kaleidoskop-Finish (Image + Buffer B)
const int   FINISH    = 0;     // 0 = aus, 1 = Sektor-Faltung, 2 = Spiegel-Kachel
const float SEKTOREN  = 6.0;   // Spiegel-Sektoren (FINISH 1)
const float KACHEL    = 1.2;   // Kacheln pro Bildhoehe (FINISH 2)
const float FALT_VOR_BLOOM = 0.0; // 1.0 = das Bloom sieht das GEFALTETE Bild
// Politur (Image)
const float NEBEL     = 1.0;   // Tiefen-Nebel-Skala (Dichte haengt an STIMMUNG)
const float BELICHTUNG= 1.0;   // Skala vor dem 1-exp-Tonemapping
const float VIGNETTE  = 0.32;  // Randabdunklung
const float DITHER    = 1.5;   // Anti-Banding-Rauschen in 1/255-Stufen
// ----------------------------------------------------------------------------

// ---- STIMMUNGs-Kopplung: dark = fette Post, brighter = nuechterne Post ------
float effBloomStaerke() { return BLOOM_STAERKE * mix(1.6, 0.8, STIMMUNG); }
float effSchwelle()     { return SCHWELLE      * mix(0.85, 1.15, STIMMUNG); }
float effBlende()       { return BLENDE        * mix(1.4, 0.7, STIMMUNG); }
float effNebelDichte()  { return NEBEL * mix(0.0035, 0.0012, STIMMUNG); }

// ---- AUDIO (Common) - iChannel3 = Music in jedem lesenden Pass! ------------
float gBass = 0.0, gMid = 0.0, gTreb = 0.0, gVol = 0.0, gGate = 0.0;

float bandLevel(float lo, float hi)
{
    float sum = 0.0;
    const int N = 12;
    for (int i = 0; i < N; i++) {
        float x = mix(lo, hi, (float(i) + 0.5) / float(N));
        sum += texture(iChannel3, vec2(x, 0.25)).x;
    }
    return sum / float(N);
}

// einmal am Anfang von mainImage jedes lesenden Passes aufrufen
void audioFuellen()
{
    gBass = bandLevel(0.00, 0.05);
    gMid  = bandLevel(0.05, 0.25);
    gTreb = bandLevel(0.25, 0.70);
    gVol  = bandLevel(0.00, 0.70);
    gGate = smoothstep(0.60, 0.75, gBass);   // Beat-Gate (Skala: Handarbeit, s. Schablone)

    // LumiViz-Anpassung (die B-Regel der Serie, NICHT der Shadertoy-Text):
    // App-Uniforms statt FFT-Absolutpegel. Die dB-FFT-Zeile des Standalone-
    // Testsignals saettigt bei 1.0 (Sonde S67) - Absolut-Schwellen wie
    // smoothstep(0.60, 0.75, ...) koennen dort nie mehr schalten; `beat`
    // ersetzt das handkalibrierte Gate. Auf shadertoy.com gilt der
    // Tutorial-Text oben unveraendert.
    gBass = bass;
    gMid  = mid;
    gTreb = treb;
    gVol  = vol;
    gGate = beat;
}

// ---- geteilte Helfer --------------------------------------------------------

#define R(a) mat2(cos(a), sin(a), -sin(a), cos(a))

float hash21(vec2 p) { return fract(sin(dot(p, vec2(127.1, 311.7))) * 43758.5453); }
float hash31(vec3 p) { return fract(sin(dot(p, vec3(127.1, 311.7, 74.7))) * 43758.5453); }

float lum(vec3 c) { return dot(c, vec3(0.299, 0.587, 0.114)); }

// zentrierte, hoehen-normierte Koordinaten -> 0..1-Texturkoordinaten
vec2 uvZuTex(vec2 uv)
{
    return uv * vec2(iResolution.y / iResolution.x, 1.0) + 0.5;
}

// Winkel-Faltung: alle Richtungen in EINEN Sektor spiegeln
vec2 falteWinkel(vec2 uv, float n)
{
    float sektor = 6.28318 / n;
    float ang = atan(uv.y, uv.x);
    ang = mod(ang + gMid * 0.5, sektor);     // [4] Mitten drehen das Mandala
    ang = abs(ang - 0.5 * sektor);
    return length(uv) * vec2(cos(ang), sin(ang));
}

// Spiegel-Kachel: die Ebene als Teppich gespiegelter Kopien einer Zelle
vec2 falteKachel(vec2 uv, float dichte)
{
    return abs(fract(uv * dichte) - 0.5) / dichte;
}

// die Finish-Weiche
vec2 faltUV(vec2 uv)
{
    if (FINISH == 1) return falteWinkel(uv, SEKTOREN);
    if (FINISH == 2) return falteKachel(uv, KACHEL);
    return uv;
}

// fester Abtast-Satz fuer das Gather-DOF: 1 Zentrum + 8 auf dem Ring
const int  DOF_TAPS = 9;
const vec2 TAP[DOF_TAPS] = vec2[DOF_TAPS](
    vec2( 0.000,  0.000),
    vec2( 1.000,  0.000), vec2( 0.707,  0.707), vec2( 0.000,  1.000),
    vec2(-0.707,  0.707), vec2(-1.000,  0.000), vec2(-0.707, -0.707),
    vec2( 0.000, -1.000), vec2( 0.707, -0.707)
);

// ==== Ende Common - ab hier der Pass-eigene Code =========================

// BUFFER A - die Szene: der kondensierte Moloch.
// rgb = lineares HDR-Bild, a = Marsch-Distanz (Tiefe). KEINE Politur hier.

float smin(float a, float b, float k)
{
    float h = max(k - abs(a - b), 0.0) / k;
    return min(a, b) - h * h * k * 0.25;
}
float smax(float a, float b, float k) { return -smin(-a, -b, k); }

vec3 gSonne = vec3(0.0, 0.3, 1.0);   // wird je Frame in kamera() gesetzt

vec3 gedreht(vec3 p)
{
    p.yz *= R(0.42);
    p.xz *= R(iTime * 0.02);
    return p;
}

float fugen(vec3 p, float zelle)
{
    vec3 q = abs(fract(p / zelle) - 0.5) * zelle;
    return zelle * 0.5 - max(q.x, max(q.y, q.z));
}

float map(vec3 p)
{
    vec3 q = gedreht(p);

    float d = length(q) - RADIUS;

    vec3 z1 = floor(q / ZELLE1);
    d -= (hash31(z1) - 0.5) * PLATTE;

    float slab   = fugen(q, ZELLE1) - FUGE;
    float schale = (RADIUS - SCHALE) - length(q);
    d = smax(d, -max(slab, schale), GLAETTE);

    vec3 z2 = floor(q / ZELLE2);
    d -= step(0.72, hash31(z2 + 7.0)) * AUFBAU * (0.35 + 0.65 * hash31(z2 + 13.0));

    vec3 h = pow(abs(2.0 * fract(q / ZELLE3) - 1.0), vec3(3.0));
    d += RILLE * (h.x + h.y + h.z) * 0.33;

    return d;
}

float march(vec3 ro, vec3 rd, out float glow)
{
    glow = 0.0;
    float t = 0.0;
    for (int i = 0; i < 160; i++) {
        vec3 p = ro + rd * t;
        float d = map(p);
        glow += 0.012 / (0.05 + d * d);
        if (d < 0.001 + 0.0008 * t) return t;
        if (t > TIEFE_MAX) break;
        t += d * DROSSEL;
    }
    return -1.0;
}

vec3 calcNormal(vec3 p)
{
    const vec2 e = vec2(0.004, 0.0);
    return normalize(vec3(map(p + e.xyy) - map(p - e.xyy),
                          map(p + e.yxy) - map(p - e.yxy),
                          map(p + e.yyx) - map(p - e.yyx)));
}

float fenster(vec3 q)
{
    vec3 z = floor(q / ZELLE2);
    float an = step(0.93, hash31(z + 29.0));

    float sp = 0.10 + 0.25 * hash31(z + 3.0);
    float w  = 0.5 + 0.5 * sin(6.28318 * (iTime * sp + hash31(z + 11.0)));
    float blink = 0.25 + 0.75 * smoothstep(0.55, 0.95, w);

    vec3 lokal = (fract(q / ZELLE2) - 0.5) * ZELLE2;
    float punkt = 1.0 - smoothstep(0.06, 0.24, length(lokal));

    return an * blink * punkt;
}

vec3 himmel(vec3 rd)
{
    vec3 oben  = mix(vec3(0.010, 0.012, 0.022), vec3(0.10, 0.13, 0.20), STIMMUNG);
    vec3 unten = mix(vec3(0.030, 0.028, 0.045), vec3(0.24, 0.20, 0.18), STIMMUNG);
    vec3 col = mix(unten, oben, clamp(rd.y * 1.5 + 0.5, 0.0, 1.0));

    // Korona ohne Strahlenkeulen - das Strahlen macht die Bloom-Kette
    float s = max(dot(rd, gSonne), 0.0);
    vec3 sonnenFarbe = mix(vec3(0.35, 0.42, 0.60), vec3(1.2, 0.9, 0.6), STIMMUNG);
    col += pow(s, 30.0) * sonnenFarbe * 1.2;
    col += pow(s, 5.0) * sonnenFarbe * 0.12;

    return col;
}

vec3 shade(vec3 p, vec3 rd)
{
    vec3 n = calcNormal(p);

    vec3 sonnenFarbe = mix(vec3(0.30, 0.38, 0.55), vec3(1.05, 0.80, 0.55), STIMMUNG);
    vec3 himmelLicht = mix(vec3(0.020, 0.025, 0.045), vec3(0.10, 0.12, 0.16), STIMMUNG);
    float difStaerke = mix(0.6, 1.0, STIMMUNG);
    float rimStaerke = mix(0.55, 0.22, STIMMUNG);
    float speStaerke = mix(0.06, 0.30, STIMMUNG);

    vec3 albedo = vec3(0.16, 0.17, 0.19);

    float dif = max(dot(n, gSonne), 0.0);
    float amb = 0.5 + 0.5 * n.y;
    vec3 col = albedo * (dif * sonnenFarbe * difStaerke + amb * himmelLicht);

    float rim = pow(1.0 - max(dot(n, -rd), 0.0), 3.0);
    col += rim * sonnenFarbe * rimStaerke;

    float spe = pow(max(dot(reflect(rd, n), gSonne), 0.0), 24.0);
    col += spe * sonnenFarbe * speStaerke;

    vec3 lichtFarbe = mix(vec3(1.0, 0.12, 0.08), vec3(1.0, 0.75, 0.45), STIMMUNG);
    col += fenster(gedreht(p)) * lichtFarbe * mix(1.4, 0.8, STIMMUNG);

    return col;
}

void kamera(vec2 uv, out vec3 ro, out vec3 rd)
{
    float zt = iTime * TEMPO;

    float wink   = 2.6 * sin(zt * 0.021);
    float radius = mix(NAH, FERN, 0.5 + 0.5 * sin(zt * 0.013));
    float hoehe  = mix(-3.2, 0.6, 0.5 + 0.5 * sin(zt * 0.017));

    ro = vec3(sin(wink) * radius, hoehe, cos(wink) * radius);
    vec3 ta = vec3(0.0, mix(1.8, -0.5, 0.5 + 0.5 * sin(zt * 0.029)), 0.0);

    vec3 fw = normalize(ta - ro);
    vec3 rt = normalize(cross(vec3(0.0, 1.0, 0.0), fw));
    vec3 up = cross(fw, rt);

    rd = normalize(fw * 1.1 + rt * uv.x + up * uv.y);

    gSonne = normalize(mix(fw + vec3(0.0, 0.35, 0.0),
                           rt * 1.3 + vec3(0.0, 0.55, 0.0) - fw * 0.10,
                           STIMMUNG));
}

vec3 szene(vec2 uv, out float tiefe)
{
    vec3 ro, rd;
    kamera(uv, ro, rd);

    float glow;
    float t = march(ro, rd, glow);

    vec3 col;
    if (t > 0.0) { col = shade(ro + rd * t, rd); tiefe = t; }
    else         { col = himmel(rd);             tiefe = TIEFE_MAX; }

    vec3 strahlFarbe = mix(vec3(0.28, 0.34, 0.55), vec3(1.0, 0.75, 0.50), STIMMUNG);
    col += glow * 0.06 * GODRAY * strahlFarbe;

    return col;
}

void mainImage(out vec4 fragColor, in vec2 fragCoord)
{
    audioFuellen();                                           // iChannel3 = Music!

    vec2 uv = (fragCoord - 0.5 * iResolution.xy) / iResolution.y;

    float tiefe;
    vec3 col = szene(uv, tiefe);

    // Temporal-Glaettung: nur die FARBE mischt mit dem Vorframe -
    // die Tiefe (Geometrie) wird jeden Frame frisch geschrieben
    vec3 alt = texture(iChannel0, fragCoord / iResolution.xy).rgb;
    float nz = NACHZIEH * (1.0 - 0.6 * min(gVol * 2.0, 1.0));  // [5] Lautheit kuerzt
    col = mix(col, alt, nz);                                   //     das Gedaechtnis

    fragColor = vec4(col, tiefe);
}
