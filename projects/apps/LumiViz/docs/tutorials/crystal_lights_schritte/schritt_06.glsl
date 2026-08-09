// Schritt 6 - Kristall-Facetten: Voronoi-Platten
// Rekonstruiert aus CrystalLights-tutorial.md (SSOT dort; ab Schritt 6 nur Diffs).
#define R(a) mat2(cos(a), sin(a), -sin(a), cos(a))

float hash21(vec2 p) { return fract(sin(dot(p, vec2(127.1, 311.7))) * 43758.5453); }

// NEU: 2D-Hash mit 2D-Ergebnis (fuer die Voronoi-Punkte)
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

// NEU: Voronoi - liefert (Abstand^2, Zell-Id.x, Zell-Id.y) der naechsten Zelle
vec3 voronoi(vec2 p)
{
    vec2 i = floor(p), f = fract(p);
    float best = 8.0;
    vec2 bestId = vec2(0.0);
    for (int y = -1; y <= 1; y++)
    for (int x = -1; x <= 1; x++) {
        vec2 g = vec2(float(x), float(y));
        vec2 r = g + hash22(i + g) - f;        // Vektor zum Zufallspunkt der Nachbarzelle
        float d = dot(r, r);
        if (d < best) { best = d; bestId = i + g; }
    }
    return vec3(best, bestId);
}

// GEAENDERT: weiche Grundform + Platten-Versatz je Voronoi-Zelle
float terrainGlatt(vec2 p)
{
    return (fbm(p * 0.35) - 0.45) * 2.4;
}

float terrain(vec2 p)
{
    float h = terrainGlatt(p);
    vec3 vo = voronoi(p * 1.5);
    float platte = (hash21(vo.yz) - 0.5) * 0.55;   // jede Platte: -0.27 .. +0.27
    return h + platte;
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

vec3 himmelFarbe(vec3 rd)
{
    return mix(vec3(0.10, 0.12, 0.22), vec3(0.02, 0.03, 0.08),
               clamp(rd.y * 3.0, 0.0, 1.0));
}

void mainImage(out vec4 fragColor, in vec2 fragCoord)
{
    vec2 uv = (fragCoord - 0.5 * iResolution.xy) / iResolution.y;

    vec3 ro = vec3(0.0, 2.8, iTime * 0.8);
    vec3 rd = normalize(vec3(uv, 1.3));
    rd.yz *= R(-0.12);

    float t = marchTerrain(ro, rd);

    vec3 color;
    if (t > 0.0) {
        vec3 p = ro + rd * t;
        vec3 n = terrainNormal(p.xz, t);

        vec3 mond = normalize(vec3(0.4, 0.75, -0.5));
        float dif  = max(dot(n, mond), 0.0);
        float amb  = 0.5 + 0.5 * n.y;

        color  = dif * vec3(0.35, 0.42, 0.55);
        color += amb * vec3(0.08, 0.12, 0.18);
    } else {
        color = himmelFarbe(rd);
    }

    fragColor = vec4(color, 1.0);
}
