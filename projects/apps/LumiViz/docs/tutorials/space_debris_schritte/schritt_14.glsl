// ============================================================================
// "Space Debris" - taumelndes Truemmerfeld im Orbit ueber einem Glutplaneten
// Endstand des Tutorials (Schritt 14). Braucht keine iChannels.
// Stil-Verwandtschaft: martin - space debris.milk (rs_lav-Glutgrund /
// noise3-Oktaven, scol-Signalfarben, Wolken in Zeitlupe, tilt-Rollen).
// ============================================================================

// ---- STELLSCHRAUBEN --------------------------------------------------------
const float ZELLE         = 3.0;   // Kantenlaenge einer Gitterzelle
const float DICHTE        = 0.55;  // Anteil belegter Zellen (0..~0.9)
const float GROESSE_MAX   = 1.0;   // Groessen-Budget je Teil (Zellregel beachten!)
const float TAUMEL        = 1.0;   // globales Taumel-Tempo (0 = eingefroren)
const float BLINK_ANTEIL  = 0.25;  // Anteil der Teile mit Positionslicht
const float PLANET_HOEHE  = 8.0;   // Abstand Kamerabahn -> Planetenoberflaeche
const float PLANET_RADIUS = 60.0;  // Kruemmungsradius des Planeten
const float GLUT          = 1.2;   // Intensitaet des Lavagrunds
const float GLUT_LICHT    = 0.9;   // Planet-Gluehen auf den Truemmern
const float TEMPO         = 1.0;   // Gesamttempo der Kamerafahrt
const float BELICHTUNG    = 1.3;   // Tonemapping-Verstaerkung

// Abgeleitet - Zellregel: max. Umkugel-Radius eines Teils ist 1.1 * GROESSE_MAX
const float MARGE = ZELLE * 0.5 - 1.1 * GROESSE_MAX;
const vec3  PLANET_ZENTRUM = vec3(0.0, -(PLANET_RADIUS + PLANET_HOEHE), 0.0);
const vec3  SONNE = normalize(vec3(0.65, 0.28, -0.70));
// ----------------------------------------------------------------------------

vec3 gKamera = vec3(0.0);           // Kameraposition, fuer die Blase in map()

// ---- Hashes & Rauschen -----------------------------------------------------

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

float fbm(vec2 p)
{
    float v = 0.0, a = 0.5;
    for (int i = 0; i < 5; i++) { v += a * vnoise(p); p = p * 2.03 + 11.7; a *= 0.5; }
    return v;
}

// ---- Sterne ----------------------------------------------------------------

vec2 richtungsUv(vec3 rd)
{
    vec3 a = abs(rd);
    if (a.z >= a.x && a.z >= a.y) return rd.xy / a.z;
    if (a.x >= a.y)               return rd.zy / a.x;
    return rd.xz / a.y;
}

vec3 sterne(vec3 rd, vec3 kam)
{
    vec3 acc = vec3(0.0);
    for (int s = 0; s < 3; s++) {
        float fs = float(s);
        vec2 su = richtungsUv(rd) * (24.0 + 30.0 * fs) + 13.7 * fs;
        su += (kam.xz + kam.y * 0.4) * 0.5 / (1.0 + fs);   // Parallaxe je Schicht
        float h = hash21(floor(su));
        float stern = smoothstep(0.988 + 0.004 * fs, 1.0, h);
        acc += stern * (0.30 + 0.70 * fract(h * 41.7)) * (1.0 - 0.28 * fs);
    }
    return acc * vec3(0.80, 0.87, 1.00);
}

// ---- SDF-Werkzeug ----------------------------------------------------------

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

float sdTorus(vec3 p, vec2 t)
{
    vec2 q = vec2(length(p.xz) - t.x, p.y);
    return length(q) - t.y;
}

// ---- das Truemmerfeld ------------------------------------------------------

