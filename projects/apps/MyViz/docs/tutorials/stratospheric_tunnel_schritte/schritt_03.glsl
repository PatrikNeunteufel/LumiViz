#define R(a) mat2(cos(a), sin(a), -sin(a), cos(a))

const float RADIUS = 1.0;
const float TAU    = 6.28318530;

float mapTunnel(vec3 p)
{
    return RADIUS - length(p.xy);
}

float march(vec3 ro, vec3 rd)
{
    float t = 0.02;
    for (int i = 0; i < 120; i++) {
        float d = mapTunnel(ro + rd * t);
        if (d < 0.0015 + 0.001 * t) break;
        t += d;
        if (t > 60.0) break;
    }
    return min(t, 60.0);
}

void mainImage(out vec4 fragColor, in vec2 fragCoord)
{
    vec2 uv = (fragCoord - 0.5 * iResolution.xy) / iResolution.y;

    vec3 ro = vec3(0.0, 0.0, iTime * 2.0);
    vec3 rd = normalize(vec3(uv, 1.4));

    float t = march(ro, rd);
    vec3 p = ro + rd * t;

    float w  = atan(p.y, p.x);        // Winkel um die Achse: -pi..pi
    float wu = w / TAU + 0.5;         // 0..1 einmal um den Umfang

    // Schachbrett auf der abgerollten Wandflaeche (Winkel, z)
    float schach = mod(floor(wu * 14.0) + floor(p.z * 1.5), 2.0);
    vec3 basis = mix(vec3(0.05, 0.06, 0.10), vec3(0.16, 0.19, 0.27), schach);

    // Normale des nackten Zylinders: zeigt von der Wand zur Achse
    vec3 n = normalize(vec3(-p.xy, 0.0));

    // SCHEINWERFER: Punktlicht an der Kamera, 1/d^2-artiger Abfall
    vec3 zk = ro - p;                        // vom Wandpunkt zur Kamera
    float dk = max(length(zk), 1e-3);
    float dif = max(dot(n, zk / dk), 0.0);
    vec3 color = basis * (1.6 * dif / (1.0 + dk * dk * 0.12) + 0.02);

    fragColor = vec4(color, 1.0);
}
