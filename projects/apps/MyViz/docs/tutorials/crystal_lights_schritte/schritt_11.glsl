// Schritt 11 - Glow und Sparkle: liquide Stellen leuchten
// Rekonstruiert aus CrystalLights-tutorial.md (SSOT dort; ab Schritt 6 nur Diffs).

// ---- STELLSCHRAUBEN (erweitert) --------------------------------------------
const float LUECKEN      = 0.30;  // 0.0 = geschlossene Decke .. 0.6 = Inselwelt
const float LUECKEN_DYN  = 0.15;  // dynamischer Anteil: 0.0 = statisch
const float TIEFE        = -1.8;  // y der Lichtebene (unter dem Terrain)
const float LAMPEN_ZELLE = 1.7;   // Rasterabstand der Leuchtkoerper
const float DICHTE       = 0.55;  // Absorption im Kristall: 0 = Glas .. 1.5 = Milcheis
const float GLOW         = 1.6;   // Streu-Glow an liquiden Stellen
// ----------------------------------------------------------------------------

#define R(a) mat2(cos(a), sin(a), -sin(a), cos(a))

float hash21(vec2 p) { return fract(sin(dot(p, vec2(127.1, 311.7))) * 43758.5453); }

vec2 hash22(vec2 p)
{
    return fract(sin(vec2(dot(p, vec2(127.1, 311.7)),
                          dot(p, vec2(269.5, 183.3)))) * 43758.5453);
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

vec3 voronoi(vec2 p)
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

float terrainGlatt(vec2 p)
{
    return (fbm(p * 0.35) - 0.45) * 2.4;
}

float liquid(vec2 p)
{
    return smoothstep(0.35, 0.75, vnoise(p * 0.22 + vec2(0.0, iTime * 0.06)));
}

float gapMask(vec2 p)
{
    float schwelle = LUECKEN + LUECKEN_DYN * (0.5 + 0.5 * sin(iTime * 0.09));
    float m = vnoise(p * 0.30 + 41.0);
    return smoothstep(schwelle - 0.10, schwelle + 0.10, m);
}

float terrain(vec2 p)
{
    float glatt = terrainGlatt(p);

    vec3 vo = voronoi(p * 1.5);
    float kristall = glatt + (hash21(vo.yz) - 0.5) * 0.55;

    float welle = (vnoise(p * 1.4 + vec2(iTime * 0.25, 0.0)) - 0.5) * 0.12;
    float fluessig = glatt + welle;

    float h = mix(kristall, fluessig, liquid(p));

    return mix(-6.0, h, gapMask(p));
}

float marchTerrain(vec3 ro, vec3 rd)
{
    float t = 0.0;
    for (int i = 0; i < 150; i++) {
        vec3 p = ro + rd * t;
        float d = p.y - terrain(p.xz);
        if (d < 0.001 + 0.0015 * t) return t;
        if (t > 45.0) break;
        t += d * 0.4;
    }
    return -1.0;
}

vec3 terrainNormal(vec2 p, float t)
{
    vec2 e = vec2(0.012 * (1.0 + t * 0.12), 0.0);
    return normalize(vec3(terrain(p - e.xy) - terrain(p + e.xy),
                          2.0 * e.x,
                          terrain(p - e.yx) - terrain(p + e.yx)));
}

vec3 lampColor(float t)
{
    return 0.55 + 0.45 * cos(6.28318 * (t + vec3(0.0, 0.33, 0.67)));
}

float blink(vec2 id)
{
    float ph = hash21(id + 31.7);
    float sp = 0.35 + 0.75 * hash21(id + 17.3);
    float w  = 0.5 + 0.5 * sin(6.28318 * (iTime * sp * 0.25 + ph));
    return smoothstep(0.70, 0.97, w) * (0.2 + 0.8 * hash21(id + 5.1));
}

vec3 lightsAt(vec2 q, float weich)
{
    vec2 base = floor(q / LAMPEN_ZELLE);
    vec3 acc = vec3(0.0);
    for (int y = -1; y <= 1; y++)
    for (int x = -1; x <= 1; x++) {
        vec2 id = base + vec2(float(x), float(y));
        vec2 c  = (id + 0.5 + 0.7 * (hash22(id + 7.0) - 0.5)) * LAMPEN_ZELLE;
        vec2 d  = q - c;
        float hell = blink(id) + 0.05;
        acc += lampColor(hash21(id)) * hell / (0.02 + dot(d, d) * 14.0 / weich);
    }
    return acc * 0.05;
}

vec3 himmelFarbe(vec3 rd)
{
    return mix(vec3(0.10, 0.12, 0.22), vec3(0.02, 0.03, 0.08),
               clamp(rd.y * 3.0, 0.0, 1.0));
}

// GEAENDERT: shadeKristall bekommt zwei neue Terme ((3b) Glow, (3c) Sparkle)
vec3 shadeKristall(vec3 p, vec3 rd, float t)
{
    vec3 n  = terrainNormal(p.xz, t);
    float L = liquid(p.xz);

    vec3 rr = refract(rd, n, 1.0 / 1.45);
    if (dot(rr, rr) < 0.5) rr = rd;

    float tt = (TIEFE - p.y) / min(rr.y, -0.05);
    vec2 q = (p + rr * tt).xz;

    float dicke = max(p.y - TIEFE, 0.0);
    vec3 T = exp(-dicke * vec3(0.85, 0.30, 0.16) * DICHTE);

    vec3 col = lightsAt(q, 1.0) * T;

    // (3b) GLOW: an liquiden Stellen streut das Material das Licht -
    //      dieselben Lampen, aber mit weitem Streukegel, gewichtet mit L
    col += lightsAt(q, 1.0 + L * 10.0) * T * L * GLOW;

    // (3c) SPARKLE: harte Glanzblitze des Monds - NUR auf kristallinen Facetten
    vec3 mond = normalize(vec3(0.4, 0.75, -0.5));
    float spec = pow(max(dot(reflect(rd, n), mond), 0.0), 60.0);
    col += spec * (1.0 - L) * vec3(0.9, 0.95, 1.0) * 0.6;

    // (4) OBERFLAECHE: Fresnel-Spiegelung des kalten Himmels ...
    float fres = pow(1.0 - max(dot(n, -rd), 0.0), 3.0);
    col += fres * vec3(0.35, 0.50, 0.65) * 0.5;

    //     ... und ein Hauch Mond (mond wird aus (3c) wiederverwendet)
    float dif = max(dot(n, mond), 0.0);
    col += dif * vec3(0.10, 0.14, 0.20);

    return col;
}

void mainImage(out vec4 fragColor, in vec2 fragCoord)
{
    vec2 uv = (fragCoord - 0.5 * iResolution.xy) / iResolution.y;

    vec3 ro = vec3(0.0, 2.8, iTime * 0.8);
    vec3 rd = normalize(vec3(uv, 1.3));
    rd.yz *= R(-0.12);

    float t = marchTerrain(ro, rd);

    float tP = (rd.y < -0.001) ? (TIEFE - ro.y) / rd.y : 1e5;

    vec3 color;
    if (tP < 1e4 && (t < 0.0 || tP < t)) {
        vec2 q = (ro + rd * tP).xz;
        color = vec3(0.010, 0.015, 0.030) + lightsAt(q, 1.0);
    } else if (t > 0.0) {
        vec3 p = ro + rd * t;
        color = shadeKristall(p, rd, t);
    } else {
        color = himmelFarbe(rd);
    }

    fragColor = vec4(color, 1.0);
}
