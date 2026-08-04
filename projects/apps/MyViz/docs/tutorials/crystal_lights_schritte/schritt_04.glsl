// Schritt 4 - FBM: aus Sinus wird Terrain
// Voll-Listing aus CrystalLights-tutorial.md (SSOT dort).
#define R(a) mat2(cos(a), sin(a), -sin(a), cos(a))

// Hash: Gitterpunkt -> deterministische "Zufallszahl" 0..1
float hash21(vec2 p) { return fract(sin(dot(p, vec2(127.1, 311.7))) * 43758.5453); }

// Value-Noise: weich interpolierte Zufallswerte auf einem Einheitsgitter
float vnoise(vec2 p)
{
    vec2 i = floor(p), f = fract(p);
    vec2 u = f * f * (3.0 - 2.0 * f);            // smoothstep-Kurve
    return mix(mix(hash21(i),                hash21(i + vec2(1, 0)), u.x),
               mix(hash21(i + vec2(0, 1)),   hash21(i + vec2(1, 1)), u.x), u.y);
}

// Fraktales Rauschen: mehrere Oktaven, jede doppelt so fein und halb so laut
float fbm(vec2 p)
{
    float v = 0.0, a = 0.5;
    for (int i = 0; i < 5; i++) {
        v += a * vnoise(p);
        p = p * 2.03 + 11.7;                      // feiner + versetzt (gegen Gittermuster)
        a *= 0.5;
    }
    return v;                                     // ~0..1
}

float terrain(vec2 p)
{
    return (fbm(p * 0.35) - 0.45) * 2.4;          // Huegel ca. -1.1 .. +1.3
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

void mainImage(out vec4 fragColor, in vec2 fragCoord)
{
    vec2 uv = (fragCoord - 0.5 * iResolution.xy) / iResolution.y;

    vec3 ro = vec3(0.0, 2.8, iTime * 0.8);
    vec3 rd = normalize(vec3(uv, 1.3));
    rd.yz *= R(-0.12);

    float t = marchTerrain(ro, rd);

    vec3 color;
    if (t > 0.0) {
        color = vec3(clamp(1.0 - t * 0.035, 0.0, 1.0));
    } else {
        color = mix(vec3(0.10, 0.12, 0.22), vec3(0.02, 0.03, 0.08),
                    clamp(rd.y * 3.0, 0.0, 1.0));
    }

    fragColor = vec4(color, 1.0);
}
