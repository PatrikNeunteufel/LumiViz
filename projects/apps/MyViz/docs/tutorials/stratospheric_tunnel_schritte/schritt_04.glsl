#define R(a) mat2(cos(a), sin(a), -sin(a), cos(a))

// ---- STELLSCHRAUBEN --------------------------------------------------------
const float RADIUS        = 1.0;    // Grundradius des Tunnels
const float ROEHREN       = 14.0;   // Roehren um den Umfang (ganzzahlig!)
const float ROEHREN_TIEFE = 0.10;   // Woelbung der Roehren
const float SPANT_ABSTAND = 4.0;    // Abstand der Spanten-Ringe (z)
const float SPANT_TIEFE   = 0.05;   // Hoehe der Spanten
// ----------------------------------------------------------------------------

const float TAU = 6.28318530;

// Relief: wie weit die Wand am Ort (w, z) in den Tunnel hineinragt
float wandRelief(float w, float z)
{
    // Roehren: weiches cos-Profil, Fugen bei w*ROEHREN = 2*pi*k
    float roehre = ROEHREN_TIEFE * (0.5 - 0.5 * cos(w * ROEHREN));

    // Spanten: schmale erhabene Ringe in festen z-Intervallen
    float sz    = abs(fract(z / SPANT_ABSTAND) - 0.5) * SPANT_ABSTAND;
    float spant = SPANT_TIEFE * smoothstep(0.35, 0.0, sz);

    return roehre + spant;
}

float mapTunnel(vec3 p)
{
    float w = atan(p.y, p.x);
    return RADIUS - wandRelief(w, p.z) - length(p.xy);
}

float march(vec3 ro, vec3 rd)
{
    float t = 0.02;
    for (int i = 0; i < 120; i++) {
        float d = mapTunnel(ro + rd * t);
        if (d < 0.0015 + 0.001 * t) break;
        t += d * 0.7;                 // Drossel: das Relief macht d zur Schaetzung
        if (t > 60.0) break;
    }
    return min(t, 60.0);
}

// Normale aus zentralen Differenzen der SDF
vec3 wandNormale(vec3 p)
{
    vec2 e = vec2(0.004, 0.0);
    return normalize(vec3(mapTunnel(p + e.xyy) - mapTunnel(p - e.xyy),
                          mapTunnel(p + e.yxy) - mapTunnel(p - e.yxy),
                          mapTunnel(p + e.yyx) - mapTunnel(p - e.yyx)));
}

void mainImage(out vec4 fragColor, in vec2 fragCoord)
{
    vec2 uv = (fragCoord - 0.5 * iResolution.xy) / iResolution.y;

    vec3 ro = vec3(0.0, 0.0, iTime * 2.0);
    vec3 rd = normalize(vec3(uv, 1.4));

    float t = march(ro, rd);
    vec3 p = ro + rd * t;

    float w  = atan(p.y, p.x);
    float wu = w / TAU + 0.5;

    float schach = mod(floor(wu * ROEHREN) + floor(p.z * 1.5), 2.0);
    vec3 basis = mix(vec3(0.05, 0.06, 0.10), vec3(0.16, 0.19, 0.27), schach);

    vec3 n = wandNormale(p);

    vec3 zk = ro - p;
    float dk = max(length(zk), 1e-3);
    float dif = max(dot(n, zk / dk), 0.0);
    vec3 color = basis * (1.6 * dif / (1.0 + dk * dk * 0.12) + 0.02);

    fragColor = vec4(color, 1.0);
}
