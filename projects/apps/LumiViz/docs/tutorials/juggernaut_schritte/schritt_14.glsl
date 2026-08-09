// ============================================================================
// "Juggernaut" - ein kolossaler Moloch im Dunst, von Grund auf geraymarcht.
// Endstand des Tutorials (Schritt 14). Braucht keine iChannels.
// Stil-Verwandtschaft: MilkDrop2077 vs martin - juggernaut brighter /
// juggernaut 2 dark (Riesen-Orb, 27er-Shine-Loop -> God-Rays, kubisches
// h1-Gitter -> Greebles, Dither). EIN Shader, ZWEI Licht-Stimmungen.
// ============================================================================

// ---- STELLSCHRAUBEN --------------------------------------------------------
const float STIMMUNG = 0.0;   // 0.0 = dark .. 1.0 = brighter  (DIE Stellschraube)
const float RADIUS   = 6.0;   // Radius des Molochs
const float ZELLE1   = 2.6;   // Kantenlaenge der grossen Platten
const float ZELLE2   = 0.9;   // Raster der mittleren Aufbauten (+ Positionslichter)
const float ZELLE3   = 0.32;  // Raster der feinen Rillen
const float PLATTE   = 0.35;  // Hoehenspiel der grossen Platten
const float AUFBAU   = 0.22;  // Hoehe der mittleren Aufbauten
const float FUGE     = 0.07;  // halbe Breite der Panelfugen
const float TIEFE    = 0.30;  // Fugen-Schale unter dem Nennradius
const float RILLE    = 0.02;  // Tiefe der feinen Rillen (h1-Idiom)
const float GLAETTE  = 0.05;  // Kanten-Weiche der smax-Fugen
const float DROSSEL  = 0.5;   // Marsch-Drossel (Displacement -> nur Bound!)
const float GODRAY   = 1.0;   // Staerke des volumetrischen Glows
const float STRAHLEN = 27.0;  // Strahlenkeulen der Streu-Sonne (Preset: anz = 27)
const float TEMPO    = 1.0;   // Orbit-Tempo (0.3 = meditativ)
const float NAH      = 8.5;   // Orbit-Radius nah (Ehrfurcht)
const float FERN     = 15.0;  // Orbit-Radius fern (Uebersicht)
const float DITHER   = 1.5;   // Dither gegen Banding, in 1/255-Stufen
// ----------------------------------------------------------------------------

#define R(a) mat2(cos(a), sin(a), -sin(a), cos(a))

float gStimmung = STIMMUNG;            // Laufzeit-Kopie: Anhang A laesst sie driften
vec3  gSonne    = vec3(0.0, 0.3, 1.0); // wird je Frame in kamera() gesetzt

// ---- Zufall & weiche Boolesche Ops -----------------------------------------

float hash21(vec2 p) { return fract(sin(dot(p, vec2(127.1, 311.7))) * 43758.5453); }
float hash31(vec3 p) { return fract(sin(dot(p, vec3(127.1, 311.7, 74.7))) * 43758.5453); }

float smin(float a, float b, float k)
{
    float h = max(k - abs(a - b), 0.0) / k;
    return min(a, b) - h * h * k * 0.25;
}
float smax(float a, float b, float k) { return -smin(-a, -b, k); }

// ---- Geometrie --------------------------------------------------------------

// traege Drehung um eine schiefe Achse (Objekt-Koordinaten)
vec3 gedreht(vec3 p)
{
    p.yz *= R(0.42);
    p.xz *= R(iTime * 0.02);
    return p;
}

// Abstand zur naechsten Gitterebene einer kubischen Zellteilung
float fugen(vec3 p, float zelle)
{
    vec3 q = abs(fract(p / zelle) - 0.5) * zelle;
    return zelle * 0.5 - max(q.x, max(q.y, q.z));
}