bool belegt(vec3 id)
{
    float cluster = vnoise(id.xz * 0.23 + id.y * 0.31);   // Schwarm-Regionen
    float schwelle = DICHTE * 1.6 * smoothstep(0.25, 0.75, cluster);
    return hash13(id + 4.7) < schwelle;
}

// Formbibliothek - jede Form passt in eine Umkugel vom Radius 1.1 * gr
float truemmerForm(vec3 q, vec3 id, float gr)
{
    float wForm = hash13(id + 7.3);
    float d;

    if (wForm < 0.40) {                                   // Brocken (verbeult)
        vec3 b = gr * (0.30 + 0.28 * hash33(id + 2.6));
        d = sdBox(q, b);
        d -= 0.08 * gr * sin(4.7 * q.x) * sin(4.3 * q.y) * sin(5.1 * q.z);
    } else if (wForm < 0.65) {                            // Platte / Paneel
        d = sdBox(q, gr * vec3(0.85, 0.06, 0.55));
    } else if (wForm < 0.85) {                            // Traeger / Stab
        d = sdBox(q, gr * vec3(0.08, 0.95, 0.08));
    } else {                                              // Ring
        d = sdTorus(q, gr * vec2(0.62, 0.10));
    }
    return d;
}

float map(vec3 p)
{
    vec3 id = floor(p / ZELLE);
    vec3 q  = mod(p, ZELLE) - 0.5 * ZELLE;

    // Zellwand-Klammer: konservative Schranke fuer alles ausserhalb der Zelle
    float wand = ZELLE * 0.5 - max(abs(q.x), max(abs(q.y), abs(q.z)));
    float sicher = wand + MARGE;

    if (!belegt(id)) return sicher;

    // Groesse je Zelle + Kamera-Blase (Zellen an der Kamera schrumpfen weg)
    float gr = GROESSE_MAX * (0.35 + 0.65 * hash13(id + 3.1));
    vec3 zentrum = (id + 0.5) * ZELLE;
    gr *= smoothstep(1.2, 4.5, length(zentrum - gKamera));
    if (gr < 0.02) return sicher;

    // Taumeln: Achse, Tempo, Phase = Konstanten der Zelle; nur der Winkel laeuft
    vec3 achse = normalize(hash33(id + 5.7) - 0.5 + vec3(0.01, 0.02, 0.03));
    float tempo = (0.25 + 1.25 * hash13(id + 9.2)) * TAUMEL;
    float phase = 6.28318 * hash13(id + 1.9);
    q = rotAchse(achse, iTime * tempo + phase) * q;

    return min(truemmerForm(q, id, gr), sicher);
}

float marchDebris(vec3 ro, vec3 rd, float tMax)
{
    float t = 0.0;
    for (int i = 0; i < 110; i++) {
        float d = map(ro + rd * t);
        if (d < 0.001 + 0.0012 * t) return t;
        if (t > tMax) break;
        t += d * 0.7;                       // Drossel: Reserve fuer die Beulen
    }
    return -1.0;
}

vec3 calcNormal(vec3 p)
{
    const vec2 e = vec2(0.0012, -0.0012);
    return normalize(e.xyy * map(p + e.xyy) + e.yyx * map(p + e.yyx) +
                     e.yxy * map(p + e.yxy) + e.xxx * map(p + e.xxx));
}

// ---- der Planet ------------------------------------------------------------

float planetHit(vec3 ro, vec3 rd)
{
    vec3 oc = ro - PLANET_ZENTRUM;
    float b = dot(oc, rd);
    float c = dot(oc, oc) - PLANET_RADIUS * PLANET_RADIUS;
    float h = b * b - c;
    if (h < 0.0) return -1.0;
    float t = -b - sqrt(h);
    return (t > 0.0) ? t : -1.0;
}

