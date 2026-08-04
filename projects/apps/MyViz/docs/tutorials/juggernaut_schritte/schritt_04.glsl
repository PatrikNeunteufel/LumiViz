#define R(a) mat2(cos(a), sin(a), -sin(a), cos(a))

// ---- STELLSCHRAUBEN --------------------------------------------------------
const float RADIUS  = 6.0;    // Radius des Molochs
const float NAH     = 8.5;    // Kameraabstand von der Achse
const float ZELLE1  = 2.6;    // Kantenlaenge der grossen Platten
const float PLATTE  = 0.35;   // Hoehenspiel der Platten (+- die Haelfte)
const float FUGE    = 0.07;   // halbe Breite der Panelfugen
const float TIEFE   = 0.30;   // Fugen-Schale: so tief unter den NENN-Radius
const float GLAETTE = 0.05;   // Kanten-Weiche der smax-Fugen
// ----------------------------------------------------------------------------

// NEU: 3D-Hash - eine deterministische Zufallszahl je Gitterzelle
float hash31(vec3 p) { return fract(sin(dot(p, vec3(127.1, 311.7, 74.7))) * 43758.5453); }

// NEU: weiche Boolesche Operatoren (polynomiale Form)
float smin(float a, float b, float k)
{
    float h = max(k - abs(a - b), 0.0) / k;
    return min(a, b) - h * h * k * 0.25;
}
float smax(float a, float b, float k) { return -smin(-a, -b, k); }

// NEU: Abstand zur naechsten Gitterebene einer kubischen Zellteilung
float fugen(vec3 p, float zelle)
{
    vec3 q = abs(fract(p / zelle) - 0.5) * zelle;   // Abstand zur Zellmitte je Achse
    return zelle * 0.5 - max(q.x, max(q.y, q.z));   // -> Abstand zur Zellgrenze
}

// GEAENDERT: Kugel + Platten-Versatz + Fugen-Schnitt
float map(vec3 p)
{
    // Basis: die Riesenkugel
    float d = length(p) - RADIUS;

    // Oktave 1: grosse Platten - jede Wuerfelzelle hat ihren eigenen Radius
    vec3 z1 = floor(p / ZELLE1);
    d -= (hash31(z1) - 0.5) * PLATTE;

    // Fugen: Gitterebenen-Slab, begrenzt auf die aeussere Schale, abgezogen
    float slab   = fugen(p, ZELLE1) - FUGE;        // < 0 nahe einer Zellgrenze
    float schale = (RADIUS - TIEFE) - length(p);   // < 0 in der aeusseren Schale
    d = smax(d, -max(slab, schale), GLAETTE);      // Schnittvolumen ABZIEHEN

    return d;
}

float march(vec3 ro, vec3 rd)
{
    float t = 0.0;
    for (int i = 0; i < 160; i++) {
        vec3 p = ro + rd * t;
        float d = map(p);
        if (d < 0.001 + 0.0008 * t) return t;
        if (t > 40.0) break;
        t += d * 0.5;
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

void kamera(vec2 uv, out vec3 ro, out vec3 rd)
{
    ro = vec3(0.0, -3.2, NAH);
    vec3 ta = vec3(0.0, 1.2, 0.0);

    vec3 fw = normalize(ta - ro);
    vec3 rt = normalize(cross(vec3(0.0, 1.0, 0.0), fw));
    vec3 up = cross(fw, rt);

    rd = normalize(fw * 1.1 + rt * uv.x + up * uv.y);
}

void mainImage(out vec4 fragColor, in vec2 fragCoord)
{
    vec2 uv = (fragCoord - 0.5 * iResolution.xy) / iResolution.y;

    vec3 ro, rd;
    kamera(uv, ro, rd);

    float t = march(ro, rd);

    vec3 col;
    if (t > 0.0) {
        vec3 n = calcNormal(ro + rd * t);
        float dif = max(dot(n, normalize(vec3(0.5, 0.7, 0.4))), 0.0);
        col = vec3(0.16, 0.17, 0.19) * (dif + 0.15);
    } else {
        col = mix(vec3(0.030, 0.028, 0.045), vec3(0.010, 0.012, 0.022),
                  clamp(rd.y * 1.5 + 0.5, 0.0, 1.0));
    }

    fragColor = vec4(col, 1.0);
}
