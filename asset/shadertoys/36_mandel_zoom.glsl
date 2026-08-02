// 36 Mandel-Zoom — Patrik Neunteufel (LumiViz, 2026). Eigenwerk.
// Shadertoy: iChannel0 = Music. MANDELBROT mit atmendem Zoom.
//
// IDEE: Die Mandelbrot-Menge an einem "Seepferdchen"-Punkt; der Zoom atmet
// exponentiell (exp(sin)) hinein und wieder heraus — ein hypnotisches
// Pumpen. Glatte Iterationszählung (log-log) verhindert Farbringe.
// Der Bass verschiebt die Palette, die Höhen lassen den Rand glitzern.

// ---- STELLSCHRAUBEN --------------------------------------------------------
const vec2  ZENTRUM     = vec2(-0.74543, 0.11301); // Seepferdchen-Tal
const float ZOOM_MITTE  = 2.6;   // mittlere Zoom-Stufe (exp-Skala)
const float ZOOM_HUB    = 1.8;   // Atem-Amplitude (exp-Skala)
const float ATEM_TEMPO  = 0.15;
const int   ITERATIONEN = 160;
const float FARB_ZYKLEN = 0.08;  // Farbzyklen pro Iteration
// ----------------------------------------------------------------------------

void mainImage(out vec4 fragColor, in vec2 fragCoord)
{
    vec2 uv = (2.0 * fragCoord - iResolution.xy) / iResolution.y;
    float bass = texture(iChannel0, vec2(0.05, 0.25)).x;
    float treb = texture(iChannel0, vec2(0.70, 0.25)).x;

    // atmender Zoom: exponentiell rein/raus
    float zoom = exp(ZOOM_MITTE + ZOOM_HUB * sin(iTime * ATEM_TEMPO));
    vec2 c = ZENTRUM + uv / zoom;

    vec2 z = vec2(0.0);
    float m = 0.0;
    bool drin = true;
    for (int i = 0; i < ITERATIONEN; ++i)
    {
        z = vec2(z.x * z.x - z.y * z.y, 2.0 * z.x * z.y) + c;
        if (dot(z, z) > 64.0) { drin = false; break; }
        m += 1.0;
    }
    vec3 col = vec3(0.0);
    if (!drin)
    {
        // glatte Iterationszahl: m − log2(log|z|) entfernt die Stufenringe
        float glatt = m - log2(log2(dot(z, z)) * 0.5);
        col = 0.5 + 0.5 * cos(6.28318 * (glatt * FARB_ZYKLEN + bass * 0.15) +
                              iTime * 0.1 + vec3(0.0, 2.1, 4.2));
        // Rand-Glitzern: hohe Iterationszahlen (nahe der Menge) leuchten auf
        col += treb * 0.6 * smoothstep(0.75, 1.0, m / float(ITERATIONEN));
    }
    fragColor = vec4(col, 1.0);
}
