// 09 Raymarch-Blobs — Patrik Neunteufel (LumiViz, 2026). Eigenwerk.
// Shadertoy: iChannel0 = Music (512x2: y=0.25 FFT, y=0.75 Waveform).
//
// IDEE: Raymarching (Sphere-Tracing) über ein SDF: drei Kugeln, per
// smooth-min zu Metaballs verschmolzen; Radien atmen mit Bass/Mitten/Höhen.

// ---- STELLSCHRAUBEN --------------------------------------------------------
const float RADIUS_1     = 0.45;  // Grundradien der drei Blobs
const float RADIUS_2     = 0.35;
const float RADIUS_3     = 0.30;
const float ATMUNG_1     = 0.35;  // Radius-Zuwachs je Audio-Band (Bass/Mitte/Höhen)
const float ATMUNG_2     = 0.30;
const float ATMUNG_3     = 0.25;
const float VERSCHMELZUNG = 0.4;  // smooth-min-k: größer = zäherer Teig
const int   SCHRITTE     = 80;    // Marsch-Schritte (Qualität vs. Kosten)
const float MAX_DIST     = 6.0;
// ----------------------------------------------------------------------------

float g_bass, g_mid, g_treb;  // global, damit map() sie sieht

// smooth-min: weiches Minimum mit Übergangsbreite k (Metaball-Verschmelzung)
float smin(float a, float b, float k)
{
    float h = clamp(0.5 + 0.5 * (b - a) / k, 0.0, 1.0);
    return mix(b, a, h) - k * h * (1.0 - h);
}
// SDF der Szene: Abstand zur nächsten Oberfläche (Kugel: length−Radius)
float map(vec3 p)
{
    float d = length(p - vec3(0.7 * cos(iTime), 0.5 * sin(iTime * 1.3), 0.0)) -
              (RADIUS_1 + ATMUNG_1 * g_bass);
    d = smin(d, length(p - vec3(-0.6 * cos(iTime * 0.8), -0.4 * sin(iTime), 0.2)) -
                    (RADIUS_2 + ATMUNG_2 * g_mid), VERSCHMELZUNG);
    d = smin(d, length(p - vec3(0.0, 0.7 * cos(iTime * 0.6), -0.2)) -
                    (RADIUS_3 + ATMUNG_3 * g_treb), VERSCHMELZUNG);
    return d;
}
void mainImage(out vec4 fragColor, in vec2 fragCoord)
{
    vec2 uv = (2.0 * fragCoord - iResolution.xy) / iResolution.y;
    g_bass = texture(iChannel0, vec2(0.05, 0.25)).x;
    g_mid  = texture(iChannel0, vec2(0.30, 0.25)).x;
    g_treb = texture(iChannel0, vec2(0.70, 0.25)).x;

    vec3 ro = vec3(0.0, 0.0, -2.6);      // Kamera
    vec3 rd = normalize(vec3(uv, 1.6));  // Strahl (1.6 = Brennweite)

    // Sphere-Tracing: immer um die sichere Distanz d vorrücken
    float t = 0.0;
    float d = 1.0;
    for (int i = 0; i < SCHRITTE; ++i)
    {
        d = map(ro + rd * t);
        if (d < 0.001 || t > MAX_DIST) break;
        t += d;
    }
    vec3 col = vec3(0.01, 0.01, 0.03);
    if (d < 0.001)
    {
        vec3 p = ro + rd * t;
        // Normale = SDF-Gradient (zentrale Differenzen)
        vec2 e = vec2(0.002, 0.0);
        vec3 n = normalize(vec3(map(p + e.xyy) - map(p - e.xyy),
                                map(p + e.yxy) - map(p - e.yxy),
                                map(p + e.yyx) - map(p - e.yyx)));
        vec3 l = normalize(vec3(0.6, 0.7, -0.5));
        float diff = max(dot(n, l), 0.0);
        float spec = pow(max(dot(reflect(-l, n), -rd), 0.0), 32.0);
        vec3 base = 0.5 + 0.5 * cos(p.z * 2.0 + iTime * 0.5 + vec3(0.0, 2.1, 4.2));
        col = base * (0.15 + 0.85 * diff) + spec * vec3(1.0);
    }
    fragColor = vec4(col, 1.0);
}
