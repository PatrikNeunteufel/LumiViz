// ============================================================================
// COMPOSITE: TRANSITIONS - Schritt 2: Welt B als Skelett (Juggernaut, dark)
// Vollausbau und Herleitung: Juggernaut-Shader-Tutorial (gleicher Ordner)
// ============================================================================

#define R(a) mat2(cos(a), sin(a), -sin(a), cos(a))

float hash31(vec3 p) { return fract(sin(dot(p, vec3(127.1, 311.7, 74.7))) * 43758.5453); }

float smin(float a, float b, float k)
{
    float h = max(k - abs(a - b), 0.0) / k;
    return min(a, b) - h * h * k * 0.25;
}
float smax(float a, float b, float k) { return -smin(-a, -b, k); }

// ---- WELT B: Juggernaut (Skelett, dark) ------------------------------------

const float B_RADIUS = 6.0;    // Radius des Molochs
const float B_ZELLE1 = 2.6;    // grosse Platten
const float B_ZELLE2 = 0.9;    // Raster der Positionslichter
const float B_ZELLE3 = 0.32;   // feine Rillen

vec3 b_sonne = vec3(0.0, 0.3, 1.0);   // setzt die Kamera je Frame (Gegenlicht)

vec3 b_gedreht(vec3 p)
{
    p.yz *= R(0.42);                   // schiefe Achse
    p.xz *= R(iTime * 0.02);           // traege Eigendrehung
    return p;
}

float b_fugen(vec3 p, float zelle)
{
    vec3 q = abs(fract(p / zelle) - 0.5) * zelle;
    return zelle * 0.5 - max(q.x, max(q.y, q.z));
}

float b_map(vec3 p)
{
    vec3 q = b_gedreht(p);
    float d = length(q) - B_RADIUS;

    vec3 z1 = floor(q / B_ZELLE1);                    // Oktave 1: grosse Platten
    d -= (hash31(z1) - 0.5) * 0.35;

    float slab   = b_fugen(q, B_ZELLE1) - 0.07;       // Panelfugen, weich abgezogen
    float schale = (B_RADIUS - 0.30) - length(q);
    d = smax(d, -max(slab, schale), 0.05);

    vec3 h = pow(abs(2.0 * fract(q / B_ZELLE3) - 1.0), vec3(3.0));
    d += 0.02 * (h.x + h.y + h.z) * 0.33;             // Oktave 3: feine Rillen

    return d;
}

float b_march(vec3 ro, vec3 rd, out float glow)
{
    glow = 0.0;
    float t = 0.0;
    for (int i = 0; i < 110; i++) {           // Skelett: 110 statt 160 Schritte
        vec3 p = ro + rd * t;
        float d = b_map(p);
        glow += 0.012 / (0.05 + d * d);       // God-Ray-Saat: Naehe zum Moloch
        if (d < 0.001 + 0.001 * t) return t;
        if (t > 40.0) break;
        t += d * 0.5;                          // Displacement -> Drossel 0.5
    }
    return -1.0;
}

vec3 b_normal(vec3 p)
{
    const vec2 e = vec2(0.004, 0.0);
    return normalize(vec3(b_map(p + e.xyy) - b_map(p - e.xyy),
                          b_map(p + e.yxy) - b_map(p - e.yxy),
                          b_map(p + e.yyx) - b_map(p - e.yyx)));
}

float b_fenster(vec3 q)
{
    vec3 z = floor(q / B_ZELLE2);
    float an = step(0.93, hash31(z + 29.0));
    float sp = 0.10 + 0.25 * hash31(z + 3.0);
    float w  = 0.5 + 0.5 * sin(6.28318 * (iTime * sp + hash31(z + 11.0)));
    float blink = 0.25 + 0.75 * smoothstep(0.55, 0.95, w);
    vec3 lokal = (fract(q / B_ZELLE2) - 0.5) * B_ZELLE2;
    float punkt = 1.0 - smoothstep(0.06, 0.24, length(lokal));
    return an * blink * punkt;
}

vec3 b_himmel(vec3 rd)
{
    vec3 col = mix(vec3(0.030, 0.028, 0.045), vec3(0.010, 0.012, 0.022),
                   clamp(rd.y * 1.5 + 0.5, 0.0, 1.0));
    float s = max(dot(rd, b_sonne), 0.0);
    col += pow(s, 30.0) * vec3(0.35, 0.42, 0.60) * 1.2;   // fahle Streu-Sonne
    col += pow(s, 5.0)  * vec3(0.35, 0.42, 0.60) * 0.12;
    return col;
}

vec3 b_shade(vec3 p, vec3 rd)
{
    vec3 n = b_normal(p);
    vec3 albedo = vec3(0.16, 0.17, 0.19);        // dunkles, mattes Metall

    float dif = max(dot(n, b_sonne), 0.0);
    float amb = 0.5 + 0.5 * n.y;
    vec3 col = albedo * (dif * vec3(0.30, 0.38, 0.55) * 0.6
                       + amb * vec3(0.020, 0.025, 0.045));

    float rim = pow(1.0 - max(dot(n, -rd), 0.0), 3.0);
    col += rim * vec3(0.30, 0.38, 0.55) * 0.55;  // Silhouetten-Saum (traegt dark)

    col += b_fenster(b_gedreht(p)) * vec3(1.0, 0.12, 0.08) * 1.4;  // rote Lichter

    return col;
}

// liefert LINEARE Farbe - gleiche Schnittstelle wie a_render
vec3 b_render(vec3 ro, vec3 rd)
{
    float glow;
    float t = b_march(ro, rd, glow);
    vec3 col;
    if (t > 0.0) {
        col = b_shade(ro + rd * t, rd);
        col = mix(col, vec3(0.020, 0.024, 0.040),     // dichter dark-Dunst
                  1.0 - exp(-0.0035 * t * t));
    } else {
        col = b_himmel(rd);
    }
    col += glow * 0.05 * vec3(0.28, 0.34, 0.55);      // God-Ray-Glow
    return col;
}

void b_kamera(vec2 uv, out vec3 ro, out vec3 rd)
{
    float zt = iTime;
    float wink   = 2.6 * sin(zt * 0.021);                       // Orbit + Umkehr
    float radius = mix(8.5, 13.0, 0.5 + 0.5 * sin(zt * 0.013));
    float hoehe  = mix(-3.2, 0.6, 0.5 + 0.5 * sin(zt * 0.017));

    ro = vec3(sin(wink) * radius, hoehe, cos(wink) * radius);
    vec3 ta = vec3(0.0, 1.2, 0.0);

    vec3 fw = normalize(ta - ro);
    vec3 rt = normalize(cross(vec3(0.0, 1.0, 0.0), fw));
    vec3 up = cross(fw, rt);
    rd = normalize(fw * 1.1 + rt * uv.x + up * uv.y);           // Weitwinkel

    b_sonne = normalize(fw + vec3(0.0, 0.35, 0.0));  // Sonne HINTER dem Orb
}

void mainImage(out vec4 fragColor, in vec2 fragCoord)
{
    vec2 uv = (fragCoord - 0.5 * iResolution.xy) / iResolution.y;

    vec3 ro, rd;
    b_kamera(uv, ro, rd);
    vec3 col = b_render(ro, rd);

    col = 1.0 - exp(-col * 2.3);       // dark braucht mehr Belichtung (s. Original)
    col = pow(col, vec3(1.0 / 2.2));
    col *= 1.0 - 0.33 * dot(uv, uv);

    fragColor = vec4(col, 1.0);
}
