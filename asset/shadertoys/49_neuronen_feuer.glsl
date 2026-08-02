// 49 Neuronen-Feuer — Patrik Neunteufel (LumiViz, 2026). Eigenwerk.
// Shadertoy: iChannel0 = Music. BIOLOGIE: feuerndes Nervennetz.
//
// IDEE: Ein Voronoi-Netz liefert Zellkerne (Somata) und Zellgrenzen
// (Dendriten-Bahnen). Auf den Grenzlinien laufen Lichtpulse (sin über die
// Randdistanz + Zeit) — das Netz "feuert". Die Höhen erhöhen die Feuerrate.

// ---- STELLSCHRAUBEN --------------------------------------------------------
const float ZOOM        = 3.5;
const float SOMA_GROESSE = 0.06;
const float BAHN_DICKE  = 0.015;
const float PULS_TEMPO  = 3.0;   // Grund-Feuerrate
const float PULS_HOEHEN = 4.0;   // Höhen-Zuschlag
const float WANDERN     = 0.15;  // Kerne driften leicht
// ----------------------------------------------------------------------------

vec2 hash2(vec2 p)
{
    p = vec2(dot(p, vec2(127.1, 311.7)), dot(p, vec2(269.5, 183.3)));
    return fract(sin(p) * 43758.5453);
}
void mainImage(out vec4 fragColor, in vec2 fragCoord)
{
    vec2 uv = (2.0 * fragCoord - iResolution.xy) / iResolution.y * ZOOM;
    float treb = texture(iChannel0, vec2(0.70, 0.25)).x;

    vec2 g = floor(uv);
    vec2 f = fract(uv);
    // Voronoi mit ZWEI kleinsten Distanzen: d2−d1 = Abstand zur ZELLGRENZE
    float d1 = 8.0, d2 = 8.0;
    vec2 id1 = vec2(0.0);
    for (int y = -1; y <= 1; ++y)
    for (int x = -1; x <= 1; ++x)
    {
        vec2 o = vec2(float(x), float(y));
        vec2 h = hash2(g + o);
        vec2 c = o + 0.5 + WANDERN * sin(iTime * (0.3 + h.x) + 6.28318 * h) - f;
        float d = length(c);
        if (d < d1) { d2 = d1; d1 = d; id1 = h; }
        else if (d < d2) { d2 = d; }
    }
    float grenze = d2 - d1;  // 0 auf der Zellgrenze

    vec3 col = vec3(0.01, 0.01, 0.025);
    // Somata: Glühkern nahe dem Zentrum jeder Zelle
    float soma = smoothstep(SOMA_GROESSE, 0.0, d1);
    // Bahnen: schmale Zone um die Grenze
    float bahn = smoothstep(BAHN_DICKE, 0.0, grenze);
    // Feuerpuls: läuft die Bahn entlang (über die Distanz zum Soma)
    float puls = pow(0.5 + 0.5 * sin(d1 * 12.0 - iTime * (PULS_TEMPO +
                                     PULS_HOEHEN * treb) + id1.x * 6.28318), 6.0);
    col += soma * vec3(0.9, 0.6, 1.0);
    col += bahn * vec3(0.12, 0.2, 0.35);              // ruhende Bahn
    col += bahn * puls * vec3(0.4, 0.8, 1.0) * 1.6;   // laufender Impuls
    fragColor = vec4(col, 1.0);
}