vec3 lavaFarbe(vec2 q)
{
    float grund = fbm(q * 0.045 + vec2(iTime * 0.010, 0.0));
    float adern = fbm(q * 0.16 + grund * 1.8 + 7.0);
    float glut  = pow(clamp(adern * 1.35 - 0.25, 0.0, 1.0), 2.2) * GLUT;

    vec3 col = vec3(0.028, 0.010, 0.012);
    col = mix(col, vec3(0.55, 0.08, 0.015), smoothstep(0.10, 0.45, glut));
    col = mix(col, vec3(1.15, 0.55, 0.10),  smoothstep(0.45, 0.85, glut));
    col = mix(col, vec3(1.60, 1.25, 0.55),  smoothstep(0.85, 1.05, glut));
    return col;
}

vec3 planetLicht(vec2 q)
{
    float g = fbm(q * 0.030 + vec2(iTime * 0.010, 0.0));
    return mix(vec3(0.30, 0.05, 0.01), vec3(1.00, 0.45, 0.10),
               smoothstep(0.35, 0.80, g));
}

vec3 atmosphaere(vec3 ro, vec3 rd)
{
    vec3 oc = ro - PLANET_ZENTRUM;
    float tca = -dot(oc, rd);
    if (tca < 0.0) return vec3(0.0);
    float hmin = sqrt(max(dot(oc, oc) - tca * tca, 0.0)) - PLANET_RADIUS;
    float saum = exp(-max(hmin, 0.0) * 0.30);
    return saum * vec3(0.90, 0.32, 0.08) * 0.55;
}

vec3 shadePlanet(vec3 p, float t)
{
    vec3 col = lavaFarbe(p.xz);

    float cld = fbm(p.xz * 0.020 + iTime * vec2(0.020, 0.008));  // Zeitlupe!
    float w = smoothstep(0.45, 0.75, cld);
    col = mix(col, vec3(0.050, 0.035, 0.030) + col * 0.25, w);

    col = mix(col, vec3(0.45, 0.18, 0.06), 1.0 - exp(-t * 0.010));
    return col;
}

// ---- Licht & Material ------------------------------------------------------

vec3 signalFarbe(float x)   // scol-Erbe: rotierende Signalfarben
{
    return 0.1 + 0.9 * clamp(0.5 + sin(3.14159 / 6.0 *
        (12.0 * x + iTime / 4.0 + vec3(3.0, -1.0, -5.0))), 0.0, 1.0);
}

float blink(vec3 id)
{
    float gate = step(hash13(id + 4.4), BLINK_ANTEIL);
    float ph = hash13(id + 8.8);
    float sp = 0.5 + 1.3 * hash13(id + 6.6);
    float w  = 0.5 + 0.5 * sin(6.28318 * (iTime * sp * 0.35 + ph));
    return gate * smoothstep(0.82, 0.96, w);
}

vec3 shadeDebris(vec3 p, vec3 n, vec3 rd)
{
    vec3 id = floor(p / ZELLE);

    float ton = hash13(id + 12.5);
    vec3 alb = vec3(0.42, 0.44, 0.47) * (0.55 + 0.90 * ton);
    alb = mix(alb, alb * vec3(1.12, 0.94, 0.78), hash13(id + 15.1));

    // (1) Sonne: hart, fast weiss, KEIN Umgebungslicht
    float dif = max(dot(n, SONNE), 0.0);
    vec3 col = alb * dif * vec3(1.30, 1.18, 1.00);

    // (2) Metall-Glint
    float spec = pow(max(dot(reflect(rd, n), SONNE), 0.0), 24.0);
    col += spec * vec3(0.90, 0.85, 0.75) * 0.8;

    // (3) Silhouetten-Gegenlicht (nur Richtung Sonne)
    float fres = pow(1.0 - max(dot(n, -rd), 0.0), 4.0);
    col += fres * vec3(1.00, 0.85, 0.60) * 0.35 * clamp(dot(rd, SONNE), 0.0, 1.0);

    // (4) Planet-Gluehen von unten, Farbe aus dem Untergrund
    float unten = max(dot(n, vec3(0.0, -1.0, 0.0)), 0.0);
    float hoehe = clamp((p.y + PLANET_HOEHE) / PLANET_HOEHE, 0.0, 2.0);
    col += unten * planetLicht(p.xz) * GLUT_LICHT * exp(-hoehe * 1.1);

    // (5) Positionslicht: Emission, unabhaengig von jeder Beleuchtung
    col += blink(id) * signalFarbe(hash13(id + 3.3)) * 1.8;

    return col;
}

