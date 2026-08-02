// 27 Schwarzes Loch — Patrik Neunteufel (LumiViz, 2026). Eigenwerk.
// Shadertoy: iChannel0 = Music. GRAVITATIONSLINSE (2D-Näherung).
//
// IDEE: Die Bildkoordinaten werden zum Zentrum hin gebogen (Ablenkung
// ~1/Abstand — die Linsen-Näherung), dann wird damit eine wirbelnde
// Akkretionsscheibe (FBM in Polar) gesampelt. Innerhalb des
// Ereignishorizonts bleibt es schwarz; ein Photonenring glüht am Rand.
// Doppler-Färbung: die auf uns zukommende Seite bläulich, die andere rot.

// ---- STELLSCHRAUBEN --------------------------------------------------------
const float HORIZONT      = 0.18;  // Radius des schwarzen Kerns
const float LINSE         = 0.10;  // Stärke der Lichtablenkung
const float SCHEIBEN_TEMPO = 0.8;  // Rotation der Akkretionsscheibe
const float SCHEIBE_INNEN = 0.20;  // Scheibenring innen/außen
const float SCHEIBE_AUSSEN = 0.85;
const float DOPPLER       = 0.5;   // Blau/Rot-Asymmetrie
const float BASS_GLUT     = 0.8;   // Bass heizt die Scheibe
const int   OKTAVEN       = 4;
// ----------------------------------------------------------------------------

float n21(vec2 p) { return fract(sin(dot(p, vec2(41.3, 289.1))) * 43758.5453); }
float vnoise(vec2 p)
{
    vec2 i = floor(p), f = fract(p);
    f = f * f * (3.0 - 2.0 * f);
    return mix(mix(n21(i), n21(i + vec2(1.0, 0.0)), f.x),
               mix(n21(i + vec2(0.0, 1.0)), n21(i + vec2(1.0, 1.0)), f.x), f.y);
}
float fbm(vec2 p)
{
    float v = 0.0, a = 0.5;
    for (int i = 0; i < OKTAVEN; ++i) { v += a * vnoise(p); p *= 2.13; a *= 0.5; }
    return v;
}
void mainImage(out vec4 fragColor, in vec2 fragCoord)
{
    vec2 uv = (2.0 * fragCoord - iResolution.xy) / iResolution.y;
    float bass = texture(iChannel0, vec2(0.05, 0.25)).x;

    float r = length(uv) + 1e-4;
    // Gravitationslinse: Punkte werden zum Zentrum hin verschoben (~1/r)
    vec2 gebogen = uv * (1.0 - LINSE / r);
    float rb = length(gebogen) + 1e-4;
    float ab = atan(gebogen.y, gebogen.x);

    vec3 col = vec3(0.0);
    if (r > HORIZONT)
    {
        // Akkretionsscheibe: FBM entlang der (gebogenen) Polarkoordinaten,
        // differentiell rotierend (innen schneller — wie echte Scheiben)
        float rot = iTime * SCHEIBEN_TEMPO / max(rb, 0.15);
        float dichte = fbm(vec2(ab * 2.0 + rot, rb * 6.0));
        float ring = smoothstep(SCHEIBE_INNEN, SCHEIBE_INNEN + 0.1, rb) *
                     smoothstep(SCHEIBE_AUSSEN, SCHEIBE_AUSSEN - 0.3, rb);
        float glut = dichte * ring * (1.0 + BASS_GLUT * bass);
        // Doppler: linke/rechte Seite verschieden getönt
        float seite = 0.5 + 0.5 * sin(ab);
        vec3 heiss = mix(vec3(1.0, 0.45, 0.15), vec3(0.55, 0.75, 1.0),
                         DOPPLER * seite + (1.0 - DOPPLER) * 0.5);
        col = glut * glut * 2.2 * heiss;
        // Photonenring: schmaler Glühsaum direkt am Horizont
        col += smoothstep(0.03, 0.0, abs(r - HORIZONT * 1.15)) *
               vec3(1.0, 0.8, 0.5) * (0.8 + bass);
        // Hintergrund-Sterne, ebenfalls gelinst
        float stern = step(0.9975, n21(floor(gebogen * 60.0)));
        col += stern * 0.6 * smoothstep(SCHEIBE_INNEN, SCHEIBE_AUSSEN, rb);
    }
    fragColor = vec4(col, 1.0);
}
