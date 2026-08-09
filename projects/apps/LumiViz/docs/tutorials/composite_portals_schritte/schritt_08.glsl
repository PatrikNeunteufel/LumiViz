// ============================================================================
// Schritt 8 - Der Kristallboden: ein Hoehenfeld im min().
// Materialisierte Rekonstruktion (SSOT: CompositePortals-tutorial.md):
// Schritt 7 + hash22/voronoi (Gemeingut), kristallHoehe/bodenHoehe,
// tunnelMap mit konservativer Distanz (0.5) + billiger Schranke.
// ============================================================================

// ---- STELLSCHRAUBEN: TUNNEL ------------------------------------------------
const float T_RADIUS  = 1.0;    // Grundradius der Roehre
const float T_ROEHREN = 12.0;   // Roehren um den Umfang (ganzzahlig!)
const float T_TIEFE   = 0.10;   // Woelbung der Roehren
const float T_SPALTEN = 6.0;    // Fensterspalten um den Umfang (ganzzahlig!)
const float T_ABSTAND = 5.0;    // Fensterabstand entlang z
const float T_DICHTE  = 0.55;   // Anteil der Zellen mit Fenster
const float T_NEON    = 0.010;  // Helligkeit der Neonfugen
const float T_LICHT   = 1.4;    // Kamera-Scheinwerfer
// ----------------------------------------------------------------------------

// ---- STELLSCHRAUBEN: PORTAL (neu) ------------------------------------------
const float P_MASSSTAB = 4.0;    // 1 = Aussenwelt in Weltgroesse, >1 = Diorama
const float P_ATMEN  = 0.25;   // Fenster oeffnen/schliessen sich (0 = statisch)
const float P_RAHMEN = 0.06;   // Breite des Neon-Rahmens um jedes Portal
const vec3  D_URSPRUNG = vec3(7.0, 2.0, 13.0);   // Lage der Aussenwelt
// ----------------------------------------------------------------------------

// ---- STELLSCHRAUBEN: KRISTALL (neu) ----------------------------------------
const float K_PERIODE = 24.0;   // Streckenperiode der Bodenabschnitte (z)
const float K_ANTEIL  = 0.45;   // Anteil der Strecke mit Kristallboden
const float K_HOEHE   = 0.72;   // Bodenniveau unter der Tunnelachse
// ----------------------------------------------------------------------------

// ---- STELLSCHRAUBEN: DEBRIS ------------------------------------------------
const float D_ZELLE    = 3.0;   // Kantenlaenge einer Gitterzelle
const float D_DICHTE   = 0.50;  // Anteil belegter Zellen
const float D_GROESSE  = 0.9;   // Groessen-Budget je Teil (Zellregel!)
const float D_TAUMEL   = 1.0;   // Taumel-Tempo
const float D_PLANET_R = 60.0;  // Kruemmungsradius des Planeten
const float D_PLANET_H = 8.0;   // Abstand Weltnull -> Planetenoberflaeche
const float D_GLUT     = 1.2;   // Intensitaet des Lavagrunds

// abgeleitet - Zellregel: Umkugel (1.1 * Groesse) passt in die halbe Zelle
const float D_MARGE   = D_ZELLE * 0.5 - 1.1 * D_GROESSE;   // 1.5 - 0.99 = 0.51 > 0
const vec3  D_ZENTRUM = vec3(0.0, -(D_PLANET_R + D_PLANET_H), 0.0);
const vec3  D_SONNE   = normalize(vec3(0.65, 0.28, -0.70));
// ----------------------------------------------------------------------------

// ---- gemeinsame Helfer (jede Funktion existiert genau EINMAL) --------------
// Ersetzt die Helfer-Bloecke BEIDER Skelette.

#define R(a) mat2(cos(a), sin(a), -sin(a), cos(a))

const float TAU = 6.28318530;
const float PI  = 3.14159265;

float hash21(vec2 p) { return fract(sin(dot(p, vec2(127.1, 311.7))) * 43758.5453); }
float hash13(vec3 p) { return fract(sin(dot(p, vec3(127.1, 311.7, 74.7))) * 43758.5453); }

