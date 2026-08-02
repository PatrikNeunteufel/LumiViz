// 22 Menger-Flug — Patrik Neunteufel (LumiViz, 2026). Eigenwerk.
// Shadertoy: iChannel0 = Music. RAYMARCHING-FRAKTAL (3D, unendlich).
//
// IDEE: Der Menger-Schwamm als Distance Estimator (Kreuz-Subtraktion, per
// Faltung wiederholt); der Raum wird per mod() unendlich gekachelt und die
// Kamera fliegt hindurch. Bass = Flugtempo, Höhen = Kanten-Glühen.

// ---- STELLSCHRAUBEN --------------------------------------------------------
const int   FALTUNGEN   = 4;     // Menger-Tiefe (mehr = feinere Löcher, teurer)
const int   MARSCH      = 96;
const float FLUG_GRUND  = 0.5;   // Flugtempo
const float FLUG_BASS   = 1.2;
const float KACHEL      = 4.0;   // Raumkachel-Größe
const float DREHUNG     = 0.12;  // langsame Rollbewegung im Flug
const float GLOW        = 1.4;   // Silhouetten-Glühen
// ----------------------------------------------------------------------------

vec2 rot2(vec2 p, float a) { return vec2(p.x * cos(a) - p.y * sin(a),
                                         p.x * sin(a) + p.y * cos(a)); }
// Box-SDF (Hilfsfunktion des Menger-DE)
float sdBox(vec3 p, vec3 b)
{
    vec3 q = abs(p) - b;
    return length(max(q, 0.0)) + min(max(q.x, max(q.y, q.z)), 0.0);
}
// Menger: Box minus unendliches Kreuz, auf 3 Achsen, skaliert wiederholt
float mengerDE(vec3 p)
{
    float d = sdBox(p, vec3(1.0));
    float s = 1.0;
    for (int i = 0; i < FALTUNGEN; ++i)
    {
        vec3 a = mod(p * s, 2.0) - 1.0;   // Zelle -1..1
        s *= 3.0;
        vec3 r = abs(1.0 - 3.0 * abs(a)); // Abstand zu den Kreuzarmen
        float da = max(r.x, r.y);
        float db = max(r.y, r.z);
        float dc = max(r.z, r.x);
        float c = (min(da, min(db, dc)) - 1.0) / s;  // das Loch
        d = max(d, c);                    // Loch aus der Box schneiden
    }
    return d;
}
void mainImage(out vec4 fragColor, in vec2 fragCoord)
{
    vec2 uv = (2.0 * fragCoord - iResolution.xy) / iResolution.y;
    float bass = texture(iChannel0, vec2(0.05, 0.25)).x;
    float treb = texture(iChannel0, vec2(0.70, 0.25)).x;

    // Kamera: geradeaus durch die Kachelwelt, mit leichter Rollbewegung
    float z0 = iTime * (FLUG_GRUND + FLUG_BASS * bass);
    vec3 ro = vec3(0.15 * sin(iTime * 0.3), 0.1 * cos(iTime * 0.23), z0);
    vec3 rd = normalize(vec3(rot2(uv, iTime * DREHUNG), 1.4));

    float t = 0.0;
    float d = 1.0;
    int schritte = 0;
    for (int i = 0; i < MARSCH; ++i)
    {
        // Raum kacheln: xy zentriert wiederholen, z läuft weiter
        vec3 p = ro + rd * t;
        p.xy = mod(p.xy + 0.5 * KACHEL, KACHEL) - 0.5 * KACHEL;
        p.z = mod(p.z + 0.5 * KACHEL, KACHEL) - 0.5 * KACHEL;
        d = mengerDE(p);
        schritte = i;
        if (d < 0.001 || t > 20.0) break;
        t += d;
    }
    vec3 col = vec3(0.005, 0.005, 0.02);
    if (d < 0.001)
    {
        // Tiefen-Nebel + Struktur-Farbe aus der Marschtiefe
        vec3 base = 0.5 + 0.5 * cos(t * 0.5 + iTime * 0.15 + vec3(0.0, 2.1, 4.2));
        col = base * exp(-t * 0.14);
    }
    // Kanten glühen (viele Schritte = Streifschuss) — Höhen verstärken
    col += GLOW * (0.2 + 0.6 * treb) *
           pow(float(schritte) / float(MARSCH), 3.0) * vec3(1.0, 0.5, 0.2);
    fragColor = vec4(col, 1.0);
}
