#define R(a) mat2(cos(a), sin(a), -sin(a), cos(a))

const float RADIUS = 6.0;
const float NAH    = 8.5;    // Abstand der Kamera von der Achse des Molochs

float map(vec3 p)
{
    return length(p) - RADIUS;
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

// NEU: Look-at-Kamera - Position tief unten, Blick hinauf zum Moloch
void kamera(vec2 uv, out vec3 ro, out vec3 rd)
{
    ro = vec3(0.0, -3.2, NAH);                 // tief unten: Low-Angle
    vec3 ta = vec3(0.0, 1.2, 0.0);             // Blickpunkt am oberen Rumpf

    vec3 fw = normalize(ta - ro);              // Blickrichtung
    vec3 rt = normalize(cross(vec3(0.0, 1.0, 0.0), fw));
    vec3 up = cross(fw, rt);

    rd = normalize(fw * 1.1 + rt * uv.x + up * uv.y);   // 1.1 = Weitwinkel
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