vec3 hash33(vec3 p)
{
    return fract(sin(vec3(dot(p, vec3(127.1, 311.7,  74.7)),
                          dot(p, vec3(269.5, 183.3, 246.1)),
                          dot(p, vec3(113.5, 271.9, 124.6)))) * 43758.5453);
}

float vnoise(vec2 p)
{
    vec2 i = floor(p), f = fract(p);
    vec2 u = f * f * (3.0 - 2.0 * f);
    return mix(mix(hash21(i),              hash21(i + vec2(1, 0)), u.x),
               mix(hash21(i + vec2(0, 1)), hash21(i + vec2(1, 1)), u.x), u.y);
}

float fbm(vec2 p)   // ENTSCHEID: die 4-Oktaven-Fassung fuer ALLE Welten (s. Text)
{
    float v = 0.0, a = 0.5;
    for (int i = 0; i < 4; i++) { v += a * vnoise(p); p = p * 2.03 + 11.7; a *= 0.5; }
    return v;
}

vec3 pal(float t) { return 0.5 + 0.5 * cos(TAU * (t + vec3(0.0, 0.33, 0.67))); }

mat3 rotAchse(vec3 a, float w)
{
    float c = cos(w), s = sin(w), k = 1.0 - c;
    return mat3(a.x * a.x * k + c,       a.y * a.x * k + a.z * s,  a.z * a.x * k - a.y * s,
                a.x * a.y * k - a.z * s, a.y * a.y * k + c,        a.z * a.y * k + a.x * s,
                a.x * a.z * k + a.y * s, a.y * a.z * k - a.x * s,  a.z * a.z * k + c);
}

float sdBox(vec3 p, vec3 b)
{
    vec3 q = abs(p) - b;
    return length(max(q, 0.0)) + min(max(q.x, max(q.y, q.z)), 0.0);
}

// NEU: 2D-Hash mit 2D-Ergebnis + Voronoi - kondensiert aus Crystal Lights.
// (Beides zu den GEMEINSAMEN Helfern legen - kuenftiges Gemeingut.)
vec2 hash22(vec2 p)
{
    return fract(sin(vec2(dot(p, vec2(127.1, 311.7)),
                          dot(p, vec2(269.5, 183.3)))) * 43758.5453);
}

vec3 voronoi(vec2 p)   // (Abstand^2, Zell-Id)
{
    vec2 i = floor(p), f = fract(p);
    float best = 8.0;
    vec2 bestId = vec2(0.0);
    for (int y = -1; y <= 1; y++)
    for (int x = -1; x <= 1; x++) {
        vec2 g = vec2(float(x), float(y));
        vec2 r = g + hash22(i + g) - f;
        float d = dot(r, r);
        if (d < best) { best = d; bestId = i + g; }
    }
    return vec3(best, bestId);
}


// Globaler Portal-Austrittspunkt - fuer die Blase in debrisMap (Schritt 4)
vec3 gAuge = vec3(0.0);

float debrisMap(vec3 p)
{
    vec3 id = floor(p / D_ZELLE);
    vec3 q  = mod(p, D_ZELLE) - 0.5 * D_ZELLE;

    // Zellwand-Klammer: konservative Schranke fuer alles ausserhalb der Zelle
    float wand   = D_ZELLE * 0.5 - max(abs(q.x), max(abs(q.y), abs(q.z)));
    float sicher = wand + D_MARGE;

    if (hash13(id + 4.7) > D_DICHTE) return sicher;

    float gr = D_GROESSE * (0.35 + 0.65 * hash13(id + 3.1));

    // Portal-Blase: Zellen direkt am Austrittspunkt schrumpfen weg -
    // sonst kleben halbe Brocken "am Glas" (Space Debris, Schritt 13)
    vec3 zentrum = (id + 0.5) * D_ZELLE;
    gr *= smoothstep(1.0, 3.5, length(zentrum - gAuge));
    if (gr < 0.02) return sicher;

    vec3 achse = normalize(hash33(id + 5.7) - 0.5 + vec3(0.01, 0.02, 0.03));
    float tempo = (0.25 + 1.25 * hash13(id + 9.2)) * D_TAUMEL;
    q = rotAchse(achse, iTime * tempo + TAU * hash13(id + 1.9)) * q;

    float d = sdBox(q, gr * (0.30 + 0.28 * hash33(id + 2.6)));
    d -= 0.08 * gr * sin(4.7 * q.x) * sin(4.3 * q.y) * sin(5.1 * q.z);
    return min(d, sicher);
}

