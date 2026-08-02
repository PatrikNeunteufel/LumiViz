// 57 Phönix-Flug — Patrik Neunteufel (LumiViz, 2026). Eigenwerk.
// Shadertoy: iChannel0 = Music. FRAKTAL: die Phönix-Iteration.
//
// IDEE: z_{n+1} = z_n² + c + p·z_{n−1} — der Rückgriff auf den VORLETZTEN
// Wert (p) erzeugt die federartigen Phönix-Strukturen. p pendelt langsam,
// der Bass schlägt mit den "Flügeln" (p-Störung).

// ---- STELLSCHRAUBEN --------------------------------------------------------
const vec2  C           = vec2(0.5667, 0.0);   // klassischer Phönix-Punkt
const float P_GRUND     = -0.5;
const float P_PENDEL    = 0.06;  // langsames Pendeln von p
const float P_BASS      = 0.08;  // Bass-Flügelschlag
const int   ITERATIONEN = 96;
const float ZOOM        = 1.3;
const float FARB_ZYKLEN = 1.4;
// ----------------------------------------------------------------------------

void mainImage(out vec4 fragColor, in vec2 fragCoord)
{
    vec2 uv = (2.0 * fragCoord - iResolution.xy) / iResolution.y * ZOOM;
    float bass = texture(iChannel0, vec2(0.05, 0.25)).x;
    float p = P_GRUND + P_PENDEL * sin(iTime * 0.2) + P_BASS * bass;

    vec2 z = vec2(uv.y, uv.x);  // klassische Phönix-Ansicht: gedreht
    vec2 zAlt = vec2(0.0);
    float m = 0.0;
    bool drin = true;
    for (int i = 0; i < ITERATIONEN; ++i)
    {
        vec2 zNeu = vec2(z.x * z.x - z.y * z.y, 2.0 * z.x * z.y) + C + p * zAlt;
        zAlt = z;
        z = zNeu;
        if (dot(z, z) > 64.0) { drin = false; break; }
        m += 1.0;
    }
    vec3 col = vec3(0.0);
    if (!drin)
    {
        float glatt = m - log2(log2(dot(z, z)) * 0.5);
        float s = glatt / float(ITERATIONEN);
        // Feder-Palette: tiefes Violett → Orange → Goldweiß
        col = 0.5 + 0.5 * cos(6.28318 * (s * FARB_ZYKLEN + 0.05 * iTime) +
                              vec3(4.5, 2.8, 1.2));
        col *= pow(s, 0.4) * 1.3;
    }
    fragColor = vec4(col, 1.0);
}
