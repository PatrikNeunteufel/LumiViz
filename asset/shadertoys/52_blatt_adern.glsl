// 52 Blatt-Adern — Patrik Neunteufel (LumiViz, 2026). Eigenwerk.
// Shadertoy: iChannel0 = Music. BIOLOGIE × FRAKTAL: durchleuchtetes Blatt.
//
// IDEE: Ein Adernetz aus gespiegelten, skalierten Faltungen (KIFS light,
// gegen die Mittelrippe gefaltet) auf einer Blattform (zwei Kreisbögen).
// Gegenlicht: das Blattgrün leuchtet, die Adern bleiben dunkler und ein
// Lichtfleck wandert (Sonne durchs Laub). Mitten = Blattzittern.

// ---- STELLSCHRAUBEN --------------------------------------------------------
const int   ADER_TIEFE  = 6;
const float ADER_WINKEL = 0.85;  // Abzweig-Winkel der Seitenadern
const float ADER_DICKE  = 0.008;
const float ZITTERN     = 0.02;  // Mitten-Zittern
const float SONNE_TEMPO = 0.3;   // wandernder Lichtfleck
// ----------------------------------------------------------------------------

vec2 rot2(vec2 p, float a) { return vec2(p.x * cos(a) - p.y * sin(a),
                                         p.x * sin(a) + p.y * cos(a)); }
void mainImage(out vec4 fragColor, in vec2 fragCoord)
{
    vec2 uv = (2.0 * fragCoord - iResolution.xy) / iResolution.y;
    float mid = texture(iChannel0, vec2(0.30, 0.25)).x;
    uv = rot2(uv, ZITTERN * mid * sin(iTime * 7.0));  // Blatt zittert im Takt
    uv.y += 0.1;

    // Blattform: Schnitt zweier verschobener Kreise (Lanzett-Form)
    float blatt = max(length(uv - vec2(0.45, 0.0)) - 1.0,
                      length(uv + vec2(0.45, 0.0)) - 1.0);
    float drin = smoothstep(0.01, -0.01, blatt);

    // Adern: gegen die Mittelrippe falten und wiederholt abzweigen
    vec2 p = vec2(uv.x, abs(uv.y));  // Spiegel an der Mittelrippe
    float dMin = abs(uv.y);          // Mittelrippe selbst
    float s = 1.0;
    for (int i = 0; i < ADER_TIEFE; ++i)
    {
        p = rot2(p - vec2(0.22, 0.0), -ADER_WINKEL);  // zur Seitenader drehen
        p.y = abs(p.y);                                // nächste Abzweigung
        s *= 1.35;
        p *= 1.35;
        dMin = min(dMin, abs(p.y) / s);
    }
    float ader = smoothstep(ADER_DICKE, ADER_DICKE * 0.3, dMin);

    // Gegenlicht: Blattgrün + wandernder Sonnenfleck
    vec2 sonne = 0.5 * vec2(cos(iTime * SONNE_TEMPO), sin(iTime * SONNE_TEMPO * 0.7));
    float licht = exp(-dot(uv - sonne, uv - sonne) * 3.0);
    vec3 gruen = mix(vec3(0.1, 0.35, 0.05), vec3(0.5, 0.9, 0.2), licht);
    vec3 col = drin * (gruen * (1.0 - 0.55 * ader));   // Adern dunkler
    col += drin * ader * licht * vec3(0.7, 0.9, 0.3) * 0.3;  // Adern im Licht
    col += (1.0 - drin) * vec3(0.02, 0.025, 0.04);     // Hintergrund
    fragColor = vec4(col, 1.0);
}
