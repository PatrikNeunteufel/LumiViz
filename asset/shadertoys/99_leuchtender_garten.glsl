// 99 Leuchtender Garten — Patrik Neunteufel (LumiViz, 2026). Eigenwerk.
// Shadertoy: iChannel0 = Music. KOMBI: Fraktal-Baum × Glühwürmchen ×
// Bio-Lumineszenz — ein nachtleuchtender Garten: gefaltete Leuchtbäume,
// blinkende Käfer dazwischen, pulsierender Boden-Schimmer. Viel Glow.

// ---- STELLSCHRAUBEN --------------------------------------------------------
const int   BAUM_TIEFE = 7;
const float AST_WINKEL = 0.5;
const float WIND       = 0.06;
const int   KAEFER     = 24;
const float BODEN_PULS = 0.8;
// ----------------------------------------------------------------------------

vec2 rot2(vec2 p, float a) { return vec2(p.x * cos(a) - p.y * sin(a),
                                         p.x * sin(a) + p.y * cos(a)); }
float h1(float n) { return fract(sin(n) * 43758.5453); }
// Leucht-Baum an Wurzelposition `wurzel` (kompakte Faltungs-Version)
vec3 baum(vec2 uv, vec2 wurzel, float groesse, float phase, float mid)
{
    vec2 p = (uv - wurzel) / groesse;
    float s = 1.0;
    float laenge = 0.5;
    vec3 glow = vec3(0.0);
    float wind = WIND * sin(iTime * 0.7 + phase) * (1.0 + 2.0 * mid);
    for (int i = 0; i < BAUM_TIEFE; ++i)
    {
        vec2 q = vec2(p.x, p.y - clamp(p.y, 0.0, laenge));
        float d = length(q) / s * groesse;
        float fi = float(i) / float(BAUM_TIEFE - 1);
        // Äste leuchten zur Spitze hin türkis-violett
        vec3 farbe = mix(vec3(0.1, 0.5, 0.35), vec3(0.5, 0.3, 0.9), fi);
        glow += exp(-d * (500.0 - 300.0 * fi)) * farbe * (0.5 + 0.5 * fi);
        p.y -= laenge;
        p.x = abs(p.x);
        p = rot2(p, -(AST_WINKEL + wind));
        p /= 0.7;
        s /= 0.7;
        laenge *= 0.8;
    }
    return glow;
}
void mainImage(out vec4 fragColor, in vec2 fragCoord)
{
    vec2 uv = (2.0 * fragCoord - iResolution.xy) / iResolution.y;
    float mid = texture(iChannel0, vec2(0.30, 0.25)).x;
    float vol = texture(iChannel0, vec2(0.15, 0.25)).x;

    vec3 col = mix(vec3(0.008, 0.012, 0.03), vec3(0.0, 0.004, 0.012),
                   uv.y * 0.5 + 0.5);
    // drei Leuchtbäume verschiedener Größe
    col += baum(uv, vec2(-0.65, -0.75), 0.9, 0.0, mid);
    col += baum(uv, vec2(0.25, -0.8), 1.2, 2.0, mid);
    col += baum(uv, vec2(0.85, -0.7), 0.6, 4.0, mid);
    // Boden-Schimmer: pulsierendes Leucht-Moos
    float boden = smoothstep(-0.65, -0.95, uv.y);
    col += boden * (0.4 + BODEN_PULS * vol * (0.5 + 0.5 * sin(iTime * 2.0 + uv.x * 5.0))) *
           vec3(0.05, 0.25, 0.18);
    // Glühwürmchen
    for (int i = 0; i < KAEFER; ++i)
    {
        float fi = float(i);
        vec2 p = vec2(1.6 * (h1(fi * 3.1) - 0.5) * 2.0 + 0.08 * sin(iTime * 0.5 + fi),
                      -0.6 + 1.0 * h1(fi * 7.7) + 0.05 * sin(iTime * 0.6 + fi * 2.0));
        float blink = pow(0.5 + 0.5 * sin(iTime * (0.5 + h1(fi) * 0.8) * 6.28318 +
                                          fi * 2.7), 10.0);
        float d = length(uv - p);
        col += (smoothstep(0.005, 0.0, d) + exp(-d * 120.0) * 0.5) *
               vec3(0.7, 1.0, 0.3) * blink;
    }
    fragColor = vec4(col, 1.0);
}
