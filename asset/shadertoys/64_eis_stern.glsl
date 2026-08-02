// 64 Eis-Stern — Patrik Neunteufel (LumiViz, 2026). Eigenwerk.
// Shadertoy: iChannel0 = Music. KRISTALL: wachsende Schneeflocke.
//
// IDEE: 6-fach-Symmetrie durch Winkelfaltung; im gefalteten Keil wachsen
// Hauptast + Seitenäste (Strecken-SDFs), deren LÄNGE mit der Zeit
// zyklisch wächst — die Flocke friert vor, schmilzt, friert neu.
// Schimmer: ein Lichtband läuft radial über das Eis. Höhen = Glitzern.

// ---- STELLSCHRAUBEN --------------------------------------------------------
const float WACHS_DAUER = 8.0;   // Sekunden je Wachstumszyklus
const int   SEITENAESTE = 4;
const float AST_WINKEL  = 0.9;   // Winkel der Seitenäste
const float DICKE       = 0.010;
const float SCHIMMER_TEMPO = 1.5;
const float GLITZER_HOEHEN = 1.0;
// ----------------------------------------------------------------------------

vec2 rot2(vec2 p, float a) { return vec2(p.x * cos(a) - p.y * sin(a),
                                         p.x * sin(a) + p.y * cos(a)); }
float strecke(vec2 p, vec2 a, vec2 b)
{
    vec2 pa = p - a, ba = b - a;
    float h = clamp(dot(pa, ba) / dot(ba, ba), 0.0, 1.0);
    return length(pa - ba * h);
}
float n21(vec2 p) { return fract(sin(dot(p, vec2(41.3, 289.1))) * 43758.5453); }
void mainImage(out vec4 fragColor, in vec2 fragCoord)
{
    vec2 uv = (2.0 * fragCoord - iResolution.xy) / iResolution.y;
    float treb = texture(iChannel0, vec2(0.70, 0.25)).x;
    float wuchs = smoothstep(0.0, 0.7, fract(iTime / WACHS_DAUER)) *
                  smoothstep(1.0, 0.85, fract(iTime / WACHS_DAUER));  // wachsen+schmelzen

    // 6-fach falten (+ Spiegel): alles landet in einem 30°-Keil
    float a = atan(uv.y, uv.x);
    float r = length(uv);
    a = abs(mod(a, 1.0472) - 0.5236);
    vec2 p = vec2(cos(a), sin(a)) * r;

    // Hauptast + Seitenäste (Länge über `wuchs` animiert)
    float haupt = 0.85 * wuchs;
    float d = strecke(p, vec2(0.0), vec2(haupt, 0.0));
    for (int i = 1; i <= SEITENAESTE; ++i)
    {
        float basis = haupt * float(i) / float(SEITENAESTE + 1);
        float laenge = 0.28 * wuchs * (1.0 - basis / max(haupt, 1e-3) * 0.7);
        vec2 wurzel = vec2(basis, 0.0);
        vec2 spitze = wurzel + laenge * vec2(cos(AST_WINKEL), sin(AST_WINKEL));
        d = min(d, strecke(p, wurzel, spitze));
    }
    float eis = smoothstep(DICKE, DICKE * 0.3, d);
    float halo = smoothstep(DICKE * 6.0, 0.0, d) * 0.3;

    // Schimmer: radiales Lichtband + Glitzerpunkte auf dem Eis
    float band = pow(0.5 + 0.5 * sin(r * 14.0 - iTime * SCHIMMER_TEMPO), 5.0);
    float glitzer = step(0.985, n21(floor(uv * 90.0) + floor(iTime * 6.0))) *
                    eis * GLITZER_HOEHEN * treb;
    vec3 col = vec3(0.01, 0.02, 0.05);
    col += (eis * (0.6 + 0.4 * band) + halo) * vec3(0.55, 0.8, 1.0);
    col += glitzer * vec3(1.0);
    fragColor = vec4(col, 1.0);
}
