// 21 Mandelbulb-Puls — Patrik Neunteufel (LumiViz, 2026). Eigenwerk.
// Shadertoy: iChannel0 = Music. RAYMARCHING-FRAKTAL (3D).
//
// IDEE: Der Mandelbulb ist die 3D-Verallgemeinerung der Mandelbrot-Menge:
// z -> z^POWER + c in Kugelkoordinaten. Ein Distance Estimator (DE) liefert
// eine sichere Marschdistanz; ein Orbit-Trap (kleinster |z| unterwegs)
// färbt die Oberfläche. Der Bass pulst die Power — das Fraktal "atmet".

// ---- STELLSCHRAUBEN --------------------------------------------------------
const float POWER_GRUND  = 8.0;   // klassischer Bulb; 2..12 = andere Wesen
const float POWER_BASS   = 1.5;   // Bass-Puls auf die Power
const int   FRAKTAL_ITER = 8;     // Fraktal-Iterationen (Detailtiefe)
const int   MARSCH       = 90;    // Raymarch-Schritte
const float KAMERA_DIST  = 2.6;   // Abstand der Kamera
const float DREH_TEMPO   = 0.25;  // Umkreisung des Bulbs
const float GLOW_STAERKE = 1.2;   // Rand-Glow aus der Schrittzahl
// ----------------------------------------------------------------------------

float g_power;
float g_trap;  // Orbit-Trap: kleinster |z| während der Iteration

// Distance Estimator des Mandelbulb (Standard-Herleitung über |z'|)
float mandelbulbDE(vec3 p)
{
    vec3 z = p;
    float dr = 1.0;
    float r = 0.0;
    g_trap = 1e9;
    for (int i = 0; i < FRAKTAL_ITER; ++i)
    {
        r = length(z);
        g_trap = min(g_trap, r);
        if (r > 2.0) break;
        // Kugelkoordinaten: Winkel mit POWER multiplizieren, Radius potenzieren
        float theta = acos(clamp(z.z / max(r, 1e-6), -1.0, 1.0)) * g_power;
        float phi = atan(z.y, z.x) * g_power;
        float zr = pow(r, g_power);
        dr = pow(r, g_power - 1.0) * g_power * dr + 1.0;
        z = zr * vec3(sin(theta) * cos(phi), sin(theta) * sin(phi), cos(theta)) + p;
    }
    return 0.5 * log(max(r, 1e-6)) * r / dr;
}
void mainImage(out vec4 fragColor, in vec2 fragCoord)
{
    vec2 uv = (2.0 * fragCoord - iResolution.xy) / iResolution.y;
    float bass = texture(iChannel0, vec2(0.05, 0.25)).x;
    float treb = texture(iChannel0, vec2(0.70, 0.25)).x;
    g_power = POWER_GRUND + POWER_BASS * bass;

    // Kamera umkreist den Bulb
    float ca = iTime * DREH_TEMPO;
    vec3 ro = KAMERA_DIST * vec3(cos(ca), 0.35 * sin(ca * 0.7), sin(ca));
    vec3 ww = normalize(-ro);                        // Blick zur Mitte
    vec3 uu = normalize(cross(ww, vec3(0.0, 1.0, 0.0)));
    vec3 vv = cross(uu, ww);
    vec3 rd = normalize(uv.x * uu + uv.y * vv + 1.8 * ww);

    float t = 0.0;
    float d = 1.0;
    int schritte = 0;
    for (int i = 0; i < MARSCH; ++i)
    {
        d = mandelbulbDE(ro + rd * t);
        schritte = i;
        if (d < 0.0008 * t || t > 6.0) break;
        t += d;
    }
    vec3 col = vec3(0.01, 0.01, 0.03);
    if (d < 0.0008 * t)
    {
        // Farbe aus dem Orbit-Trap (Fraktal-Tiefe) + Cosinus-Palette
        vec3 base = 0.5 + 0.5 * cos(g_trap * 5.0 + iTime * 0.2 + vec3(0.0, 2.1, 4.2));
        // simple Tiefen-Schattierung statt teurer Normalen
        float licht = clamp(1.0 - t / 5.0, 0.0, 1.0);
        col = base * licht;
    }
    // Rand-Glow: viele Schritte = knapp vorbei = Silhouette leuchtet
    col += GLOW_STAERKE * (0.15 + 0.5 * treb) *
           pow(float(schritte) / float(MARSCH), 3.0) * vec3(0.4, 0.6, 1.0);
    fragColor = vec4(col, 1.0);
}
