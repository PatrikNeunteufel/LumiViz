// 60 Radar-Phosphor — Patrik Neunteufel (LumiViz, 2026). Eigenwerk.
// Shadertoy: iChannel0 = Music. TECHNIK: Radarschirm mit Nachleuchten.
//
// IDEE: Der Nachleucht-Effekt OHNE Buffer: die Helligkeit hängt vom
// Winkelabstand HINTER dem rotierenden Strahl ab (je weiter zurück, desto
// dunkler) — exakt wie Phosphor verblasst. Blips sitzen an gehashten
// Positionen und leben ein paar Umdrehungen; Bass = Umlauftempo.

// ---- STELLSCHRAUBEN --------------------------------------------------------
const float UMLAUF_GRUND = 0.5;   // Umdrehungen pro Sekunde
const float UMLAUF_BASS  = 0.4;
const float NACHLEUCHTEN = 3.5;   // Verblass-Stärke (größer = kürzer)
const int   BLIPS        = 8;
const float BLIP_GROESSE = 0.012;
const float RING_ZAHL    = 4.0;   // Entfernungsringe
// ----------------------------------------------------------------------------

float h1(float n) { return fract(sin(n) * 43758.5453); }
void mainImage(out vec4 fragColor, in vec2 fragCoord)
{
    vec2 uv = (2.0 * fragCoord - iResolution.xy) / iResolution.y;
    float bass = texture(iChannel0, vec2(0.05, 0.25)).x;
    float r = length(uv);
    float a = atan(uv.y, uv.x);

    float umlauf = iTime * (UMLAUF_GRUND + UMLAUF_BASS * bass) * 6.28318;
    // Winkelabstand hinter dem Strahl: 0 = frisch überstrichen
    float hinter = fract((umlauf - a) / 6.28318);
    float phosphor = exp(-hinter * NACHLEUCHTEN);

    vec3 col = vec3(0.0);
    if (r < 1.0)
    {
        // Grundschirm + Entfernungsringe + Fadenkreuz
        col = vec3(0.0, 0.05, 0.015);
        float ring = smoothstep(0.008, 0.0, abs(fract(r * RING_ZAHL) - 0.5) / RING_ZAHL);
        float kreuz = smoothstep(0.004, 0.0, min(abs(uv.x), abs(uv.y)));
        col += (ring * 0.5 + kreuz * 0.4) * vec3(0.0, 0.35, 0.12);
        // Phosphor-Keil hinter dem Strahl
        col += phosphor * vec3(0.05, 0.6, 0.18) * (1.0 - r * 0.5);
        // Strahl selbst
        col += smoothstep(0.02, 0.0, hinter) * vec3(0.3, 1.0, 0.45);
        // Blips: leuchten auf, wenn der Strahl sie überstreicht, verblassen mit
        for (int i = 0; i < BLIPS; ++i)
        {
            float fi = float(i);
            // Blip wandert langsam (Kurs), lebt zyklisch
            vec2 p = (0.25 + 0.65 * h1(fi * 3.3)) *
                     vec2(cos(h1(fi * 7.1) * 6.28318 + iTime * 0.05),
                          sin(h1(fi * 7.1) * 6.28318 + iTime * 0.04));
            float pa = atan(p.y, p.x);
            float ph = exp(-fract((umlauf - pa) / 6.28318) * NACHLEUCHTEN * 0.7);
            col += smoothstep(BLIP_GROESSE, 0.0, length(uv - p)) *
                   vec3(0.5, 1.0, 0.6) * ph * 1.5;
        }
        col *= smoothstep(1.0, 0.97, r);  // Schirmrand
    }
    fragColor = vec4(col, 1.0);
}
