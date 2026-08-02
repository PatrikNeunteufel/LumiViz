// 23 Nebel-Galaxie — Patrik Neunteufel (LumiViz, 2026). Eigenwerk.
// Shadertoy: iChannel0 = Music. FBM-VOLUMETRIK (2D-Fake, mehrschichtig).
//
// IDEE: Eine Spiralgalaxie: die Winkelkoordinate wird mit log(r) verdreht
// (logarithmische Spirale), darüber liegt mehrschichtiges FBM als Nebel;
// zwei Sternebenen parallaxen. Bass = Kernglühen, Höhen = Sternfunkeln.

// ---- STELLSCHRAUBEN --------------------------------------------------------
const float SPIRAL_ARME   = 2.0;   // Anzahl Spiralarme
const float SPIRAL_ENGE   = 3.5;   // wie eng die Arme gewickelt sind
const float DREH_TEMPO    = 0.05;  // Rotation der Galaxie
const int   NEBEL_OKTAVEN = 5;     // FBM-Detail
const float KERN_GLUT     = 1.2;   // Helligkeit des Zentrums
const float STERN_DICHTE  = 0.997; // näher an 1 = weniger Sterne
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
    for (int i = 0; i < NEBEL_OKTAVEN; ++i) { v += a * vnoise(p); p *= 2.07; a *= 0.5; }
    return v;
}
void mainImage(out vec4 fragColor, in vec2 fragCoord)
{
    vec2 uv = (2.0 * fragCoord - iResolution.xy) / iResolution.y;
    float bass = texture(iChannel0, vec2(0.05, 0.25)).x;
    float treb = texture(iChannel0, vec2(0.70, 0.25)).x;

    float r = length(uv) + 1e-4;
    float a = atan(uv.y, uv.x) - iTime * DREH_TEMPO;
    // logarithmische Spirale: Winkel + Enge·log(r) — DAS macht die Arme
    float arm = a * SPIRAL_ARME + SPIRAL_ENGE * log(r);
    float armDichte = 0.5 + 0.5 * cos(arm);

    // Nebel: FBM in spiralverzerrten Koordinaten (folgt den Armen)
    vec2 np = vec2(arm * 0.6, r * 4.0 - iTime * 0.05);
    float nebel = fbm(np) * armDichte;
    nebel *= smoothstep(1.4, 0.2, r);  // außen ausblenden

    // Farbmischung: Kern warm, Arme blau-violett
    vec3 kern = vec3(1.0, 0.85, 0.6) * (KERN_GLUT + 0.8 * bass) *
                exp(-r * 4.5);
    vec3 arme = mix(vec3(0.15, 0.25, 0.6), vec3(0.6, 0.3, 0.7),
                    fbm(np + 7.0)) * nebel * 1.4;
    vec3 col = kern + arme;

    // zwei Sternebenen (verschiedene Raster = Parallaxe beim Drehen)
    for (int layer = 0; layer < 2; ++layer)
    {
        vec2 sp = uv * (3.0 + 2.0 * float(layer));
        float stern = step(STERN_DICHTE, n21(floor(sp * 40.0) + float(layer) * 13.0));
        // Funkeln: Zufallsphase je Stern, Höhen verstärken
        float blink = 0.5 + 0.5 * sin(iTime * 3.0 + n21(floor(sp * 40.0)) * 40.0);
        col += stern * blink * (0.25 + 0.75 * treb);
    }
    fragColor = vec4(col, 1.0);
}
