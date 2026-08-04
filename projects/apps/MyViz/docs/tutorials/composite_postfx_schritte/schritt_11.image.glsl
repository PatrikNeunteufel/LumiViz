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
vec4 lesBilinear1(vec2 st)
{
    vec2 p  = st * iResolution.xy - 0.5;
    vec2 i  = floor(p);
    vec2 fr = p - i;
    vec2 px = 1.0 / iResolution.xy;
    vec2 b  = (i + 0.5) * px;
    vec4 c00 = texture(iChannel1, b);
    vec4 c10 = texture(iChannel1, b + vec2(px.x, 0.0));
    vec4 c01 = texture(iChannel1, b + vec2(0.0, px.y));
    vec4 c11 = texture(iChannel1, b + px);
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
// Temporal (Buffer A)
const float NACHZIEH  = 0.35;  // Vorframe-Anteil (0.0 = aus; nie >= 1.0!)
// Bloom (Buffer B + C)
const float SCHWELLE  = 0.3;   // Bright-Pass: ab dieser Leuchtdichte "ueberstrahlt"  [Chain-Kalibrierung: Original 0.7, s. make_schritte.py]
const float KNIE      = 0.5;   // weicher Uebergang oberhalb der Schwelle
const int   RAD       = 6;     // Blur-Taps je Seite (Kernel = 2*RAD+1 = 13)
const float SIGMA     = 3.0;   // Gauss-Breite in Tap-Einheiten
const float STREU     = 2.5;   // Tap-Abstand in PROMILLE der Bildhoehe
const float BLOOM_STAERKE = 0.7; // Anteil des Blooms im Endbild
// Depth of Field (Image)
const float FOKUS     = 6.0;   // Fokus-Distanz in Welteinheiten
const float FOKUS_HUB = 3.0;   // Pendel-Amplitude der Fokus-Uhr (0.0 = statisch)
const float BLENDE    = 0.012; // Blendenweite: maximaler Zerstreuungskreis (uv-Einheiten)
const float COC_MAX   = 0.015; // Sicherheitsdeckel des Zerstreuungskreises
// Kaleidoskop-Finish (Image; ab Schritt 9 optional Buffer B)
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
// (Herleitung: Pimped-Kaleidoscope-Tutorial, Schritt 8)
vec2 falteWinkel(vec2 uv, float n)
{
    float sektor = 6.28318 / n;
    float ang = atan(uv.y, uv.x);
    ang = mod(ang, sektor);
    ang = abs(ang - 0.5 * sektor);
    return length(uv) * vec2(cos(ang), sin(ang));
}

// Spiegel-Kachel: die Ebene als Teppich gespiegelter Kopien einer Zelle
// (Herleitung: Pimped-Kaleidoscope-Tutorial, Schritt 9)
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

void mainImage(out vec4 fragColor, in vec2 fragCoord)
{
    vec2 uv = (fragCoord - 0.5 * iResolution.xy) / iResolution.y;

    // NEU: das Finish - alle LESUNGEN laufen ueber die gefaltete Koordinate
    vec2 k = faltUV(uv);

    float tiefe = lesBilinear0(uvZuTex(k)).a;
    float fokus = FOKUS + FOKUS_HUB * sin(iTime * 0.11);
    float coc = min(BLENDE * abs(tiefe - fokus) / max(tiefe, 1.0), COC_MAX);

    vec3 szeneCol = vec3(0.0);
    for (int i = 0; i < DOF_TAPS; i++)
        szeneCol += lesBilinear0(uvZuTex(k + TAP[i] * coc)).rgb;
    szeneCol /= float(DOF_TAPS);

    // (4) NEU: Tiefen-Nebel auf die SZENE - der Dunst aus dem Juggernaut-
    //     Tutorial, wiederauferstanden aus dem Alpha-Kanal. Der Himmel
    //     (tiefe = TIEFE_MAX) ist per Vertrag ausgenommen: er traegt seine
    //     Dunstfarbe schon im Verlauf.
    float dichte = NEBEL * mix(0.0035, 0.0012, STIMMUNG);
    vec3 nebelFarbe = mix(vec3(0.020, 0.024, 0.040), vec3(0.16, 0.15, 0.15), STIMMUNG);
    float nebel = (tiefe < TIEFE_MAX * 0.99) ? 1.0 - exp(-dichte * tiefe * tiefe) : 0.0;
    szeneCol = mix(szeneCol, nebelFarbe, nebel);

    // (5) Bloom addieren - NACH dem Nebel: die Hoefe stechen durch den Dunst
    vec2 kBloom = (FALT_VOR_BLOOM > 0.5) ? uv : k;
    vec3 bloom  = lesBilinear1(uvZuTex(kBloom)).rgb;
    vec3 col    = szeneCol + bloom * BLOOM_STAERKE;

    if (ANSICHT == 1) { fragColor = vec4(vec3(tiefe / TIEFE_MAX), 1.0); return; }
    if (ANSICHT == 2) { fragColor = vec4(bloom, 1.0); return; }

    // (6) NEU: die echte Politur - ganz am Ende der Kette
    col *= 0.92 + 0.08 * cos(iTime * 0.04 + vec3(0.0, 2.1, 4.2));       // Farbdrift
    col = 1.0 - exp(-col * BELICHTUNG * mix(2.4, 1.6, STIMMUNG));       // Tonemapping
    col = pow(col, vec3(1.0 / 2.2));                                    // Gamma
    col *= 1.0 - VIGNETTE * dot(uv, uv);                                // Vignette (Schirm-uv!)
    col += (hash21(fragCoord + fract(iTime * 0.37) * 61.7) - 0.5) * (DITHER / 255.0);

    fragColor = vec4(col, 1.0);
}