float map(vec3 p)
{
    vec3 q = gedreht(p);

    // Basis: die Riesenkugel
    float d = length(q) - RADIUS;

    // Oktave 1: grosse Platten - jede Wuerfelzelle hat ihren eigenen Radius
    vec3 z1 = floor(q / ZELLE1);
    d -= (hash31(z1) - 0.5) * PLATTE;

    // Fugen: Gitterebenen-Slab, begrenzt auf die aeussere Schale, abgezogen
    float slab   = fugen(q, ZELLE1) - FUGE;
    float schale = (RADIUS - TIEFE) - length(q);
    d = smax(d, -max(slab, schale), GLAETTE);

    // Oktave 2: mittlere Aufbauten - manche Zellen stehen als Bloecke vor
    vec3 z2 = floor(q / ZELLE2);
    d -= step(0.72, hash31(z2 + 7.0)) * AUFBAU * (0.35 + 0.65 * hash31(z2 + 13.0));

    // Oktave 3: feine Rillen - das h1-Idiom des Warp-Shaders als Displacement
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
        glow += 0.012 / (0.05 + d * d);      // God-Ray-Saat: Naehe zum Moloch
        if (d < 0.001 + 0.0008 * t) return t;
        if (t > 40.0) break;
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

// ---- Positionslichter -------------------------------------------------------

float fenster(vec3 q)    // q in Objekt-Koordinaten (dreht mit dem Moloch mit)
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

// ---- Licht: EIN Shader, ZWEI Stimmungen -------------------------------------

vec3 himmel(vec3 rd)
{
    vec3 oben  = mix(vec3(0.010, 0.012, 0.022), vec3(0.10, 0.13, 0.20), gStimmung);
    vec3 unten = mix(vec3(0.030, 0.028, 0.045), vec3(0.24, 0.20, 0.18), gStimmung);
    vec3 col = mix(unten, oben, clamp(rd.y * 1.5 + 0.5, 0.0, 1.0));

    // analytische Streu-Sonne mit Strahlenkeulen (Erbe des 27er-Shine-Loops)
    float s = max(dot(rd, gSonne), 0.0);
    vec3 seit = normalize(cross(gSonne, vec3(0.0, 1.0, 0.0)));
    vec3 hoch = cross(seit, gSonne);
    float wink = atan(dot(rd, hoch), dot(rd, seit));
    float keulen = 0.75 + 0.25 * sin(wink * STRAHLEN + iTime * 0.05);

    vec3 sonnenFarbe = mix(vec3(0.35, 0.42, 0.60), vec3(1.2, 0.9, 0.6), gStimmung);
    col += pow(s, 30.0) * keulen * sonnenFarbe * 1.2;
    col += pow(s, 5.0) * sonnenFarbe * 0.12;

    return col;
}

vec3 shade(vec3 p, vec3 rd, float t)
{
    vec3 n = calcNormal(p);

    // Stimmungs-Zutaten: dark <-> brighter
    vec3 sonnenFarbe = mix(vec3(0.30, 0.38, 0.55), vec3(1.05, 0.80, 0.55), gStimmung);
    vec3 himmelLicht = mix(vec3(0.020, 0.025, 0.045), vec3(0.10, 0.12, 0.16), gStimmung);
    float difStaerke = mix(0.6, 1.0, gStimmung);
    float rimStaerke = mix(0.55, 0.22, gStimmung);
    float speStaerke = mix(0.06, 0.30, gStimmung);

    vec3 albedo = vec3(0.16, 0.17, 0.19);    // dunkles, mattes Metall

    float dif = max(dot(n, gSonne), 0.0);
    float amb = 0.5 + 0.5 * n.y;

    vec3 col = albedo * (dif * sonnenFarbe * difStaerke + amb * himmelLicht);

    // Silhouetten-Saum: traegt die dark-Stimmung fast allein
    float rim = pow(1.0 - max(dot(n, -rd), 0.0), 3.0);
    col += rim * sonnenFarbe * rimStaerke;

    // Glanzlicht: lebt erst im brighter-Setup richtig auf
    float spe = pow(max(dot(reflect(rd, n), gSonne), 0.0), 24.0);
    col += spe * sonnenFarbe * speStaerke;

    // Positionslichter: rot im Dunkel, warm und dezent im Hellen
    vec3 lichtFarbe = mix(vec3(1.0, 0.12, 0.08), vec3(1.0, 0.75, 0.45), gStimmung);
    col += fenster(gedreht(p)) * lichtFarbe * mix(1.4, 0.8, gStimmung);

    return col;
}

// ---- Kamera -----------------------------------------------------------------

void kamera(vec2 uv, out vec3 ro, out vec3 rd)
{
    float zt = iTime * TEMPO;

    float wink   = 2.6 * sin(zt * 0.021);                       // Orbit + Umkehr
    float radius = mix(NAH, FERN, 0.5 + 0.5 * sin(zt * 0.013)); // Ehrfurcht<->Uebersicht
    float hoehe  = mix(-3.2, 0.6, 0.5 + 0.5 * sin(zt * 0.017)); // Frosch<->Augenhoehe

    ro = vec3(sin(wink) * radius, hoehe, cos(wink) * radius);

    vec3 ta = vec3(0.0, mix(1.8, -0.5, 0.5 + 0.5 * sin(zt * 0.029)), 0.0);

    vec3 fw = normalize(ta - ro);
    vec3 rt = normalize(cross(vec3(0.0, 1.0, 0.0), fw));
    vec3 up = cross(fw, rt);

    rd = normalize(fw * 1.1 + rt * uv.x + up * uv.y);   // 1.1 = Weitwinkel

    // Licht haengt an der Kamera: die Stimmung bleibt im Orbit konstant
    gSonne = normalize(mix(fw + vec3(0.0, 0.35, 0.0),
                           rt * 1.3 + vec3(0.0, 0.55, 0.0) - fw * 0.10,
                           gStimmung));
}

// ---- Hauptprogramm ----------------------------------------------------------

void mainImage(out vec4 fragColor, in vec2 fragCoord)
{
    vec2 uv = (fragCoord - 0.5 * iResolution.xy) / iResolution.y;

    gStimmung = STIMMUNG;    // Anhang A ersetzt diese Zeile durch Audio

    vec3 ro, rd;
    kamera(uv, ro, rd);

    float glow;
    float t = march(ro, rd, glow);

    vec3 col;
    if (t > 0.0) {
        col = shade(ro + rd * t, rd, t);

        // NEU (1): Dunst - im dark-Setup deutlich dichter
        float dichte = mix(0.0035, 0.0012, gStimmung);
        vec3 dunstFarbe = mix(vec3(0.020, 0.024, 0.040),
                              vec3(0.16, 0.15, 0.15), gStimmung);
        col = mix(col, dunstFarbe, 1.0 - exp(-dichte * t * t));
    } else {
        col = himmel(rd);
    }

    // God-Rays: der beim Marsch gesammelte Glow um die Silhouette
    vec3 strahlFarbe = mix(vec3(0.28, 0.34, 0.55), vec3(1.0, 0.75, 0.50), gStimmung);
    col += glow * 0.06 * GODRAY * strahlFarbe;

    // NEU (2): Farbdrift - das Bild wandert langsam durch benachbarte Toene
    col *= 0.92 + 0.08 * cos(iTime * 0.04 + vec3(0.0, 2.1, 4.2));

    // NEU (3): Tonemapping 1-exp - Belichtung haengt an der Stimmung
    col = 1.0 - exp(-col * mix(2.4, 1.6, gStimmung));

    // NEU (4): Gamma + Vignette
    col = pow(col, vec3(1.0 / 2.2));
    col *= 1.0 - 0.32 * dot(uv, uv);

    // NEU (5): Dither gegen Banding im Dunst (Erbe des Preset-Rauschens)
    col += (hash21(fragCoord + fract(iTime * 0.37) * 61.7) - 0.5) * (DITHER / 255.0);

    fragColor = vec4(col, 1.0);
}
