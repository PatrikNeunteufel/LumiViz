// 91 Sternen-Feuerwerk, Buffer A — Patrik Neunteufel (LumiViz, 2026). Eigenwerk.
// "Buffer A": iChannel0 = SELBST, iChannel1 = Music. KOMBI: Sternenflug ×
// Feuerwerk — während des Warp-Flugs detonieren Farb-Bursts im Feld
// (Explosionen wie Feuerwerk, aber im All; Trail zieht beides).

// ---- STELLSCHRAUBEN --------------------------------------------------------
const float TRAIL      = 0.86;
const int   STERNE     = 48;
const float FLUG_BASS  = 1.0;
const float BURST_RATE = 0.6;   // Detonationen pro Sekunde
const int   BURST_FUNKEN = 28;
// ----------------------------------------------------------------------------

float h1(float n) { return fract(sin(n) * 43758.5453); }
void mainImage(out vec4 fragColor, in vec2 fragCoord)
{
    vec2 uv = (2.0 * fragCoord - iResolution.xy) / iResolution.y;
    float bass = texture(iChannel1, vec2(0.05, 0.25)).x;
    vec3 col = texture(iChannel0, fragCoord / iResolution.xy).rgb * TRAIL;

    // Warp-Sterne (kompakte Version von Nr. 08)
    float speed = 0.25 + FLUG_BASS * bass;
    for (int i = 0; i < STERNE; ++i)
    {
        float fi = float(i);
        float z = fract(h1(fi * 7.31) - iTime * speed * (0.3 + 0.7 * h1(fi + 0.7)));
        vec2 dir = normalize(vec2(h1(fi * 3.7) - 0.5, h1(fi * 9.1) - 0.5) + 1e-4);
        vec2 p = dir * (0.05 + 2.0 * (1.0 - z));
        col += smoothstep(0.015 * (1.2 - z), 0.0, length(uv - p)) *
               vec3(0.7, 0.8, 1.0) * (1.0 - z) * 0.8;
    }
    // Bursts: kugelförmige Funken-Detonationen an Zufallsorten
    for (int k = 0; k < 2; ++k)
    {
        float fenster = iTime * BURST_RATE + float(k) * 0.5;
        float seed = floor(fenster) * 3.0 + float(k);
        float t = fract(fenster) / BURST_RATE;
        vec2 zentrum = vec2(h1(seed * 3.3) * 1.6 - 0.8, h1(seed * 7.7) * 1.2 - 0.6);
        float glut = exp(-t * 2.0);
        vec3 farbe = 0.5 + 0.5 * cos(seed * 29.0 + vec3(0.0, 2.1, 4.2));
        for (int i = 0; i < BURST_FUNKEN; ++i)
        {
            float fi = float(i);
            float w = 6.28318 * fi / float(BURST_FUNKEN) + h1(seed + fi) * 0.4;
            vec2 p = zentrum + vec2(cos(w), sin(w)) * (0.2 + 0.15 * h1(seed * 5.0 + fi)) * t;
            col += smoothstep(0.005, 0.0, length(uv - p)) * farbe * glut;
        }
    }
    fragColor = vec4(min(col, vec3(2.5)), 1.0);
}
