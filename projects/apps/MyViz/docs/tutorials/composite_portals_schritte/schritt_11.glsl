// ============================================================================
// Schritt 11 - Kohaerenz: eine Uhr, eine Palette, ein Tonemapping.
// Materialisierte Rekonstruktion (SSOT: CompositePortals-tutorial.md):
// Schritt 10 + gZeit/TEMPO/BELICHTUNG, Farb-Uhr in pal, Kamera mit
// Atmen+Rollen, Motivkopplung, EINE Schluss-Klammer. Ergebnis ist
// inhaltsgleich mit dem Gesamtlisting von Schritt 12 (dort Fixpunkt).
// ============================================================================

// ============================================================================
// "Composite: Portals" - drei Shader der Serie werden EIN Werk:
//   Wirt:       Stratospheric Tunnel (kondensiert) - Roehrenwand, Neon, Fenster
//   Aussenwelt: Space Debris (kondensiert)         - hinter den Fenstern (Portal)
//   Boden:      Crystal Lights (kondensiert)       - abschnittsweise (Material-Id)
// Endstand des Tutorials (Schritt 12). Braucht keine iChannels.
// ============================================================================

// ---- STELLSCHRAUBEN: GEMEINSAM ---------------------------------------------
const float TEMPO      = 1.0;    // die EINE Uhr (Kamera, Taumeln, Blinken, Atmen)
const float BELICHTUNG = 1.5;    // das EINE Tonemapping
const int   AA         = 1;      // 1 = aus, 2 = 2x2-Supersampling (4x Kosten!)

// ---- STELLSCHRAUBEN: TUNNEL (Wirtswelt) ------------------------------------
const float T_RADIUS   = 1.0;    // Grundradius der Roehre
const float T_ROEHREN  = 12.0;   // Roehren um den Umfang (ganzzahlig!)
const float T_TIEFE    = 0.10;   // Woelbung der Roehren
const float T_SPALTEN  = 6.0;    // Fensterspalten um den Umfang (ganzzahlig!)
const float T_ABSTAND  = 5.0;    // Fensterabstand entlang z
const float T_DICHTE   = 0.55;   // Anteil der Zellen mit Fenster
const float T_NEON     = 0.010;  // Helligkeit der Neonfugen
const float T_LICHT    = 1.4;    // Kamera-Scheinwerfer

// ---- STELLSCHRAUBEN: PORTAL ------------------------------------------------
const float P_MASSSTAB = 4.0;    // 1 = Aussenwelt in Weltgroesse, >1 = Diorama
const float P_ATMEN    = 0.25;   // Fenster oeffnen/schliessen sich (0 = statisch)
const float P_RAHMEN   = 0.06;   // Breite des Neon-Rahmens um jedes Portal
const vec3  D_URSPRUNG = vec3(7.0, 2.0, 13.0);   // Lage der Aussenwelt

// ---- STELLSCHRAUBEN: DEBRIS (Aussenwelt) -----------------------------------
const float D_ZELLE    = 3.0;    // Kantenlaenge einer Gitterzelle
const float D_DICHTE   = 0.50;   // Anteil belegter Zellen
const float D_GROESSE  = 0.9;    // Groessen-Budget je Teil (Zellregel!)
const float D_TAUMEL   = 1.0;    // Taumel-Tempo
const float D_PLANET_R = 60.0;   // Kruemmungsradius des Planeten
const float D_PLANET_H = 8.0;    // Abstand Weltnull -> Planetenoberflaeche
const float D_GLUT     = 1.2;    // Intensitaet des Lavagrunds

// ---- STELLSCHRAUBEN: KRISTALL (Bodenabschnitte) ----------------------------
const float K_PERIODE  = 24.0;   // Streckenperiode der Abschnitte (z)
const float K_ANTEIL   = 0.45;   // Anteil der Strecke mit Kristallboden
const float K_HOEHE    = 0.72;   // Bodenniveau unter der Tunnelachse
const float K_TIEFE    = 0.55;   // Lampenebene unter dem Boden
const float K_DICHTE   = 0.9;    // Absorption im Kristall (!= D_DICHTE!)
const float K_ZELLE    = 0.9;    // Rasterabstand der Lampen

// ---- abgeleitet ------------------------------------------------------------
const float D_MARGE   = D_ZELLE * 0.5 - 1.1 * D_GROESSE;   // Zellregel-Reserve
const vec3  D_ZENTRUM = vec3(0.0, -(D_PLANET_R + D_PLANET_H), 0.0);
const vec3  D_SONNE   = normalize(vec3(0.65, 0.28, -0.70));
// ----------------------------------------------------------------------------

#define R(a) mat2(cos(a), sin(a), -sin(a), cos(a))

