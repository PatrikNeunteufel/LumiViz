// 54 Newton-Bassin — Patrik Neunteufel (LumiViz, 2026). Eigenwerk.
// Shadertoy: iChannel0 = Music. FRAKTAL: Newton-Verfahren für z³ = 1.
//
// IDEE: Von jedem Pixel aus läuft das Newton-Verfahren; welche der drei
// Wurzeln es findet, färbt das Becken — die Grenzen sind fraktal verwoben.
// Die Wurzeln selbst ROTIEREN langsam (Polynome mit gedrehten Wurzeln),
// die Konvergenz-Geschwindigkeit schattiert. Mitten drehen schneller.

// ---- STELLSCHRAUBEN --------------------------------------------------------
const int   SCHRITTE   = 32;
const float DREH_TEMPO = 0.10;
const float DREH_MITTEN = 0.25;
const float ZOOM       = 1.6;
const float SCHATTEN   = 0.10;   // Abdunklung pro benötigtem Schritt
// ----------------------------------------------------------------------------

vec2 cmul(vec2 a, vec2 b) { return vec2(a.x * b.x - a.y * b.y, a.x * b.y + a.y * b.x); }
vec2 cdiv(vec2 a, vec2 b) { return cmul(a, vec2(b.x, -b.y)) / max(dot(b, b), 1e-9); }
void mainImage(out vec4 fragColor, in vec2 fragCoord)
{
    vec2 uv = (2.0 * fragCoord - iResolution.xy) / iResolution.y * ZOOM;
    float mid = texture(iChannel0, vec2(0.30, 0.25)).x;
    float phi = iTime * (DREH_TEMPO + DREH_MITTEN * mid);

    // die drei rotierenden Wurzeln von z³ = e^{3iφ}
    vec2 w0 = vec2(cos(phi), sin(phi));
    vec2 w1 = vec2(cos(phi + 2.0944), sin(phi + 2.0944));
    vec2 w2 = vec2(cos(phi + 4.1888), sin(phi + 4.1888));

    vec2 z = uv;
    float schritte = 0.0;
    for (int i = 0; i < SCHRITTE; ++i)
    {
        // Newton für p(z) = (z−w0)(z−w1)(z−w2):
        // z -= p / p'  mit  p'/p = Σ 1/(z−wk)  →  z -= 1 / Σ 1/(z−wk)
        vec2 s = cdiv(vec2(1.0, 0.0), z - w0) + cdiv(vec2(1.0, 0.0), z - w1) +
                 cdiv(vec2(1.0, 0.0), z - w2);
        vec2 dz = cdiv(vec2(1.0, 0.0), s);
        z -= dz;
        schritte += 1.0;
        if (dot(dz, dz) < 1e-6) break;
    }
    // welche Wurzel? → Farbbecken
    float d0 = dot(z - w0, z - w0);
    float d1 = dot(z - w1, z - w1);
    float d2 = dot(z - w2, z - w2);
    vec3 col = (d0 < d1 && d0 < d2) ? vec3(0.9, 0.35, 0.25)
             : (d1 < d2)            ? vec3(0.25, 0.75, 0.9)
                                    : vec3(0.55, 0.9, 0.35);
    // Konvergenz-Schatten: fraktale Grenzen werden dunkel
    col *= exp(-schritte * SCHATTEN);
    fragColor = vec4(col, 1.0);
}
