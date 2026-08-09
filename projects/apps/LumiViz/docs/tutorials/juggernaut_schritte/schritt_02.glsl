#define R(a) mat2(cos(a), sin(a), -sin(a), cos(a))

const float RADIUS = 6.0;    // Radius des Molochs

float map(vec3 p)
{
    return length(p) - RADIUS;   // vorerst: die nackte Riesenkugel
}

float march(vec3 ro, vec3 rd)
{
    float t = 0.0;
    for (int i = 0; i < 160; i++) {
        vec3 p = ro + rd * t;
        float d = map(p);
        if (d < 0.001 + 0.0008 * t) return t;   // Toleranz waechst mit der Ferne
        if (t > 40.0) break;
        t += d * 0.5;                            // Drossel - Begruendung: Schritt 5
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

void mainImage(out vec4 fragColor, in vec2 fragCoord)
{
    vec2 uv = (fragCoord - 0.5 * iResolution.xy) / iResolution.y;

    vec3 ro = vec3(0.0, 0.0, 10.0);          // 10 Einheiten vor der Kugel
    vec3 rd = normalize(vec3(uv, -1.5));     // Blick in -z, Brennweite 1.5

    float t = march(ro, rd);

    vec3 col;
    if (t > 0.0) {
        vec3 n = calcNormal(ro + rd * t);
        float dif = max(dot(n, normalize(vec3(0.5, 0.7, 0.4))), 0.0);
        col = vec3(0.16, 0.17, 0.19) * (dif + 0.15);   // neutrales Werkstatt-Licht
    } else {
        col = mix(vec3(0.030, 0.028, 0.045), vec3(0.010, 0.012, 0.022),
                  clamp(rd.y * 1.5 + 0.5, 0.0, 1.0));
    }

    fragColor = vec4(col, 1.0);
}
