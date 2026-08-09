#define R(a) mat2(cos(a), sin(a), -sin(a), cos(a))

// ---- STELLSCHRAUBEN --------------------------------------------------------
const float RADIUS  = 6.0;
const float NAH     = 8.5;
const float ZELLE1  = 2.6;
const float ZELLE2  = 0.9;
const float ZELLE3  = 0.32;
const float PLATTE  = 0.35;
const float AUFBAU  = 0.22;
const float FUGE    = 0.07;
const float TIEFE   = 0.30;
const float RILLE   = 0.02;
const float GLAETTE = 0.05;
const float DROSSEL = 0.5;
// ----------------------------------------------------------------------------

vec3 gSonne = vec3(0.0, 0.3, 1.0);

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

float march(vec3 ro, vec3 rd)
{
    float t = 0.0;
    for (int i = 0; i < 160; i++) {
        vec3 p = ro + rd * t;
        float d = map(p);
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

// GEAENDERT: brighter - STREIFLICHT von rechts oben, quer zum Blick
void kamera(vec2 uv, out vec3 ro, out vec3 rd)
{
    ro = vec3(0.0, -3.2, NAH);
    vec3 ta = vec3(0.0, 1.2, 0.0);

    vec3 fw = normalize(ta - ro);
    vec3 rt = normalize(cross(vec3(0.0, 1.0, 0.0), fw));
    vec3 up = cross(fw, rt);

    rd = normalize(fw * 1.1 + rt * uv.x + up * uv.y);

    gSonne = normalize(rt * 1.3 + vec3(0.0, 0.55, 0.0) - fw * 0.10);
}

// GEAENDERT: hellerer, waermerer Himmel + kraeftiger Sonnen-Hof
vec3 himmel(vec3 rd)
{
    vec3 col = mix(vec3(0.24, 0.20, 0.18), vec3(0.10, 0.13, 0.20),
                   clamp(rd.y * 1.5 + 0.5, 0.0, 1.0));

    float s = max(dot(rd, gSonne), 0.0);
    col += pow(s, 6.0) * vec3(0.50, 0.38, 0.22);

    return col;
}

// GEAENDERT: das brighter-Material
vec3 shade(vec3 p, vec3 rd, float t)
{
    vec3 n = calcNormal(p);

    vec3 sonnenFarbe = vec3(1.05, 0.80, 0.55);   // warmes, kraeftiges Streiflicht
    vec3 himmelLicht = vec3(0.10, 0.12, 0.16);   // kuehles Fuell-Licht von oben

    vec3 albedo = vec3(0.16, 0.17, 0.19);

    float dif = max(dot(n, gSonne), 0.0);
    float amb = 0.5 + 0.5 * n.y;

    vec3 col = albedo * (dif * sonnenFarbe * 1.0 + amb * himmelLicht);

    // Rim bleibt, aber leise - nur noch Kantentrenner, kein Hauptdarsteller
    float rim = pow(1.0 - max(dot(n, -rd), 0.0), 3.0);
    col += rim * sonnenFarbe * 0.22;

    // NEU: Glanzlicht - die Rillen und Fasen antworten der Sonne
    float spe = pow(max(dot(reflect(rd, n), gSonne), 0.0), 24.0);
    col += spe * sonnenFarbe * 0.30;

    return col;
}

void mainImage(out vec4 fragColor, in vec2 fragCoord)
{
    vec2 uv = (fragCoord - 0.5 * iResolution.xy) / iResolution.y;

    vec3 ro, rd;
    kamera(uv, ro, rd);

    float t = march(ro, rd);

    vec3 col;
    if (t > 0.0) col = shade(ro + rd * t, rd, t);
    else         col = himmel(rd);

    fragColor = vec4(col, 1.0);
}
