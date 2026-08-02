// 08 Sternenfeld-Warp, Buffer A — Patrik Neunteufel (LumiViz, 2026). Eigenwerk.
// Shadertoy-Tab "Buffer A": iChannel0 = Buffer A (SELBST = Vorframe!),
//                           iChannel1 = Music.
//
// IDEE: Sterne fliegen radial nach außen; Tiefe z läuft je Stern 1→0 und
// wrappt (endloser Flug). Der Decay-Trail macht bei Bass Warp-Streifen.

// ---- STELLSCHRAUBEN --------------------------------------------------------
const int   STERNE       = 64;    // Anzahl Sterne
const float TRAIL_GRUND  = 0.80;  // Trail-Decay (näher an 1 = längere Streifen)
const float TRAIL_BASS   = 0.15;  // Bass verlängert den Trail zusätzlich
const float TEMPO_GRUND  = 0.20;  // Grund-Fluggeschwindigkeit
const float TEMPO_BASS   = 1.2;   // Bass-Schub
const float BAHN_INNEN   = 0.05;  // Startabstand von der Mitte
const float BAHN_AUSSEN  = 2.2;   // wie weit die Bahn nach außen reicht
const float STERN_GROESSE = 0.02; // Punktgröße (nah skaliert mit)
// ----------------------------------------------------------------------------

float hash1(float n) { return fract(sin(n) * 43758.5453); }  // 1D-Hash 0..1
void mainImage(out vec4 fragColor, in vec2 fragCoord)
{
    vec2 uv = (2.0 * fragCoord - iResolution.xy) / iResolution.y;
    float bass = texture(iChannel1, vec2(0.05, 0.25)).x;
    vec3 col = texture(iChannel0, fragCoord / iResolution.xy).rgb *
               (TRAIL_GRUND + TRAIL_BASS * bass);
    float speed = TEMPO_GRUND + TEMPO_BASS * bass;
    for (int i = 0; i < STERNE; ++i)
    {
        float fi = float(i);
        // Tiefe 1→0, wrappt; jeder Stern mit eigenem Tempo (Hash)
        float z = fract(hash1(fi * 7.31) - iTime * speed * (0.2 + 0.8 * hash1(fi + 0.7)));
        vec2 dir = normalize(vec2(hash1(fi * 3.7) - 0.5, hash1(fi * 9.1) - 0.5) + 1e-4);
        vec2 p = dir * (BAHN_INNEN + BAHN_AUSSEN * (1.0 - z));
        float d = length(uv - p);
        // nah (kleines z) = größer und heller
        col += smoothstep(STERN_GROESSE * (1.2 - z), 0.0, d) *
               vec3(0.8, 0.9, 1.0) * (1.0 - z);
    }
    fragColor = vec4(min(col, vec3(2.0)), 1.0);  // Deckel gegen Aufschaukeln
}
