// 32 Seifenblase — Patrik Neunteufel (LumiViz, 2026). Eigenwerk.
// Shadertoy: iChannel0 = Music. RAYMARCHING + DÜNNSCHICHT-INTERFERENZ.
//
// IDEE: Eine Kugel wird geraymarcht; die Farbe kommt aus der Physik der
// Seifenhaut: die Schichtdicke variiert (FBM + Schwerkraft-Ablauf), und
// cos(Dicke / Wellenlänge) je RGB-Kanal ergibt die Interferenzfarben.
// Fresnel macht den Rand spiegelnd. Der Bass beult die Blase.

// ---- STELLSCHRAUBEN --------------------------------------------------------
const float RADIUS      = 0.85;
const float BEULE_BASS  = 0.10;  // Bass-Verformung
const float WOBBEL      = 0.04;  // Eigen-Wabern der Haut
const float SCHICHT_LAUF = 0.35; // wie schnell die Farben über die Haut ziehen
const float INTERFERENZ = 18.0;  // Farbzyklen über die Dicke (mehr = bunter)
const int   MARSCH      = 64;
// ----------------------------------------------------------------------------

float n21(vec2 p) { return fract(sin(dot(p, vec2(41.3, 289.1))) * 43758.5453); }
float vnoise(vec2 p)
{
    vec2 i = floor(p), f = fract(p);
    f = f * f * (3.0 - 2.0 * f);
    return mix(mix(n21(i), n21(i + vec2(1.0, 0.0)), f.x),
               mix(n21(i + vec2(0.0, 1.0)), n21(i + vec2(1.0, 1.0)), f.x), f.y);
}
float g_bass;
float blase(vec3 p)
{
    // Kugel mit Bass-Beule + leichtem Noise-Wabern auf der Oberfläche
    float r = RADIUS + BEULE_BASS * g_bass * sin(p.y * 4.0 + iTime * 2.0) +
              WOBBEL * (vnoise(p.xy * 3.0 + iTime * 0.5) - 0.5);
    return length(p) - r;
}
void mainImage(out vec4 fragColor, in vec2 fragCoord)
{
    vec2 uv = (2.0 * fragCoord - iResolution.xy) / iResolution.y;
    g_bass = texture(iChannel0, vec2(0.05, 0.25)).x;

    vec3 ro = vec3(0.0, 0.0, -2.4);
    vec3 rd = normalize(vec3(uv, 1.5));

    float t = 0.0;
    float d = 1.0;
    for (int i = 0; i < MARSCH; ++i)
    {
        d = blase(ro + rd * t);
        if (d < 0.001 || t > 5.0) break;
        t += d;
    }
    // Hintergrund: sanfter Studioverlauf
    vec3 col = mix(vec3(0.03, 0.03, 0.06), vec3(0.10, 0.08, 0.14),
                   uv.y * 0.5 + 0.5);
    if (d < 0.001)
    {
        vec3 p = ro + rd * t;
        vec3 n = normalize(p);  // Kugel: Normale = Richtung
        // Schichtdicke: oben dünn, unten dick (Ablauf) + FBM-Schlieren,
        // die über die Haut ziehen
        float dicke = 1.2 - n.y * 0.8 +
                      0.8 * vnoise(vec2(atan(n.z, n.x) * 2.0,
                                        n.y * 3.0 - iTime * SCHICHT_LAUF));
        // Interferenz: je Kanal eine andere effektive Wellenlänge
        vec3 film = 0.5 + 0.5 * cos(dicke * INTERFERENZ * vec3(1.0, 1.13, 1.28));
        float fresnel = pow(1.0 - max(dot(n, -rd), 0.0), 3.0);
        // Haut = überwiegend durchsichtig; Farben + Rand-Spiegel
        col = mix(col, film, 0.35 + 0.5 * fresnel);
        col += fresnel * vec3(0.6, 0.7, 0.9) * 0.5;  // Randglanz
        // Glanzpunkt der "Studioleuchte"
        vec3 l = normalize(vec3(0.6, 0.8, -0.5));
        col += pow(max(dot(reflect(-l, n), -rd), 0.0), 64.0) * vec3(1.0);
    }
    fragColor = vec4(col, 1.0);
}
