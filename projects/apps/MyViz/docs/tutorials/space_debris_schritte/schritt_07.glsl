// ---- STELLSCHRAUBEN (erweitert) --------------------------------------------
const float ZELLE       = 3.0;   // Kantenlaenge einer Gitterzelle
const float DICHTE      = 0.55;  // Anteil belegter Zellen (0 = leer .. ~0.9 = voll)
const float GROESSE_MAX = 1.0;   // Groessen-Budget je Truemmerteil (s. Zellregel!)
const float TAUMEL      = 1.0;   // globales Taumel-Tempo (0 = eingefroren)

const float PLANET_HOEHE  = 8.0;   // Abstand Kamerabahn -> Planetenoberflaeche
const float PLANET_RADIUS = 60.0;  // Kruemmungsradius (gross = flacher Horizont)
const float GLUT          = 1.2;   // Intensitaet des Lavagrunds
const vec3 PLANET_ZENTRUM = vec3(0.0, -(PLANET_RADIUS + PLANET_HOEHE), 0.0);

const float MARGE = ZELLE * 0.5 - 1.1 * GROESSE_MAX;
// ----------------------------------------------------------------------------

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

// NEU: fraktales Rauschen - unser noise3-Gegenstueck (Oktaven: feiner + leiser)
float fbm(vec2 p)
{
    float v = 0.0, a = 0.5;
    for (int i = 0; i < 5; i++) { v += a * vnoise(p); p = p * 2.03 + 11.7; a *= 0.5; }
    return v;
}

vec2 richtungsUv(vec3 rd)
{
    vec3 a = abs(rd);
    if (a.z >= a.x && a.z >= a.y) return rd.xy / a.z;
    if (a.x >= a.y)               return rd.zy / a.x;
    return rd.xz / a.y;
}

vec3 sterne(vec3 rd)
{
    vec3 acc = vec3(0.0);
    for (int s = 0; s < 3; s++) {
        float fs = float(s);
        vec2 su = richtungsUv(rd) * (24.0 + 30.0 * fs) + 13.7 * fs;
        float h = hash21(floor(su));
        float stern = smoothstep(0.988 + 0.004 * fs, 1.0, h);
        acc += stern * (0.30 + 0.70 * fract(h * 41.7)) * (1.0 - 0.28 * fs);
    }
    return acc * vec3(0.80, 0.87, 1.00);
}

