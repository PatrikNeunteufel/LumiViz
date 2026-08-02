// 68 Kristall-Höhle — Patrik Neunteufel (LumiViz, 2026). Eigenwerk.
// Shadertoy: iChannel0 = Music. RAYMARCHING: Höhle voller Kristallnadeln.
//
// IDEE: Gekachelte, verdrehte Oktaeder-Nadeln (SDF) ragen aus Boden und
// Decke; die Kamera schwebt hindurch. Kristalle schimmern über einen
// tiefenabhängigen Farbzyklus + Fresnel-Kantenlicht. Bass = Puls-Glühen.

// ---- STELLSCHRAUBEN --------------------------------------------------------
const int   MARSCH     = 80;
const float KACHEL     = 1.6;
const float NADEL_DICKE = 0.22;
const float FLUG_TEMPO = 0.4;
const float GLUEH_BASS = 0.8;
// ----------------------------------------------------------------------------

vec2 rot2(vec2 p, float a) { return vec2(p.x * cos(a) - p.y * sin(a),
                                         p.x * sin(a) + p.y * cos(a)); }
float n21(vec2 p) { return fract(sin(dot(p, vec2(41.3, 289.1))) * 43758.5453); }
// Oktaeder-SDF (|x|+|y|+|z| = konstant), vertikal gestreckt = Nadel
float nadel(vec3 p, float streck)
{
    p.y /= streck;
    return (abs(p.x) + abs(p.y) + abs(p.z) - NADEL_DICKE) * 0.57735 * streck;
}
float szene(vec3 p)
{
    vec2 id = floor(p.xz / KACHEL);
    vec2 lokal = mod(p.xz, KACHEL) - 0.5 * KACHEL;
    float rnd = n21(id);
    // je Zelle eine verdrehte Nadel von unten UND oben
    vec3 q = vec3(lokal.x, p.y, lokal.y);
    q.xz = rot2(q.xz, rnd * 6.28318);
    float unten = nadel(q - vec3(0.0, -1.2 + rnd * 0.6, 0.0), 2.5 + 2.0 * rnd);
    float oben  = nadel(q - vec3(0.0,  1.2 - rnd * 0.4, 0.0), 2.0 + 1.5 * rnd);
    return min(unten, oben);
}
void mainImage(out vec4 fragColor, in vec2 fragCoord)
{
    vec2 uv = (2.0 * fragCoord - iResolution.xy) / iResolution.y;
    float bass = texture(iChannel0, vec2(0.05, 0.25)).x;

    vec3 ro = vec3(0.3 * sin(iTime * 0.2), 0.15 * sin(iTime * 0.17),
                   iTime * FLUG_TEMPO);
    vec3 rd = normalize(vec3(uv, 1.3));
    rd.xz = rot2(rd.xz, 0.1 * sin(iTime * 0.15));

    float t = 0.0;
    float d = 1.0;
    int schritte = 0;
    for (int i = 0; i < MARSCH; ++i)
    {
        d = szene(ro + rd * t);
        schritte = i;
        if (d < 0.002 || t > 14.0) break;
        t += d * 0.85;  // konservativer Schritt (verdrehte Kacheln)
    }
    vec3 col = vec3(0.01, 0.005, 0.02);
    if (d < 0.002)
    {
        vec3 p = ro + rd * t;
        vec2 e = vec2(0.004, 0.0);
        vec3 n = normalize(vec3(szene(p + e.xyy) - szene(p - e.xyy),
                                szene(p + e.yxy) - szene(p - e.yxy),
                                szene(p + e.yyx) - szene(p - e.yyx)));
        float fresnel = pow(1.0 - max(dot(n, -rd), 0.0), 2.0);
        // Schimmer: Farbzyklus über Tiefe + Höhe, pulst mit dem Bass
        vec3 kristall = 0.5 + 0.5 * cos(p.z * 0.8 + p.y * 2.0 + iTime * 0.3 +
                                        vec3(0.0, 2.1, 4.2));
        col = kristall * (0.25 + 0.4 * max(dot(n, normalize(vec3(0.5, 0.8, -0.3))), 0.0));
        col += fresnel * kristall * (0.5 + GLUEH_BASS * bass);
        col *= exp(-t * 0.22);  // Höhlen-Dunkelheit
    }
    col += pow(float(schritte) / float(MARSCH), 3.0) * vec3(0.3, 0.15, 0.5) * 0.6;
    fragColor = vec4(col, 1.0);
}
