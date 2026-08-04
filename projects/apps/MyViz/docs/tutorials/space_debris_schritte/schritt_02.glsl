float hash21(vec2 p) { return fract(sin(dot(p, vec2(127.1, 311.7))) * 43758.5453); }

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

// Die Szene: vorerst genau EINE Kugel im Ursprung
float map(vec3 p)
{
    return length(p) - 1.0;
}

// Klassischer SDF-Marsch (vgl. Pyramid-Spiral-Tutorial, Schritte 3-5)
float marchDebris(vec3 ro, vec3 rd)
{
    float t = 0.0;
    for (int i = 0; i < 110; i++) {
        float d = map(ro + rd * t);
        if (d < 0.001 + 0.0012 * t) return t;   // Trefftoleranz waechst mit der Ferne
        if (t > 60.0) break;
        t += d;                                  // volles Tempo - noch ist die SDF exakt
    }
    return -1.0;
}

// Normale aus dem Gradienten - Tetraeder-Variante: nur 4 map()-Aufrufe statt 6
vec3 calcNormal(vec3 p)
{
    const vec2 e = vec2(0.0012, -0.0012);
    return normalize(e.xyy * map(p + e.xyy) + e.yyx * map(p + e.yyx) +
                     e.yxy * map(p + e.yxy) + e.xxx * map(p + e.xxx));
}

void mainImage(out vec4 fragColor, in vec2 fragCoord)
{
    vec2 uv = (fragCoord - 0.5 * iResolution.xy) / iResolution.y;

    vec3 ro = vec3(0.0, 0.0, -4.5);
    vec3 rd = normalize(vec3(uv, 1.4));

    float t = marchDebris(ro, rd);

    vec3 color;
    if (t > 0.0) {
        vec3 p = ro + rd * t;
        vec3 n = calcNormal(p);
        // Neutrales Testlicht - das echte Material kommt in Schritt 9
        float dif = max(dot(n, normalize(vec3(0.65, 0.28, -0.70))), 0.0);
        color = vec3(0.05) + dif * vec3(0.85);
    } else {
        color = vec3(0.008, 0.010, 0.018) + sterne(rd);
    }

    fragColor = vec4(color, 1.0);
}
