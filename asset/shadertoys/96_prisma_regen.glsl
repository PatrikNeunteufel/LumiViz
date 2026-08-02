// 96 Prisma-Regen — Patrik Neunteufel (LumiViz, 2026). Eigenwerk.
// Shadertoy: iChannel0 = Music. KOMBI: Regentropfen × Dispersion — jeder
// fallende Tropfen zieht einen kleinen REGENBOGEN-Schweif (RGB-Kanäle
// leicht versetzt gesampelt = chromatische Auffächerung). Lautstärke =
// Regendichte.

// ---- STELLSCHRAUBEN --------------------------------------------------------
const int   TROPFEN    = 60;
const float FALL_DAUER = 2.2;
const float SCHWEIF    = 0.10;   // Schweiflänge
const float DISPERSION = 0.35;   // Farb-Auffächerung im Schweif
const float VOL_DICHTE = 0.5;    // Lautstärke blendet mehr Tropfen ein
// ----------------------------------------------------------------------------

float h1(float n) { return fract(sin(n) * 43758.5453); }
void mainImage(out vec4 fragColor, in vec2 fragCoord)
{
    vec2 uv = (2.0 * fragCoord - iResolution.xy) / iResolution.y;
    float vol = texture(iChannel0, vec2(0.15, 0.25)).x;
    vec3 col = vec3(0.008, 0.01, 0.02);

    for (int i = 0; i < TROPFEN; ++i)
    {
        float fi = float(i);
        // Tropfen erst ab Schwelle sichtbar (Lautstärke = Dichte)
        if (h1(fi * 13.7) > 0.4 + VOL_DICHTE * vol) continue;
        float t = fract(iTime / FALL_DAUER + h1(fi * 3.3));
        vec2 p = vec2(h1(fi * 7.7) * 2.4 - 1.2, 0.85 - 1.8 * t);
        // Kern des Tropfens
        float d = length(uv - p);
        col += smoothstep(0.006, 0.0, d) * vec3(0.85, 0.92, 1.0);
        // Regenbogen-Schweif: Punkte oberhalb des Tropfens, Farbe über die
        // Schweifposition (Dispersion: rot oben … violett unten)
        float dy = uv.y - p.y;
        if (dy > 0.0 && dy < SCHWEIF)
        {
            float fx = abs(uv.x - p.x - DISPERSION * dy * (uv.x > p.x ? 1.0 : -1.0) * 0.3);
            float faecher = smoothstep(0.012 + dy * DISPERSION, 0.0, fx);
            float spektrum = dy / SCHWEIF;  // 0 rot .. 1 violett
            vec3 farbe = 0.5 + 0.5 * cos(6.28318 * (spektrum * 0.75) +
                                         vec3(0.0, 2.1, 4.2));
            col += faecher * farbe * (1.0 - spektrum) * 0.7;
        }
    }
    // Boden-Schimmer, wo die Tropfen landen
    col += smoothstep(0.05, 0.0, abs(uv.y + 0.95)) * vec3(0.1, 0.12, 0.2) *
           (0.5 + vol);
    fragColor = vec4(col, 1.0);
}