float debrisMarch(vec3 ro, vec3 rd, float tMax)
{
    float t = 0.0;
    for (int i = 0; i < 60; i++) {                 // Budget gesenkt: 110 -> 60
        float d = debrisMap(ro + rd * t);
        if (d < 0.0015 + 0.0015 * t) return t;
        t += d * 0.7;
        if (t > tMax) break;
    }
    return -1.0;
}

vec3 debrisNormale(vec3 p)
{
    const vec2 e = vec2(0.002, -0.002);
    return normalize(e.xyy * debrisMap(p + e.xyy) + e.yyx * debrisMap(p + e.yyx) +
                     e.yxy * debrisMap(p + e.yxy) + e.xxx * debrisMap(p + e.xxx));
}

float planetHit(vec3 ro, vec3 rd)
{
    vec3 oc = ro - D_ZENTRUM;
    float b = dot(oc, rd);
    float c = dot(oc, oc) - D_PLANET_R * D_PLANET_R;
    float h = b * b - c;
    if (h < 0.0) return -1.0;
    float t = -b - sqrt(h);
    return (t > 0.0) ? t : -1.0;
}

vec3 planetFarbe(vec2 q)
{
    float grund = fbm(q * 0.045 + vec2(iTime * 0.010, 0.0));
    float adern = fbm(q * 0.16 + grund * 1.8 + 7.0);
    float glut  = pow(clamp(adern * 1.35 - 0.25, 0.0, 1.0), 2.2) * D_GLUT;
    vec3 col = vec3(0.028, 0.010, 0.012);
    col = mix(col, vec3(0.55, 0.08, 0.015), smoothstep(0.10, 0.45, glut));
    col = mix(col, vec3(1.15, 0.55, 0.10),  smoothstep(0.45, 0.85, glut));
    return col;
}

vec3 planetLicht(vec2 q)   // grobes Gluehen - spaeter auch Lichtfarbe im Tunnel!
{
    return mix(vec3(0.30, 0.05, 0.01), vec3(1.00, 0.45, 0.10),
               smoothstep(0.35, 0.80, fbm(q * 0.030)));
}

vec3 atmosphaere(vec3 ro, vec3 rd)
{
    vec3 oc = ro - D_ZENTRUM;
    float tca = -dot(oc, rd);
    if (tca < 0.0) return vec3(0.0);
    float hmin = sqrt(max(dot(oc, oc) - tca * tca, 0.0)) - D_PLANET_R;
    return exp(-max(hmin, 0.0) * 0.30) * vec3(0.90, 0.32, 0.08) * 0.55;
}

vec3 debrisHimmel(vec3 ro, vec3 rd)
{
    vec3 col = vec3(0.008, 0.010, 0.018);
    vec2 su = rd.xy / (abs(rd.z) + 0.4);
    col += vec3(0.9) * smoothstep(0.994, 1.0, hash21(floor(su * 48.0)));
    return col + atmosphaere(ro, rd);
}

vec3 debrisShade(vec3 p, vec3 n, vec3 rd)
{
    vec3 id = floor(p / D_ZELLE);
    vec3 alb = vec3(0.42, 0.44, 0.47) * (0.55 + 0.90 * hash13(id + 12.5));

    vec3 col = alb * max(dot(n, D_SONNE), 0.0) * vec3(1.30, 1.18, 1.00);
    col += pow(max(dot(reflect(rd, n), D_SONNE), 0.0), 24.0) * vec3(0.90, 0.85, 0.75) * 0.8;

    float hoehe = clamp((p.y + D_PLANET_H) / D_PLANET_H, 0.0, 2.0);
    col += max(-n.y, 0.0) * planetLicht(p.xz) * exp(-hoehe * 1.1) * 0.9;
    return col;
}

