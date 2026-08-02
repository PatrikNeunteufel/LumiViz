// 67 Prisma-Spektrum — Patrik Neunteufel (LumiViz, 2026). Eigenwerk.
// Shadertoy: iChannel0 = Music. KRISTALL/OPTIK: Lichtbrechung mit Dispersion.
//
// IDEE: Ein weißer Strahl trifft ein Dreiecks-Prisma; hinter dem Prisma
// fächert er in Spektralfarben auf (je Wellenlänge ein eigener
// Brechungswinkel — hier als Fächer aus schmalen Glow-Strahlen).
// Der Fächer atmet mit den Mitten, Staubpartikel funkeln im Licht.

// ---- STELLSCHRAUBEN --------------------------------------------------------
const float PRISMA_GROESSE = 0.42;
const int   SPEKTRAL     = 24;   // Strahlen im Fächer
const float FAECHER      = 0.5;  // Öffnungswinkel (rad)
const float FAECHER_MITTEN = 0.25;
const float STRAHL_GLOW  = 0.0015;
const float STAUB        = 0.996; // Partikeldichte (näher 1 = weniger)
// ----------------------------------------------------------------------------

float n21(vec2 p) { return fract(sin(dot(p, vec2(41.3, 289.1))) * 43758.5453); }
float strecke(vec2 p, vec2 a, vec2 b)
{
    vec2 pa = p - a, ba = b - a;
    float h = clamp(dot(pa, ba) / dot(ba, ba), 0.0, 1.0);
    return length(pa - ba * h);
}
void mainImage(out vec4 fragColor, in vec2 fragCoord)
{
    vec2 uv = (2.0 * fragCoord - iResolution.xy) / iResolution.y;
    float mid = texture(iChannel0, vec2(0.30, 0.25)).x;
    vec3 col = vec3(0.008, 0.008, 0.015);

    // Prisma: gleichseitiges Dreieck in der Mitte (Kanten über 3 Halbebenen)
    vec2 p = uv;
    float d = max(p.y - PRISMA_GROESSE * 0.5,
                  max(0.866 * p.x - 0.5 * p.y - PRISMA_GROESSE * 0.5,
                      -0.866 * p.x - 0.5 * p.y - PRISMA_GROESSE * 0.5));
    float glas = smoothstep(0.01, -0.01, d);
    float kante = smoothstep(0.012, 0.0, abs(d));

    // Eingangsstrahl: von links auf die Prisma-Mitte
    vec2 eintritt = vec2(-PRISMA_GROESSE * 0.45, 0.0);
    float strahlEin = smoothstep(0.012, 0.0,
                                 strecke(uv, vec2(-1.8, 0.35), eintritt));
    col += strahlEin * vec3(1.0) * 0.8;

    // Spektralfächer: aus dem Prisma heraus, Winkel je "Wellenlänge"
    vec2 austritt = vec2(PRISMA_GROESSE * 0.35, -0.05);
    float basisWinkel = -0.35;
    float oeffnung = FAECHER + FAECHER_MITTEN * mid;
    for (int i = 0; i < SPEKTRAL; ++i)
    {
        float fi = float(i) / float(SPEKTRAL - 1);          // 0 rot .. 1 violett
        float w = basisWinkel - oeffnung * (fi - 0.5);
        vec2 ziel = austritt + 2.5 * vec2(cos(w), sin(w));
        float ds = strecke(uv, austritt, ziel);
        // Spektralfarbe über die Cosinus-Palette (rot → violett)
        vec3 farbe = 0.5 + 0.5 * cos(6.28318 * (fi * 0.75) + vec3(0.0, 2.1, 4.2));
        col += STRAHL_GLOW / (ds * ds + 0.0008) * farbe * 0.04;
    }
    // Glaskörper: leicht getrübt + helle Kanten
    col = mix(col, col * 0.6 + vec3(0.10, 0.12, 0.16), glas);
    col += kante * vec3(0.7, 0.8, 1.0) * 0.5;
    // Staub im Licht
    float staub = step(STAUB, n21(floor(uv * 160.0) + floor(iTime * 3.0)));
    col += staub * 0.25 * smoothstep(1.4, 0.3, length(uv));
    fragColor = vec4(col, 1.0);
}
