// 80 Bio-Lumineszenz — Patrik Neunteufel (LumiViz, 2026). Eigenwerk.
// Shadertoy: iChannel0 = Music. BIOLOGIE × GLOW: leuchtende Brandungswelle.
//
// IDEE: Nächtlicher Strand: eine Wellenkante (sin + FBM) läuft zyklisch
// auf den Strand und zieht sich zurück; ENTLANG der Kante leuchtet
// Plankton türkis (Glow um die Kantenlinie), Gischt-Punkte funkeln.
// Die Lautstärke ist die Brandungs-Energie (Leuchtkraft + Wellenhub).

// ---- STELLSCHRAUBEN --------------------------------------------------------
const float WELLEN_TEMPO = 0.25;  // Brandungszyklen pro Sekunde
const float WELLEN_HUB   = 0.25;  // wie weit die Welle aufläuft
const float LEUCHT_VOL   = 1.2;   // Plankton-Leuchtkraft aus der Lautstärke
const float KANTEN_GLOW  = 0.04;
const float GISCHT       = 0.995; // Funkeldichte (näher 1 = weniger)
const int   OKTAVEN      = 4;
// ----------------------------------------------------------------------------

float n21(vec2 p) { return fract(sin(dot(p, vec2(41.3, 289.1))) * 43758.5453); }
float vnoise(vec2 p)
{
    vec2 i = floor(p), f = fract(p);
    f = f * f * (3.0 - 2.0 * f);
    return mix(mix(n21(i), n21(i + vec2(1.0, 0.0)), f.x),
               mix(n21(i + vec2(0.0, 1.0)), n21(i + vec2(1.0, 1.0)), f.x), f.y);
}
float fbm(vec2 p)
{
    float v = 0.0, a = 0.5;
    for (int i = 0; i < OKTAVEN; ++i) { v += a * vnoise(p); p *= 2.07; a *= 0.5; }
    return v;
}
void mainImage(out vec4 fragColor, in vec2 fragCoord)
{
    vec2 uv = fragCoord / iResolution.xy;
    float vol = texture(iChannel0, vec2(0.15, 0.25)).x;

    // Wellenkante: Grundlinie + Auflauf-Zyklus + FBM-Ausfransung
    float zyklus = sin(iTime * WELLEN_TEMPO * 6.28318);
    float kante = 0.35 + WELLEN_HUB * zyklus * (0.6 + 0.4 * vol) +
                  0.08 * fbm(vec2(uv.x * 6.0, iTime * 0.4));
    float dKante = uv.y - kante;

    vec3 col;
    if (dKante < 0.0)
    {
        // Meer: dunkelblau, mit Tiefe dunkler; leichte Wellenstreifen
        float streifen = 0.5 + 0.5 * sin(uv.y * 60.0 + fbm(uv * 8.0) * 6.0 -
                                         iTime * 1.5);
        col = mix(vec3(0.0, 0.03, 0.08), vec3(0.0, 0.08, 0.14), streifen * 0.5);
        col *= 0.4 + 0.6 * uv.y / max(kante, 0.01);
    }
    else
    {
        // Strand: dunkler Sand, nass nahe der Kante (spiegelt das Leuchten)
        float nass = exp(-dKante * 12.0);
        col = vec3(0.05, 0.045, 0.04) * (0.6 + 0.4 * uv.y);
        col += nass * vec3(0.0, 0.25, 0.28) * 0.4 * (0.5 + LEUCHT_VOL * vol);
    }
    // DAS Leuchten: Plankton entlang der Wellenkante
    float leuchten = exp(-abs(dKante) / KANTEN_GLOW);
    col += leuchten * vec3(0.1, 0.9, 0.85) * (0.35 + LEUCHT_VOL * vol);
    // Gischt-Funkeln nahe der Kante
    float funkel = step(GISCHT, n21(floor(uv * 200.0) + floor(iTime * 8.0))) *
                   exp(-abs(dKante) * 8.0);
    col += funkel * vec3(0.6, 1.0, 0.95);
    // Nachthimmel-Abdunklung oben
    col *= 1.0 - 0.4 * smoothstep(0.6, 1.0, uv.y);
    fragColor = vec4(col, 1.0);
}
