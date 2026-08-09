// ---- STELLSCHRAUBEN (erweitert) --------------------------------------------
const float ZELLE       = 3.0;   // Kantenlaenge einer Gitterzelle
const float DICHTE      = 0.55;  // Anteil belegter Zellen (0 = leer .. ~0.9 = voll)
const float GROESSE_MAX = 1.0;   // Groessen-Budget je Truemmerteil (s. Zellregel!)

// Abgeleitet: Sicherheitsabstand Zellwand -> naechstmoegliche Nachbar-Oberflaeche.
// 1.1 * GROESSE_MAX ist der maximale Umkugel-Radius eines Teils (Schritt 5).
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

// Value-Noise (2D) - fuer die groben Cluster-Regionen
float vnoise(vec2 p)
{
    vec2 i = floor(p), f = fract(p);
    vec2 u = f * f * (3.0 - 2.0 * f);
    return mix(mix(hash21(i),              hash21(i + vec2(1, 0)), u.x),
               mix(hash21(i + vec2(0, 1)), hash21(i + vec2(1, 1)), u.x), u.y);
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

// NEU: ist diese Zelle belegt? Hash-Schwelle, moduliert von groben Clustern
bool belegt(vec3 id)
{
    // 2D-Noise mit y-Versatz als billiger 3D-Ersatz: dichte und leere Regionen
    float cluster = vnoise(id.xz * 0.23 + id.y * 0.31);
    float schwelle = DICHTE * 1.6 * smoothstep(0.25, 0.75, cluster);
    return hash13(id + 4.7) < schwelle;
}

// GEAENDERT: Ausduennung + Groesse je Zelle + Zellwand-Klammer
float map(vec3 p)
{
    vec3 id = floor(p / ZELLE);
    vec3 q  = mod(p, ZELLE) - 0.5 * ZELLE;

    // Zellwand-Klammer: Abstand zur naechsten Zellwand + MARGE ist eine
    // KONSERVATIVE untere Schranke fuer alles ausserhalb dieser Zelle
    float wand = ZELLE * 0.5 - max(abs(q.x), max(abs(q.y), abs(q.z)));
    float sicher = wand + MARGE;

    if (!belegt(id)) return sicher;

    float gr = GROESSE_MAX * (0.35 + 0.65 * hash13(id + 3.1));
    return min(length(q) - 0.9 * gr, sicher);
}

float marchDebris(vec3 ro, vec3 rd)
{
    float t = 0.0;
    for (int i = 0; i < 110; i++) {
        float d = map(ro + rd * t);
        if (d < 0.001 + 0.0012 * t) return t;
        if (t > 60.0) break;
        t += d;
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

    float t = marchDebris(ro, rd);

    vec3 color;
    if (t > 0.0) {
        vec3 p = ro + rd * t;
        vec3 n = calcNormal(p);
        float dif = max(dot(n, normalize(vec3(0.65, 0.28, -0.70))), 0.0);
        color = vec3(0.05) + dif * vec3(0.85);

        vec3 id = floor(p / ZELLE);
        color *= 0.4 + 0.6 * hash33(id);
    } else {
        color = vec3(0.008, 0.010, 0.018) + sterne(rd);
    }

    fragColor = vec4(color, 1.0);
}
