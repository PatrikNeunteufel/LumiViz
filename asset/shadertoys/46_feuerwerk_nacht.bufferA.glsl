// 46 Feuerwerk-Nacht, Buffer A — Patrik Neunteufel (LumiViz, 2026). Eigenwerk.
// "Buffer A": iChannel0 = Buffer A (SELBST!), iChannel1 = Music.
//
// IDEE: Raketen + Explosionen als reine Mathematik: je Zeitfenster startet
// eine Rakete (Aufstiegsspur), am Kulminationspunkt platzen N Funken
// ballistisch (Anfangsimpuls + Schwerkraft, ausglühend). Der Decay-Trail
// des Buffers zieht alle Spuren nach. Der Bass startet Zusatzraketen.

// ---- STELLSCHRAUBEN --------------------------------------------------------
const float TRAIL       = 0.90;  // Spur-Länge
const float RAKETEN_RATE = 0.5;  // Starts pro Sekunde
const int   FUNKEN      = 40;    // Funken je Explosion
const float SCHWERKRAFT = 0.25;
const float FUNKEN_DAUER = 2.2;  // Sekunden bis zum Verglühen
const float BASS_EXTRA  = 0.5;   // ab dieser Bass-Stärke: Zusatzrakete
// ----------------------------------------------------------------------------

float h1(float n) { return fract(sin(n) * 43758.5453); }
// Ein Feuerwerks-Ereignis (Rakete + Explosion) zur Saat `seed`
vec3 ereignis(vec2 uv, float seed, float lokalZeit)
{
    vec3 col = vec3(0.0);
    vec2 start = vec2(-0.7 + 1.4 * h1(seed * 7.7), -0.55);
    vec2 ziel  = vec2(start.x + 0.3 * (h1(seed * 3.1) - 0.5),
                      0.15 + 0.35 * h1(seed * 9.3));
    float aufstieg = 0.9;  // Sekunden bis zum Knall
    if (lokalZeit < aufstieg)
    {
        // Rakete: Position interpoliert, kleiner heller Kopf
        vec2 p = mix(start, ziel, lokalZeit / aufstieg);
        col += smoothstep(0.006, 0.0, length(uv - p)) * vec3(1.0, 0.8, 0.5);
    }
    else if (lokalZeit < aufstieg + FUNKEN_DAUER)
    {
        float t = lokalZeit - aufstieg;
        float glut = exp(-t * 2.2 / FUNKEN_DAUER * 3.0);
        vec3 farbe = 0.5 + 0.5 * cos(seed * 37.0 + vec3(0.0, 2.1, 4.2));
        for (int i = 0; i < FUNKEN; ++i)
        {
            float fi = float(i);
            float w = 6.28318 * fi / float(FUNKEN) + h1(seed + fi) * 0.3;
            float kraft = 0.25 + 0.2 * h1(seed * 13.0 + fi);
            // Ballistik: Anfangsimpuls · t − ½g·t²
            vec2 p = ziel + vec2(cos(w), sin(w)) * kraft * t -
                     vec2(0.0, 0.5 * SCHWERKRAFT * t * t);
            col += smoothstep(0.004, 0.0, length(uv - p)) * farbe * glut;
        }
    }
    return col;
}
void mainImage(out vec4 fragColor, in vec2 fragCoord)
{
    vec2 uv = (2.0 * fragCoord - iResolution.xy) / iResolution.y;
    float bass = texture(iChannel1, vec2(0.05, 0.25)).x;
    vec3 col = texture(iChannel0, fragCoord / iResolution.xy).rgb * TRAIL;

    // zwei überlappende Ereignis-Fenster (damit immer etwas fliegt)
    for (int k = 0; k < 2; ++k)
    {
        float fenster = iTime * RAKETEN_RATE + float(k) * 0.5;
        col += ereignis(uv, floor(fenster) * 2.0 + float(k), fract(fenster) / RAKETEN_RATE);
    }
    // Bass-Zusatzrakete in einem eigenen, schnelleren Fenster
    if (bass > BASS_EXTRA)
    {
        float fenster = iTime * 1.3;
        col += ereignis(uv, floor(fenster) * 5.0 + 3.7, fract(fenster) / 1.3) *
               vec3(1.2, 1.0, 0.8);
    }
    fragColor = vec4(min(col, vec3(2.5)), 1.0);
}
