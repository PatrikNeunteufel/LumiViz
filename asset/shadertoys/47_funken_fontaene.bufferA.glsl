// 47 Funken-Fontäne, Buffer A — Patrik Neunteufel (LumiViz, 2026). Eigenwerk.
// "Buffer A": iChannel0 = Buffer A (SELBST!), iChannel1 = Music.
//
// IDEE: Ein Boden-Vulkan sprüht Funken (ballistische Bahnen mit Streuwinkel
// und Zufallsimpuls, endlos wrappend); die Lautstärke macht den Strahl
// höher und dichter, der Trail zieht Goldfäden.

// ---- STELLSCHRAUBEN --------------------------------------------------------
const float TRAIL      = 0.88;
const int   FUNKEN     = 90;
const float STREUWINKEL = 0.35;  // halber Kegelwinkel (rad)
const float IMPULS     = 0.9;    // Grund-Abschussstärke
const float IMPULS_VOL = 0.5;    // Lautstärke-Zuschlag
const float SCHWERKRAFT = 0.9;
const float LEBENSDAUER = 1.8;   // Sekunden je Funke
// ----------------------------------------------------------------------------

float h1(float n) { return fract(sin(n) * 43758.5453); }
void mainImage(out vec4 fragColor, in vec2 fragCoord)
{
    vec2 uv = (2.0 * fragCoord - iResolution.xy) / iResolution.y;
    float vol = texture(iChannel1, vec2(0.15, 0.25)).x;
    vec3 col = texture(iChannel0, fragCoord / iResolution.xy).rgb * TRAIL;

    vec2 quelle = vec2(0.0, -0.85);
    for (int i = 0; i < FUNKEN; ++i)
    {
        float fi = float(i);
        // Lebenszyklus: jeder Funke hat Phasenversatz, wrappt endlos
        float t = fract(iTime / LEBENSDAUER + h1(fi * 3.3)) * LEBENSDAUER;
        float alter = t / LEBENSDAUER;
        float w = 1.5708 + STREUWINKEL * (h1(fi * 7.1) * 2.0 - 1.0);  // um senkrecht
        float kraft = (IMPULS + IMPULS_VOL * vol) * (0.7 + 0.5 * h1(fi * 5.7));
        vec2 p = quelle + vec2(cos(w), sin(w)) * kraft * t -
                 vec2(0.0, 0.5 * SCHWERKRAFT * t * t);
        float glut = (1.0 - alter);
        // Farbe: weißgold jung, orange-rot alt
        vec3 farbe = mix(vec3(1.0, 0.55, 0.15), vec3(1.0, 0.95, 0.8), glut);
        col += smoothstep(0.005, 0.0, length(uv - p)) * farbe * glut * glut;
    }
    fragColor = vec4(min(col, vec3(2.5)), 1.0);
}