bool belegt(vec3 id)
{
    float cluster = vnoise(id.xz * 0.23 + id.y * 0.31);
    float schwelle = DICHTE * 1.6 * smoothstep(0.25, 0.75, cluster);
    return hash13(id + 4.7) < schwelle;
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

float sdTorus(vec3 p, vec2 t)
{
    vec2 q = vec2(length(p.xz) - t.x, p.y);
    return length(q) - t.y;
}

float truemmerForm(vec3 q, vec3 id, float gr)
{
    float wForm = hash13(id + 7.3);
    float d;

    if (wForm < 0.40) {
        vec3 b = gr * (0.30 + 0.28 * hash33(id + 2.6));
        d = sdBox(q, b);
        d -= 0.08 * gr * sin(4.7 * q.x) * sin(4.3 * q.y) * sin(5.1 * q.z);
    } else if (wForm < 0.65) {
        d = sdBox(q, gr * vec3(0.85, 0.06, 0.55));
    } else if (wForm < 0.85) {
        d = sdBox(q, gr * vec3(0.08, 0.95, 0.08));
    } else {
        d = sdTorus(q, gr * vec2(0.62, 0.10));
    }
    return d;
}

float map(vec3 p)
{
    vec3 id = floor(p / ZELLE);
    vec3 q  = mod(p, ZELLE) - 0.5 * ZELLE;

    float wand = ZELLE * 0.5 - max(abs(q.x), max(abs(q.y), abs(q.z)));
    float sicher = wand + MARGE;

    if (!belegt(id)) return sicher;

    vec3 achse = normalize(hash33(id + 5.7) - 0.5 + vec3(0.01, 0.02, 0.03));
    float tempo = (0.25 + 1.25 * hash13(id + 9.2)) * TAUMEL;
    float phase = 6.28318 * hash13(id + 1.9);
    q = rotAchse(achse, iTime * tempo + phase) * q;

    float gr = GROESSE_MAX * (0.35 + 0.65 * hash13(id + 3.1));
    return min(truemmerForm(q, id, gr), sicher);
}

// NEU: Kugelschnitt - reine Algebra, kein Marsch
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

// NEU: der Glutgrund - Kontinente aus Glut, Adern aus verbogenem Noise
vec3 lavaFarbe(vec2 q)
{
    float grund = fbm(q * 0.045 + vec2(iTime * 0.010, 0.0));   // grosse Glut-Kontinente
    float adern = fbm(q * 0.16 + grund * 1.8 + 7.0);           // Noise verbiegt Noise
    float glut  = pow(clamp(adern * 1.35 - 0.25, 0.0, 1.0), 2.2) * GLUT;

    vec3 col = vec3(0.028, 0.010, 0.012);                       // dunkle Kruste
    col = mix(col, vec3(0.55, 0.08, 0.015), smoothstep(0.10, 0.45, glut));
    col = mix(col, vec3(1.15, 0.55, 0.10),  smoothstep(0.45, 0.85, glut));
    col = mix(col, vec3(1.60, 1.25, 0.55),  smoothstep(0.85, 1.05, glut));
    return col;
}

// NEU: Planet-Shading (Wolken und Atmosphaere folgen in Schritt 8)
vec3 shadePlanet(vec3 p, float t)
{
    vec3 col = lavaFarbe(p.xz);
    col *= exp(-t * 0.012);                 // Ferne daempfen (Provisorium)
    return col;
}

// GEAENDERT: der Marsch bekommt eine Obergrenze (nicht hinter den Planeten marschieren)
float marchDebris(vec3 ro, vec3 rd, float tMax)
{
    float t = 0.0;
    for (int i = 0; i < 110; i++) {
        float d = map(ro + rd * t);
        if (d < 0.001 + 0.0012 * t) return t;
        if (t > tMax) break;
        t += d * 0.7;
    }
    return -1.0;
}

vec3 calcNormal(vec3 p)
{
    const vec2 e = vec2(0.0012, -0.0012);
    return normalize(e.xyy * map(p + e.xyy) + e.yyx * map(p + e.yyx) +
                     e.yxy * map(p + e.yxy) + e.xxx * map(p + e.xxx));
}

void mainImage(out vec4 fragColor, in vec2 fragCoord)
{
    vec2 uv = (fragCoord - 0.5 * iResolution.xy) / iResolution.y;

    vec3 ro = vec3(0.4, 0.3, iTime * 0.9);
    vec3 rd = normalize(vec3(uv, 1.4));
    // Fuer den Screenshot: Blick testweise zum Planeten kippen (siehe Tutorial-
    // Text zu Schritt 7 - die Choreografie uebernimmt erst Schritt 12).
    // -0.7 statt der -0.4 aus dem Text: erst damit liegt der Horizont im Bild
    // und der Blickpfad durchs dichte Feld wird kurz genug fuer freie Sicht.
    rd.yz *= mat2(cos(-0.7), sin(-0.7), -sin(-0.7), cos(-0.7));

    float tP = planetHit(ro, rd);
    float tMax = (tP > 0.0) ? min(60.0, tP + 0.5) : 60.0;
    float tD = marchDebris(ro, rd, tMax);

    vec3 color;
    if (tD > 0.0 && (tP < 0.0 || tD < tP)) {
        vec3 p = ro + rd * tD;
        vec3 n = calcNormal(p);
        float dif = max(dot(n, normalize(vec3(0.65, 0.28, -0.70))), 0.0);
        color = vec3(0.05) + dif * vec3(0.85);
        color *= 0.4 + 0.6 * hash33(floor(p / ZELLE));
    } else if (tP > 0.0) {
        color = shadePlanet(ro + rd * tP, tP);
    } else {
        color = vec3(0.008, 0.010, 0.018) + sterne(rd);
    }

    fragColor = vec4(color, 1.0);
}
