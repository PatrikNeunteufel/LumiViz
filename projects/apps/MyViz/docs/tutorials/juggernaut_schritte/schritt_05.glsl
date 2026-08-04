#define R(a) mat2(cos(a), sin(a), -sin(a), cos(a))

// ---- STELLSCHRAUBEN (erweitert) --------------------------------------------
const float RADIUS  = 6.0;
const float NAH     = 8.5;
const float ZELLE1  = 2.6;    // grosse Platten
const float ZELLE2  = 0.9;    // Raster der mittleren Aufbauten
const float ZELLE3  = 0.32;   // Raster der feinen Rillen
const float PLATTE  = 0.35;
const float AUFBAU  = 0.22;   // Hoehe der mittleren Aufbauten
const float FUGE    = 0.07;
const float TIEFE   = 0.30;
const float RILLE   = 0.02;   // Tiefe der feinen Rillen
const float GLAETTE = 0.05;
const float DROSSEL = 0.5;    // Marsch-Drossel (Displacement -> nur noch Bound!)
// ----------------------------------------------------------------------------

float hash31(vec3 p) { return fract(sin(dot(p, vec3(127.1, 311.7, 74.7))) * 43758.5453); }

float smin(float a, float b, float k)
{
    float h = max(k - abs(a - b), 0.0) / k;
    return min(a, b) - h * h * k * 0.25;
}
float smax(float a, float b, float k) { return -smin(-a, -b, k); }

float fugen(vec3 p, float zelle)
{
    vec3 q = abs(fract(p / zelle) - 0.5) * zelle;
    return zelle * 0.5 - max(q.x, max(q.y, q.z));
}

// GEAENDERT: drei Detail-Oktaven auf der Kugel
float map(vec3 p)
{
    float d = length(p) - RADIUS;

    // Oktave 1: grosse Platten
    vec3 z1 = floor(p / ZELLE1);
    d -= (hash31(z1) - 0.5) * PLATTE;

    // Fugen der grossen Platten
    float slab   = fugen(p, ZELLE1) - FUGE;
    float schale = (RADIUS - TIEFE) - length(p);
    d = smax(d, -max(slab, schale), GLAETTE);

    // Oktave 2: mittlere Aufbauten - manche Zellen stehen als Bloecke vor
    vec3 z2 = floor(p / ZELLE2);
    d -= step(0.72, hash31(z2 + 7.0)) * AUFBAU * (0.35 + 0.65 * hash31(z2 + 13.0));

    // Oktave 3: feine Rillen - das h1-Idiom des Warp-Shaders als Displacement
    vec3 h = pow(abs(2.0 * fract(p / ZELLE3) - 1.0), vec3(3.0));
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
