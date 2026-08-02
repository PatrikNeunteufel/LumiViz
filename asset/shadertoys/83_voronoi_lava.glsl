// 83 Voronoi-Lava — Patrik Neunteufel (LumiViz, 2026). Eigenwerk.
// Shadertoy: iChannel0 = Music. KOMBI: Voronoi × Lava — die ZELLGRENZEN
// sind glühende Lavaadern zwischen dunklen Basaltschollen (zweite
// Voronoi-Distanz), die Glut pulst mit dem Bass durch das Adernetz.

// ---- STELLSCHRAUBEN --------------------------------------------------------
const float ZOOM        = 3.0;
const float ADER_BREITE = 0.06;
const float PULS_TEMPO  = 1.5;
const float BASS_GLUT   = 1.2;
const float WANDERN     = 0.18;  // Schollen driften
// ----------------------------------------------------------------------------

vec2 hash2(vec2 p)
{
    p = vec2(dot(p, vec2(127.1, 311.7)), dot(p, vec2(269.5, 183.3)));
    return fract(sin(p) * 43758.5453);
}
void mainImage(out vec4 fragColor, in vec2 fragCoord)
{
    vec2 uv = (2.0 * fragCoord - iResolution.xy) / iResolution.y * ZOOM;
    float bass = texture(iChannel0, vec2(0.05, 0.25)).x;

    vec2 g = floor(uv);
    vec2 f = fract(uv);
    float d1 = 8.0, d2 = 8.0;
    vec2 id = vec2(0.0);
    for (int y = -1; y <= 1; ++y)
    for (int x = -1; x <= 1; ++x)
    {
        vec2 o = vec2(float(x), float(y));
        vec2 h = hash2(g + o);
        vec2 c = o + 0.5 + WANDERN * sin(iTime * 0.3 + 6.28318 * h) - f;
        float d = length(c);
        if (d < d1) { d2 = d1; d1 = d; id = h; }
        else if (d < d2) d2 = d;
    }
    float grenze = d2 - d1;  // 0 an der Zellgrenze
    // Lavaader: schmal an der Grenze, Glut pulst das Netz entlang
    float ader = smoothstep(ADER_BREITE, 0.0, grenze);
    float puls = 0.6 + 0.4 * sin(id.x * 6.28318 + d1 * 6.0 - iTime * PULS_TEMPO);
    float glut = ader * puls * (0.7 + BASS_GLUT * bass);
    // Scholle: dunkler Basalt mit leichtem Krustenmuster
    vec3 basalt = vec3(0.05, 0.04, 0.045) * (0.7 + 0.3 * id.y);
    vec3 col = basalt;
    col += glut * vec3(1.0, 0.35, 0.05);
    col += glut * glut * vec3(1.0, 0.8, 0.3);   // heißer Kern der Ader
    col += exp(-grenze * 8.0) * vec3(0.3, 0.05, 0.0) * 0.4;  // Wärmesaum
    fragColor = vec4(col, 1.0);
}