// ---- Kamera ----------------------------------------------------------------

void kamera(vec2 uv, out vec3 ro, out vec3 rd)
{
    float zt = iTime * TEMPO;

    ro = vec3(sin(zt * 0.041) * 6.0,          // Drift: sin-Position =>
              sin(zt * 0.033) * 2.5,          // weiche Umkehr an den Enden
              sin(zt * 0.026) * 7.0);

    float gier = 0.5 + 0.85 * sin(zt * 0.019);
    float nick = mix(-0.55, 0.10, 0.5 + 0.5 * sin(zt * 0.023));  // Nick-Uhr
    float roll = 0.35 * sin(zt * 0.017);                          // tilt-Erbe

    vec3 fw = vec3(cos(nick) * sin(gier), sin(nick), cos(nick) * cos(gier));
    vec3 rt = normalize(cross(vec3(0.0, 1.0, 0.0), fw));
    vec3 up = cross(fw, rt);

    vec3 rt2 = rt * cos(roll) + up * sin(roll);
    up = up * cos(roll) - rt * sin(roll);
    rt = rt2;

    rd = normalize(fw * 1.4 + rt * uv.x + up * uv.y);
}

// ---- Hauptprogramm ---------------------------------------------------------

void mainImage(out vec4 fragColor, in vec2 fragCoord)
{
    vec2 uv = (fragCoord - 0.5 * iResolution.xy) / iResolution.y;

    vec3 ro, rd;
    kamera(uv, ro, rd);
    gKamera = ro;                        // VOR dem Marsch (Kamera-Blase)

    float tP = planetHit(ro, rd);
    float tMax = (tP > 0.0) ? min(60.0, tP + 0.5) : 60.0;
    float tD = marchDebris(ro, rd, tMax);

    vec3 color;
    float tHit = -1.0;

    if (tD > 0.0 && (tP < 0.0 || tD < tP)) {
        vec3 p = ro + rd * tD;
        color = shadeDebris(p, calcNormal(p), rd);
        tHit = tD;
    } else if (tP > 0.0) {
        color = shadePlanet(ro + rd * tP, tP);
        tHit = tP;
    } else {
        color = vec3(0.008, 0.010, 0.018) + sterne(rd, ro);
        color += atmosphaere(ro, rd);
    }

    // NEU (1): Dunst - die Ferne versinkt, Richtung Planet waermer als zum All
    if (tHit > 0.0) {
        vec3 dunstFarbe = mix(vec3(0.010, 0.012, 0.022),
                              vec3(0.30, 0.13, 0.05),
                              clamp(-rd.y * 2.2, 0.0, 1.0));
        color = mix(color, dunstFarbe, 1.0 - exp(-0.0009 * tHit * tHit));
    }

    // NEU (2): Farbdrift - das Gesamtbild wandert in Zeitlupe durch die Toene
    color *= 0.88 + 0.12 * cos(iTime * 0.04 + vec3(0.0, 2.1, 4.2));

    // NEU (3): Tonemapping 1 - exp(-x): Glut und Glints gluehen aus statt zu clippen
    color = 1.0 - exp(-color * BELICHTUNG);

    // NEU (4): Gamma + Vignette
    color = pow(color, vec3(1.0 / 2.2));
    color *= 1.0 - 0.32 * dot(uv, uv);

    fragColor = vec4(color, 1.0);
}