const float TAU = 6.28318530;
const float PI  = 3.14159265;

float gZeit = 0.0;              // die EINE Uhr: iTime * TEMPO (mainImage setzt sie)
vec3  gAuge = vec3(0.0);        // Portal-Austritt fuer die Debris-Blase

// ---- gemeinsame Helfer (jede Funktion existiert genau EINMAL) --------------

float hash21(vec2 p) { return fract(sin(dot(p, vec2(127.1, 311.7))) * 43758.5453); }
float hash13(vec3 p) { return fract(sin(dot(p, vec3(127.1, 311.7, 74.7))) * 43758.5453); }

vec2 hash22(vec2 p)
{
    return fract(sin(vec2(dot(p, vec2(127.1, 311.7)),
                          dot(p, vec2(269.5, 183.3)))) * 43758.5453);
}

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

float fbm(vec2 p)   // Entscheid Schritt 3: die 4-Oktaven-Fassung fuer ALLE Welten
{
    float v = 0.0, a = 0.5;
    for (int i = 0; i < 4; i++) { v += a * vnoise(p); p = p * 2.03 + 11.7; a *= 0.5; }
    return v;
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

vec3 pal(float t)   // die EINE Palette - mit eingebauter, langsamer Farb-Uhr
{
    return 0.5 + 0.5 * cos(TAU * (t + gZeit * 0.012 + vec3(0.0, 0.33, 0.67)));
}

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

// ============================================================================
// WELT 2: DEBRIS (Aussenwelt) - kondensiert aus Space Debris
// ============================================================================

float debrisMap(vec3 p)
{
    vec3 id = floor(p / D_ZELLE);
    vec3 q  = mod(p, D_ZELLE) - 0.5 * D_ZELLE;

    // Zellwand-Klammer: konservative Schranke fuer alles ausserhalb der Zelle
    float wand   = D_ZELLE * 0.5 - max(abs(q.x), max(abs(q.y), abs(q.z)));
    float sicher = wand + D_MARGE;

    if (hash13(id + 4.7) > D_DICHTE) return sicher;

    float gr = D_GROESSE * (0.35 + 0.65 * hash13(id + 3.1));

    // Portal-Blase: Zellen direkt am Austrittspunkt schrumpfen weg
    vec3 zentrum = (id + 0.5) * D_ZELLE;
    gr *= smoothstep(1.0, 3.5, length(zentrum - gAuge));
    if (gr < 0.02) return sicher;

    vec3 achse = normalize(hash33(id + 5.7) - 0.5 + vec3(0.01, 0.02, 0.03));
    float tempo = (0.25 + 1.25 * hash13(id + 9.2)) * D_TAUMEL;
    q = rotAchse(achse, gZeit * tempo + TAU * hash13(id + 1.9)) * q;

    float d = sdBox(q, gr * (0.30 + 0.28 * hash33(id + 2.6)));
    d -= 0.08 * gr * sin(4.7 * q.x) * sin(4.3 * q.y) * sin(5.1 * q.z);
    return min(d, sicher);
}

float debrisMarch(vec3 ro, vec3 rd, float tMax)
{
    float t = 0.0;
    for (int i = 0; i < 60; i++) {
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
    float grund = fbm(q * 0.045 + vec2(gZeit * 0.010, 0.0));
    float adern = fbm(q * 0.16 + grund * 1.8 + 7.0);
    float glut  = pow(clamp(adern * 1.35 - 0.25, 0.0, 1.0), 2.2) * D_GLUT;
    vec3 col = vec3(0.028, 0.010, 0.012);
    col = mix(col, vec3(0.55, 0.08, 0.015), smoothstep(0.10, 0.45, glut));
    col = mix(col, vec3(1.15, 0.55, 0.10),  smoothstep(0.45, 0.85, glut));
    return col;
}

vec3 planetLicht(vec2 q)   // grobes Gluehen - auch Fensterlicht im Tunnel
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

// Schnittstelle der Aussenwelt: Strahl rein, Linearlicht raus (KEIN Tonemapping)
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

    // Dunst der Aussenwelt - in DEREN Einheiten (Massstab bleibt draussen)
    vec3 dunst = mix(vec3(0.010, 0.012, 0.022), vec3(0.30, 0.13, 0.05),
                     clamp(-rd.y * 2.2, 0.0, 1.0));
    return mix(col, dunst, 1.0 - exp(-0.0009 * tHit * tHit));
}

// ============================================================================
// WELT 3: KRISTALL (Bodenabschnitte) - kondensiert aus Crystal Lights
// ============================================================================

float kristallZone(float z)
{
    float zz = fract(z / K_PERIODE);
    return smoothstep(0.03, 0.12, zz) * (1.0 - smoothstep(K_ANTEIL - 0.09, K_ANTEIL, zz));
}

float kristallHoehe(vec2 q)
{
    float glatt  = (fbm(q * 1.1) - 0.5) * 0.16;
    float platte = (hash21(voronoi(q * 3.0).yz) - 0.5) * 0.10;
    return glatt + platte;
}

vec3 kristallLampen(vec2 q)
{
    vec2 base = floor(q / K_ZELLE);
    vec3 acc = vec3(0.0);
    for (int y = -1; y <= 1; y++)
    for (int x = -1; x <= 1; x++) {
        vec2 id = base + vec2(float(x), float(y));
        vec2 c  = (id + 0.5 + 0.6 * (hash22(id + 7.0) - 0.5)) * K_ZELLE;
        vec2 d  = q - c;
        float wv = 0.5 + 0.5 * sin(TAU * (gZeit * (0.35 + 0.75 * hash21(id + 17.3)) * 0.25
                                          + hash21(id + 31.7)));
        float hell = smoothstep(0.70, 0.97, wv) + 0.06;
        acc += pal(hash21(id) * 0.4 + 0.55) * hell / (0.02 + dot(d, d) * 40.0);
    }
    return acc * 0.06;
}

// ============================================================================
// WELT 1: TUNNEL (Wirt) - kondensiert aus Stratospheric Tunnel
// ============================================================================

float bodenHoehe(vec3 p)
{
    // Lueckentrick: ausserhalb der Zone versinkt der Boden unter die Wand
    float ziel = -K_HOEHE + kristallHoehe(p.xz);
    return mix(-(T_RADIUS + 0.8), ziel, kristallZone(p.z));
}

// map liefert (Distanz, Material-Id): 1 = Roehrenwand, 2 = Kristallboden
vec2 tunnelMap(vec3 p)
{
    float w = atan(p.y, p.x);
    float dWand = T_RADIUS - T_TIEFE * (0.5 - 0.5 * cos(w * T_ROEHREN)) - length(p.xy);

    // Boden: oben reicht eine billige Schranke, erst unten zahlt das Hoehenfeld;
    // Hoehenfeld im min-Verbund => konservativ halbieren (Schritt 8)
    float dBoden = (p.y > -0.30) ? (p.y + 0.55)
                                 : (p.y - bodenHoehe(p)) * 0.5;

    return (dWand < dBoden) ? vec2(dWand, 1.0) : vec2(dBoden, 2.0);
}

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

vec3 tunnelNormale(vec3 p)
{
    vec2 e = vec2(0.004, 0.0);
    return normalize(vec3(tunnelMap(p + e.xyy).x - tunnelMap(p - e.xyy).x,
                          tunnelMap(p + e.yxy).x - tunnelMap(p - e.yxy).x,
                          tunnelMap(p + e.yyx).x - tunnelMap(p - e.yyx).x));
}

// Fenster: signierte Kanten-Distanz (< 0 = im Fenster) + Zell-Id
float fensterDist(float w, float z, out vec2 id)
{
    vec2 zelle = vec2(fract(w / TAU + 0.5) * T_SPALTEN, z / T_ABSTAND);
    id = floor(zelle);
    if (hash21(id + 3.1) > T_DICHTE) return 1e3;

    vec2 c = fract(zelle) - 0.5;
    c.x *= TAU * T_RADIUS / T_SPALTEN;
    c.y *= T_ABSTAND;

    float o = clamp(0.8 + P_ATMEN * sin(gZeit * (0.15 + 0.25 * hash21(id + 9.4))
                                        + TAU * hash21(id)), 0.0, 1.0);
    vec2 halb = vec2(0.30, 0.85) * o;
    return max(abs(c.x) - halb.x, abs(c.y) - halb.y);
}

vec3 tunnelNeon(float w, float z)
{
    float fu  = w * T_ROEHREN / TAU;
    float gid = mod(floor(fu + 0.5), T_ROEHREN);
    float ad  = abs(fract(fu + 0.5) - 0.5) * TAU * T_RADIUS / T_ROEHREN;
    return pal(hash21(vec2(gid, 2.6)) * 0.4 + 0.55) * T_NEON / (0.0015 + ad * ad * 60.0);
}

vec3 portalRahmen(float d, vec2 id)
{
    float band = smoothstep(P_RAHMEN, 0.0, abs(d));
    float puls = 0.6 + 0.4 * sin(gZeit * (0.8 + 0.6 * hash21(id + 6.6))
                                 + TAU * hash21(id + 2.2));
    return band * puls * pal(hash21(id + 8.5) * 0.3 + 0.60) * 0.9;
}

vec3 tunnelShade(vec3 p, vec3 n, vec3 ro, float w)
{
    vec3 basis = vec3(0.06, 0.07, 0.10);
    vec3 zk = ro - p;
    float dk = max(length(zk), 1e-3);
    vec3 col = basis * (T_LICHT * max(dot(n, zk / dk), 0.0) / (1.0 + dk * dk * 0.12) + 0.02);
    col += tunnelNeon(w, p.z);

    // Motivkopplung: Licht aus dem gegenueberliegenden Fenster in Planet-Farbe
    vec2 gid;
    float einfall = smoothstep(0.06, -0.06, fensterDist(w + PI, p.z, gid));
    col += einfall * planetLicht((D_URSPRUNG + p * P_MASSSTAB).xz) * 0.35;

    return col;
}

vec3 shadeKristall(vec3 p, vec3 n, vec3 rd, vec3 ro)
{
    // vereinfachte Brechung: refract -> Ebenen-Schnitt -> Beer-Lambert
    vec3 rr = refract(rd, n, 1.0 / 1.45);
    if (dot(rr, rr) < 0.5) rr = rd;

    float ebene = -K_HOEHE - K_TIEFE;
    float tt = (ebene - p.y) / min(rr.y, -0.05);
    vec2 q = (p + rr * tt).xz;

    vec3 T = exp(-max(p.y - ebene, 0.0) * vec3(0.85, 0.30, 0.16) * K_DICHTE * 2.0);
    vec3 col = kristallLampen(q) * T;

    // Glanz des Kamera-Scheinwerfers auf den Platten
    vec3 zk = normalize(ro - p);
    col += pow(max(dot(reflect(rd, n), zk), 0.0), 40.0) * vec3(0.9, 0.95, 1.0) * 0.4;

    // Fresnel: der Boden spiegelt das Neon (Naeherung ueber die Spiegelrichtung)
    vec3 rf = reflect(rd, n);
    float fres = pow(1.0 - max(dot(n, -rd), 0.0), 3.0);
    col += fres * tunnelNeon(atan(rf.y, rf.x), p.z) * 0.6;

    return col;
}

// ---- ein Bild ---------------------------------------------------------------

vec3 render(vec2 uv)
{
    // die EINE Kamera: Vortrieb mit weichem Atmen + leichtes Rollen
    float zpos = gZeit * 1.2 + sin(gZeit * 0.20) * 5.0;
    vec3 ro = vec3(0.0, 0.0, zpos);
    vec3 rd = normalize(vec3(R(0.15 * sin(gZeit * 0.13)) * uv, 1.4));

    vec2 hit = tunnelMarch(ro, rd);
    float t = hit.x;
    vec3 p = ro + rd * t;
    float w = atan(p.y, p.x);
    vec3 n = tunnelNormale(p);

    // Material-Dispatch ueber die Id
    vec3 col = (hit.y > 1.5) ? shadeKristall(p, n, rd, ro)
                             : tunnelShade(p, n, ro, w);

    // Tunnel-Dunst
    col = mix(col, vec3(0.010, 0.014, 0.030), 1.0 - exp(-0.0016 * t * t));

    // DAS PORTAL - nur auf der Wand, nie auf dem Kristallboden
    if (hit.y < 1.5) {
        vec2 fid;
        float fd = fensterDist(w, p.z, fid);
        float px = max(fwidth(fd), 0.004);              // Kante = eine Pixelbreite
        float F  = (1.0 - smoothstep(-px, px, fd)) * exp(-0.001 * t * t);
        if (F > 0.004) {
            vec3 roD = D_URSPRUNG + p * P_MASSSTAB;     // Uebergabe in Welt 2
            col = mix(col, debrisWelt(roD, rd), F);
        }
        col += portalRahmen(fd, fid) * exp(-0.0008 * t * t);
    }
    return col;
}

void mainImage(out vec4 fragColor, in vec2 fragCoord)
{
    gZeit = iTime * TEMPO;

    vec3 col = vec3(0.0);
    for (int m = 0; m < AA; m++)
    for (int n = 0; n < AA; n++) {
        vec2 o = (vec2(float(m), float(n)) + 0.5) / float(AA) - 0.5;
        vec2 uv = (fragCoord + o - 0.5 * iResolution.xy) / iResolution.y;
        col += render(uv);
    }
    col /= float(AA * AA);

    // EINE Farbdrift + EIN Tonemapping + Gamma + Vignette - fuer ALLES
    col *= 0.86 + 0.14 * cos(iTime * 0.045 + vec3(0.0, 2.1, 4.2));
    col = 1.0 - exp(-col * BELICHTUNG);
    col = pow(col, vec3(1.0 / 2.2));

    vec2 vuv = (fragCoord - 0.5 * iResolution.xy) / iResolution.y;
    col *= 1.0 - 0.30 * dot(vuv, vuv);

    fragColor = vec4(col, 1.0);
}
