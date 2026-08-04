#define R(a) mat2(cos(a), sin(a), -sin(a), cos(a))

// ---- STELLSCHRAUBEN (erweitert) --------------------------------------------
const float STIMMUNG = 0.0;   // 0.0 = dark .. 1.0 = brighter
const float RADIUS   = 6.0;
const float NAH      = 8.5;
const float ZELLE1   = 2.6;
const float ZELLE2   = 0.9;
const float ZELLE3   = 0.32;
const float PLATTE   = 0.35;
const float AUFBAU   = 0.22;
const float FUGE     = 0.07;
const float TIEFE    = 0.30;
const float RILLE    = 0.02;
const float GLAETTE  = 0.05;
const float DROSSEL  = 0.5;
const float GODRAY   = 1.0;   // Staerke des volumetrischen Glows
// -----------------------------------------------------------------------------

float gStimmung = STIMMUNG;
vec3  gSonne    = vec3(0.0, 0.3, 1.0);

float hash31(vec3 p) { return fract(sin(dot(p, vec3(127.1, 311.7, 74.7))) * 43758.5453); }

float smin(float a, float b, float k)
{
    float h = max(k - abs(a - b), 0.0) / k;
    return min(a, b) - h * h * k * 0.25;
}
float smax(float a, float b, float k) { return -smin(-a, -b, k); }

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
    float schale = (RADIUS - TIEFE) - length(q);
    d = smax(d, -max(slab, schale), GLAETTE);

    vec3 z2 = floor(q / ZELLE2);
    d -= step(0.72, hash31(z2 + 7.0)) * AUFBAU * (0.35 + 0.65 * hash31(z2 + 13.0));

    vec3 h = pow(abs(2.0 * fract(q / ZELLE3) - 1.0), vec3(3.0));
    d += RILLE * (h.x + h.y + h.z) * 0.33;

    return d;
}

// GEAENDERT: march() sammelt nebenbei Glow ein
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
    vec3 oben  = mix(vec3(0.010, 0.012, 0.022), vec3(0.10, 0.13, 0.20), gStimmung);
    vec3 unten = mix(vec3(0.030, 0.028, 0.045), vec3(0.24, 0.20, 0.18), gStimmung);
    vec3 col = mix(unten, oben, clamp(rd.y * 1.5 + 0.5, 0.0, 1.0));

    float s = max(dot(rd, gSonne), 0.0);
    vec3 hofFarbe = mix(vec3(0.10, 0.13, 0.22), vec3(0.50, 0.38, 0.22), gStimmung);
    col += pow(s, 6.0) * hofFarbe;

    return col;
}

vec3 shade(vec3 p, vec3 rd, float t)
{
    vec3 n = calcNormal(p);

    vec3 sonnenFarbe = mix(vec3(0.30, 0.38, 0.55), vec3(1.05, 0.80, 0.55), gStimmung);
    vec3 himmelLicht = mix(vec3(0.020, 0.025, 0.045), vec3(0.10, 0.12, 0.16), gStimmung);
    float difStaerke = mix(0.6, 1.0, gStimmung);
    float rimStaerke = mix(0.55, 0.22, gStimmung);
    float speStaerke = mix(0.06, 0.30, gStimmung);

    vec3 albedo = vec3(0.16, 0.17, 0.19);

    float dif = max(dot(n, gSonne), 0.0);
    float amb = 0.5 + 0.5 * n.y;

    vec3 col = albedo * (dif * sonnenFarbe * difStaerke + amb * himmelLicht);

    float rim = pow(1.0 - max(dot(n, -rd), 0.0), 3.0);
    col += rim * sonnenFarbe * rimStaerke;

    float spe = pow(max(dot(reflect(rd, n), gSonne), 0.0), 24.0);
    col += spe * sonnenFarbe * speStaerke;

    vec3 lichtFarbe = mix(vec3(1.0, 0.12, 0.08), vec3(1.0, 0.75, 0.45), gStimmung);
    col += fenster(gedreht(p)) * lichtFarbe * mix(1.4, 0.8, gStimmung);

    return col;
}

void kamera(vec2 uv, out vec3 ro, out vec3 rd)
{
    ro = vec3(0.0, -3.2, NAH);
    vec3 ta = vec3(0.0, 1.2, 0.0);

    vec3 fw = normalize(ta - ro);
    vec3 rt = normalize(cross(vec3(0.0, 1.0, 0.0), fw));
    vec3 up = cross(fw, rt);

    rd = normalize(fw * 1.1 + rt * uv.x + up * uv.y);

    gSonne = normalize(mix(fw + vec3(0.0, 0.35, 0.0),
                           rt * 1.3 + vec3(0.0, 0.55, 0.0) - fw * 0.10,
                           gStimmung));
}

void mainImage(out vec4 fragColor, in vec2 fragCoord)
{
    vec2 uv = (fragCoord - 0.5 * iResolution.xy) / iResolution.y;

    gStimmung = STIMMUNG;

    vec3 ro, rd;
    kamera(uv, ro, rd);

    float glow;
    float t = march(ro, rd, glow);

    vec3 col;
    if (t > 0.0) col = shade(ro + rd * t, rd, t);
    else         col = himmel(rd);

    // God-Rays (Teil 1): der beim Marsch gesammelte Glow um die Silhouette
    vec3 strahlFarbe = mix(vec3(0.28, 0.34, 0.55), vec3(1.0, 0.75, 0.50), gStimmung);
    col += glow * 0.06 * GODRAY * strahlFarbe;

    fragColor = vec4(col, 1.0);
}
