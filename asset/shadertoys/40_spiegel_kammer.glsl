// 40 Spiegel-Kammer — Patrik Neunteufel (LumiViz, 2026). Eigenwerk.
// Shadertoy: iChannel0 = Music. 2D-IFS (gefaltete Spiegelräume).
//
// IDEE: Die Ebene wird mehrfach gespiegelt (abs), rotiert und skaliert —
// wie ein Blick in einen verspiegelten Raum, der sich selbst enthält.
// In jeder Tiefe wird ein Neon-Rechteckrahmen gezeichnet; die Überlagerung
// aller Tiefen ergibt die unendliche Kammer. Mitten = Faltwinkel-Morphing,
// Bass = Rahmen-Puls.

// ---- STELLSCHRAUBEN --------------------------------------------------------
const int   TIEFEN       = 6;     // Spiegelungs-Tiefen
const float SKALA        = 1.45;  // Vergrößerung je Tiefe
const float WINKEL_GRUND = 0.35;
const float WINKEL_MITTEN = 0.30; // Mitten drehen die Spiegel
const float WINKEL_DRIFT = 0.04;
const float RAHMEN_GROESSE = 0.55;
const float RAHMEN_DICKE = 0.015;
const float BASS_PULS    = 0.10;  // Bass pumpt die Rahmengröße
// ----------------------------------------------------------------------------

vec2 rot2(vec2 p, float a) { return vec2(p.x * cos(a) - p.y * sin(a),
                                         p.x * sin(a) + p.y * cos(a)); }
// Abstand zum Rechteckrahmen (Kante eines Quadrats)
float rahmen(vec2 p, float halbe)
{
    vec2 q = abs(p) - halbe;
    return abs(length(max(q, 0.0)) + min(max(q.x, q.y), 0.0));
}
void mainImage(out vec4 fragColor, in vec2 fragCoord)
{
    vec2 uv = (2.0 * fragCoord - iResolution.xy) / iResolution.y;
    float bass = texture(iChannel0, vec2(0.05, 0.25)).x;
    float mid  = texture(iChannel0, vec2(0.30, 0.25)).x;
    float winkel = WINKEL_GRUND + WINKEL_MITTEN * mid + iTime * WINKEL_DRIFT;
    float groesse = RAHMEN_GROESSE + BASS_PULS * bass;

    vec3 col = vec3(0.01, 0.01, 0.02);
    vec2 p = uv;
    float s = 1.0;
    for (int i = 0; i < TIEFEN; ++i)
    {
        // Neon-Rahmen dieser Tiefe (Abstand in den Original-Maßstab zurück)
        float d = rahmen(p, groesse) / s;
        float kern = smoothstep(RAHMEN_DICKE, RAHMEN_DICKE * 0.4, d);
        float halo = smoothstep(RAHMEN_DICKE * 6.0, 0.0, d) * 0.25;
        vec3 farbe = 0.5 + 0.5 * cos(float(i) * 1.1 + iTime * 0.3 +
                                     vec3(0.0, 2.1, 4.2));
        col += (kern + halo) * farbe / (1.0 + float(i) * 0.6);
        // Spiegelung für die nächste Tiefe: falten, drehen, vergrößern
        p = abs(p) - groesse * 0.5;
        p = rot2(p, winkel);
        p *= SKALA;
        s *= SKALA;
    }
    fragColor = vec4(col, 1.0);
}