// DIE SCHNITTSTELLE: die komplette Aussenwelt als eine Funktion
vec3 debrisWelt(vec3 ro, vec3 rd)
{
    gAuge = ro;

    float tP = planetHit(ro, rd);
    float tMax = (tP > 0.0) ? min(50.0, tP) : 50.0;
    float tD = debrisMarch(ro, rd, tMax);

    vec3 col;
    float tHit;
    if (tD > 0.0 && (tP < 0.0 || tD < tP)) {
        vec3 p = ro + rd * tD;
        col = debrisShade(p, debrisNormale(p), rd);
        tHit = tD;
    } else if (tP > 0.0) {
        col = planetFarbe((ro + rd * tP).xz);
        tHit = tP;
    } else {
        return debrisHimmel(ro, rd);
    }

    // Dunst der Aussenwelt: Richtung Planet warm, Richtung All kalt
    vec3 dunst = mix(vec3(0.010, 0.012, 0.022), vec3(0.30, 0.13, 0.05),
                     clamp(-rd.y * 2.2, 0.0, 1.0));
    return mix(col, dunst, 1.0 - exp(-0.0009 * tHit * tHit));
}

// NEU: 0..1 - wo entlang der Strecke der Kristallboden liegt
float kristallZone(float z)
{
    float zz = fract(z / K_PERIODE);
    return smoothstep(0.03, 0.12, zz) * (1.0 - smoothstep(K_ANTEIL - 0.09, K_ANTEIL, zz));
}

// NEU: das Kristall-Hoehenfeld im Tunnelrahmen - Grundwellen + Plattenversatz
float kristallHoehe(vec2 q)
{
    float glatt  = (fbm(q * 1.1) - 0.5) * 0.16;
    float platte = (hash21(voronoi(q * 3.0).yz) - 0.5) * 0.10;
    return glatt + platte;
}

// NEU: die Bodenhoehe als Funktion des Ortes (ersetzt die mix-Zeile in tunnelMap)
float bodenHoehe(vec3 p)
{
    float ziel = -K_HOEHE + kristallHoehe(p.xz);
    return mix(-(T_RADIUS + 0.8), ziel, kristallZone(p.z));
}

// tunnelMap - das Hoehenfeld kommt KONSERVATIV in den min-Verbund
vec2 tunnelMap(vec3 p)
{
    float w = atan(p.y, p.x);
    float dWand = T_RADIUS - T_TIEFE * (0.5 - 0.5 * cos(w * T_ROEHREN)) - length(p.xy);

    // Boden: oben reicht eine billige Schranke, erst unten zahlt das Hoehenfeld
    float dBoden = (p.y > -0.30) ? (p.y + 0.55)
                                 : (p.y - bodenHoehe(p)) * 0.5;

    return (dWand < dBoden) ? vec2(dWand, 1.0) : vec2(dBoden, 2.0);
}
// GEAENDERT: der Marsch reicht die Id des Treffers heraus
vec2 tunnelMarch(vec3 ro, vec3 rd)
{
    float t = 0.02, id = 1.0;
    for (int i = 0; i < 90; i++) {
        vec2 dm = tunnelMap(ro + rd * t);
        id = dm.y;
        if (dm.x < 0.0015 + 0.001 * t) break;
        t += dm.x * 0.7;
        if (t > 40.0) break;
    }
    return vec2(min(t, 40.0), id);
}

// GEAENDERT: die Normale liest nur noch die Distanz-Komponente
vec3 tunnelNormale(vec3 p)
{
    vec2 e = vec2(0.004, 0.0);
    return normalize(vec3(tunnelMap(p + e.xyy).x - tunnelMap(p - e.xyy).x,
                          tunnelMap(p + e.yxy).x - tunnelMap(p - e.yxy).x,
                          tunnelMap(p + e.yyx).x - tunnelMap(p - e.yyx).x));
}

