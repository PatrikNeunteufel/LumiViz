// ============================================================================
// SKELETT 2: Space Debris, kondensiert.
// Behalten:   Zellgitter + Zellregel/Zellwand-Klammer, EIN Brocken-Typ,
//             Taumeln, Sonne, Glutplanet, Atmosphaeren-Saum, eine Sternschicht.
// Gestrichen: Formbibliothek, Cluster, Blinklichter, Wolken, Parallaxe,
//             Kamera-Choreografie, Politur (bis auf Notbelichtung + Dunst).
// Schnittstelle: debrisWelt(ro, rd) -> Farbe in Linearlicht.
// ============================================================================

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

const float TAU = 6.28318530;

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
    for (int i = 0; i < 4; i++) { v += a * vnoise(p); p = p * 2.03 + 11.7; a *= 0.5; }
    return v;
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

float debrisMap(vec3 p)
{
    vec3 id = floor(p / D_ZELLE);
    vec3 q  = mod(p, D_ZELLE) - 0.5 * D_ZELLE;

    // Zellwand-Klammer: konservative Schranke fuer alles ausserhalb der Zelle
    float wand   = D_ZELLE * 0.5 - max(abs(q.x), max(abs(q.y), abs(q.z)));
    float sicher = wand + D_MARGE;

    if (hash13(id + 4.7) > D_DICHTE) return sicher;

    float gr = D_GROESSE * (0.35 + 0.65 * hash13(id + 3.1));

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

void mainImage(out vec4 fragColor, in vec2 fragCoord)
{
    vec2 uv = (fragCoord - 0.5 * iResolution.xy) / iResolution.y;

    vec3 ro = vec3(sin(iTime * 0.04) * 5.0, 1.5, iTime * 0.6);
    vec3 rd = normalize(vec3(uv, 1.4));
    rd.yz *= mat2(cos(-0.15), sin(-0.15), -sin(-0.15), cos(-0.15));  // Blick hinab

    vec3 col = debrisWelt(ro, rd);

    col = 1.0 - exp(-col * 1.3);                // Notbelichtung
    fragColor = vec4(pow(col, vec3(1.0 / 2.2)), 1.0);
}
