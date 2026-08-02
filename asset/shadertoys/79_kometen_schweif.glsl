// 79 Kometen-Schweif — Patrik Neunteufel (LumiViz, 2026). Eigenwerk.
// Shadertoy: iChannel0 = Music. GLOW: Komet auf elliptischer Bahn.
//
// IDEE: Der Schweif OHNE Buffer: entlang der bekannten Bahn werden die
// POSITIONEN DER VERGANGENHEIT nachgerechnet und als verblassende
// Glühpunkte gezeichnet (je älter, desto schwächer + breiter gestreut).
// Staub- und Ionenschweif trennen sich (Ionen zeigen von der "Sonne" weg).

// ---- STELLSCHRAUBEN --------------------------------------------------------
const int   SPUR_PUNKTE = 48;   // nachgerechnete Vergangenheits-Punkte
const float SPUR_DAUER  = 2.0;  // Sekunden Schweif
const float BAHN_TEMPO  = 0.5;
const float KERN_GROESSE = 0.02;
const float BASS_AUSBRUCH = 0.6;
// ----------------------------------------------------------------------------

vec2 bahn(float t)
{
    // elliptische Bahn um die "Sonne" links oben
    return vec2(-0.5 + 1.1 * cos(t), 0.25 + 0.55 * sin(t * 1.0));
}
float h1(float n) { return fract(sin(n) * 43758.5453); }
void mainImage(out vec4 fragColor, in vec2 fragCoord)
{
    vec2 uv = (2.0 * fragCoord - iResolution.xy) / iResolution.y;
    float bass = texture(iChannel0, vec2(0.05, 0.25)).x;
    vec2 sonne = vec2(-0.9, 0.65);

    vec3 col = vec3(0.006, 0.006, 0.018);
    // Sternchen + "Sonne"
    col += step(0.9975, h1(dot(floor(uv * 70.0), vec2(1.0, 57.0)))) * 0.4;
    col += exp(-dot(uv - sonne, uv - sonne) * 30.0) * vec3(1.0, 0.9, 0.6) * 0.6;

    float jetzt = iTime * BAHN_TEMPO;
    vec2 kern = bahn(jetzt);
    // Schweif: Vergangenheit nachrechnen
    for (int i = 1; i < SPUR_PUNKTE; ++i)
    {
        float fi = float(i) / float(SPUR_PUNKTE);
        float t = jetzt - fi * SPUR_DAUER * BAHN_TEMPO;
        vec2 p = bahn(t);
        float alterFaktor = 1.0 - fi;
        // Ionenschweif: von der Sonne weg versetzt, je älter desto weiter
        vec2 weg = normalize(p - sonne);
        vec2 ion = p + weg * fi * 0.5;
        // Staubschweif: bleibt näher an der Bahn, streut leicht
        vec2 staub = p + weg * fi * 0.2 +
                     0.05 * fi * vec2(h1(fi * 91.0) - 0.5, h1(fi * 57.0) - 0.5);
        float dIon = length(uv - ion);
        float dStaub = length(uv - staub);
        col += exp(-dIon * dIon * 900.0) * vec3(0.3, 0.6, 1.0) *
               alterFaktor * 0.5;
        col += exp(-dStaub * dStaub * 600.0) * vec3(0.9, 0.8, 0.6) *
               alterFaktor * 0.4;
    }
    // Kern + Koma (Bass = Ausbruch: Koma bläht sich)
    float koma = 1.0 + smoothstep(BASS_AUSBRUCH, 1.0, bass) * 1.5;
    float dK = length(uv - kern);
    col += smoothstep(KERN_GROESSE, 0.0, dK) * vec3(1.0);
    col += exp(-dK * dK * 120.0 / koma) * vec3(0.7, 0.85, 1.0) * 0.8;
    fragColor = vec4(col, 1.0);
}