// NEU (ersetzt tunnelFenster): signierte Kanten-Distanz in Weltmass.
// < 0 = im Fenster, > 0 = auf der Wand; id = Fensterzelle (fuer Rahmen & Co.)
float fensterDist(float w, float z, out vec2 id)
{
    vec2 zelle = vec2(fract(w / TAU + 0.5) * T_SPALTEN, z / T_ABSTAND);
    id = floor(zelle);
    if (hash21(id + 3.1) > T_DICHTE) return 1e3;   // keine Oeffnung in dieser Zelle

    vec2 c = fract(zelle) - 0.5;
    c.x *= TAU * T_RADIUS / T_SPALTEN;
    c.y *= T_ABSTAND;

    // Atmen: jede Oeffnung pumpt mit eigener Phase und eigenem Tempo
    float o = clamp(0.8 + P_ATMEN * sin(iTime * (0.15 + 0.25 * hash21(id + 9.4))
                                        + TAU * hash21(id)), 0.0, 1.0);
    vec2 halb = vec2(0.30, 0.85) * o;
    return max(abs(c.x) - halb.x, abs(c.y) - halb.y);
}

// NEU: pulsierender Emissions-Rahmen entlang der Portal-Kante
vec3 portalRahmen(float d, vec2 id)
{
    float band = smoothstep(P_RAHMEN, 0.0, abs(d));
    float puls = 0.6 + 0.4 * sin(iTime * (0.8 + 0.6 * hash21(id + 6.6))
                                 + TAU * hash21(id + 2.2));
    return band * puls * pal(hash21(id + 8.5) * 0.3 + 0.60) * 0.9;
}

vec3 tunnelNeon(float w, float z)
{
    float fu  = w * T_ROEHREN / TAU;
    float gid = mod(floor(fu + 0.5), T_ROEHREN);
    float ad  = abs(fract(fu + 0.5) - 0.5) * TAU * T_RADIUS / T_ROEHREN;
    return pal(hash21(vec2(gid, 2.6)) * 0.4 + 0.55) * T_NEON / (0.0015 + ad * ad * 60.0);
}

vec3 tunnelShade(vec3 p, vec3 n, vec3 ro, float w)
{
    vec3 basis = vec3(0.06, 0.07, 0.10);
    vec3 zk = ro - p;
    float dk = max(length(zk), 1e-3);
    vec3 col = basis * (T_LICHT * max(dot(n, zk / dk), 0.0) / (1.0 + dk * dk * 0.12) + 0.02);
    col += tunnelNeon(w, p.z);
    return col;
}

// GEAENDERT: mainImage - Dispatch ueber die Id (Debug-Farbe als Platzhalter),
// und das Portal gilt NUR auf der Wand
void mainImage(out vec4 fragColor, in vec2 fragCoord)
{
    vec2 uv = (fragCoord - 0.5 * iResolution.xy) / iResolution.y;

    vec3 ro = vec3(0.0, 0.0, iTime * 1.5);
    vec3 rd = normalize(vec3(uv, 1.4));

    vec2 hit = tunnelMarch(ro, rd);
    float t = hit.x;
    vec3 p = ro + rd * t;
    float w = atan(p.y, p.x);

    vec3 col;
    if (hit.y > 1.5) col = vec3(0.10, 0.35, 0.35);          // Debug: Boden-Material
    else             col = tunnelShade(p, tunnelNormale(p), ro, w);

    col = mix(col, vec3(0.010, 0.014, 0.030), 1.0 - exp(-0.0016 * t * t));

    if (hit.y < 1.5) {
        // DAS PORTAL - jetzt auf der signierten Kanten-Distanz
        vec2 fid;
        float fd = fensterDist(w, p.z, fid);
        float F  = smoothstep(0.05, -0.05, fd) * exp(-0.001 * t * t);
        if (F > 0.004) {
            vec3 roD = D_URSPRUNG + p * P_MASSSTAB;
            col = mix(col, debrisWelt(roD, rd), F);
        }
        col += portalRahmen(fd, fid) * exp(-0.0008 * t * t);
    }

    col = 1.0 - exp(-col * 1.5);
    fragColor = vec4(pow(col, vec3(1.0 / 2.2)), 1.0);
}
