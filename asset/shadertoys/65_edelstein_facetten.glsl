// 65 Edelstein-Facetten — Patrik Neunteufel (LumiViz, 2026). Eigenwerk.
// Shadertoy: iChannel0 = Music. KRISTALL: geschliffener Stein mit Blitzen.
//
// IDEE: Voronoi-Zellen als FACETTEN: jede Zelle bekommt eine Pseudo-Normale
// aus ihrer Kern-Id; ein rotierendes "Licht" lässt Facette für Facette
// aufblitzen (spekularer cos-Peak). Innerhalb einer Stein-Silhouette,
// tiefes Rot mit weißen Blitzen. Der Bass dreht das Licht schneller.

// ---- STELLSCHRAUBEN --------------------------------------------------------
const float FACETTEN    = 4.5;   // Facettendichte
const float LICHT_TEMPO = 0.6;
const float LICHT_BASS  = 1.2;
const float BLITZ_SCHAERFE = 24.0; // Glanz-Exponent (größer = härtere Blitze)
const float STEIN_GROESSE = 0.75;
// ----------------------------------------------------------------------------

vec2 hash2(vec2 p)
{
    p = vec2(dot(p, vec2(127.1, 311.7)), dot(p, vec2(269.5, 183.3)));
    return fract(sin(p) * 43758.5453);
}
void mainImage(out vec4 fragColor, in vec2 fragCoord)
{
    vec2 uv = (2.0 * fragCoord - iResolution.xy) / iResolution.y;
    float bass = texture(iChannel0, vec2(0.05, 0.25)).x;

    // Stein-Silhouette: Achteck (Smaragd-Schliff) über gefaltete Achsen
    vec2 q = abs(uv);
    float acht = max(max(q.x, q.y), (q.x + q.y) * 0.7071);
    float stein = smoothstep(STEIN_GROESSE + 0.01, STEIN_GROESSE - 0.01, acht);

    // Facetten: Voronoi-Kern-Id im Stein
    vec2 g = floor(uv * FACETTEN);
    vec2 f = fract(uv * FACETTEN);
    float dMin = 8.0;
    vec2 idMin = vec2(0.0);
    for (int y = -1; y <= 1; ++y)
    for (int x = -1; x <= 1; ++x)
    {
        vec2 o = vec2(float(x), float(y));
        vec2 h = hash2(g + o);
        vec2 c = o + h - f;  // statische Kerne = starre Facetten
        float d = dot(c, c);
        if (d < dMin) { dMin = d; idMin = h; }
    }
    // Pseudo-Normale der Facette aus ihrer Id; Lichtwinkel rotiert
    float normalWinkel = idMin.x * 6.28318;
    float licht = iTime * (LICHT_TEMPO + LICHT_BASS * bass);
    float blitz = pow(0.5 + 0.5 * cos(normalWinkel - licht), BLITZ_SCHAERFE);
    // Facetten-Grundton: tiefes Rubinrot, leicht je Facette variiert
    vec3 rubin = vec3(0.5, 0.02, 0.08) * (0.6 + 0.8 * idMin.y);
    // Kanten zwischen Facetten dunkel (Schliffkanten)
    float kante = smoothstep(0.0, 0.02, sqrt(dMin));
    vec3 col = stein * (rubin * kante + blitz * vec3(1.0, 0.9, 0.95));
    // Untergrund: Samt mit Lichtkegel
    col += (1.0 - stein) * vec3(0.03, 0.01, 0.02) * (1.3 - length(uv));
    fragColor = vec4(col, 1.0);
}
