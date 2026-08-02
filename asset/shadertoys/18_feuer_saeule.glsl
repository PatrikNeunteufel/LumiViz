// 18 Feuer-Säule — Patrik Neunteufel (LumiViz, 2026). Eigenwerk.
// Shadertoy: iChannel0 = Music (512x2: y=0.25 FFT, y=0.75 Waveform).
//
// IDEE: FBM-Rauschen nach oben gescrollt = Flammenzug; Säulenform durch
// "unten stark, Mitte breit". Feuer-Rampe: R=f, G=f², B=f³.

// ---- STELLSCHRAUBEN --------------------------------------------------------
const float ZUG_GRUND    = 1.2;  // Zuggeschwindigkeit nach oben
const float ZUG_VOL      = 1.0;  //  … Lautstärke-Zuschlag
const float ZUNGEN_BREITE = 3.0; // horizontale Noise-Frequenz
const float FLAMMEN_HOEHE = 1.3; // größer = höhere Flammen
const float SAEULE_BREITE = 2.6; // kleiner = breitere Säule
const float BREITE_VOL   = 1.2;  // Lautstärke verbreitert die Säule
const int   OKTAVEN      = 5;    // FBM-Detail (mehr = feiner, teurer)
// ----------------------------------------------------------------------------

float n21(vec2 p) { return fract(sin(dot(p, vec2(41.3, 289.1))) * 43758.5453); }
float vnoise(vec2 p)
{
    vec2 i = floor(p);
    vec2 f = fract(p);
    f = f * f * (3.0 - 2.0 * f);
    return mix(mix(n21(i), n21(i + vec2(1.0, 0.0)), f.x),
               mix(n21(i + vec2(0.0, 1.0)), n21(i + vec2(1.0, 1.0)), f.x), f.y);
}
// FBM: Oktaven mit halber Amplitude, doppelter Frequenz
float fbm(vec2 p)
{
    float v = 0.0;
    float a = 0.5;
    for (int i = 0; i < OKTAVEN; ++i)
    {
        v += a * vnoise(p);
        p *= 2.03;  // ungerader Faktor vermeidet Gitterartefakte
        a *= 0.5;
    }
    return v;
}
void mainImage(out vec4 fragColor, in vec2 fragCoord)
{
    vec2 uv = fragCoord / iResolution.xy;
    float vol = texture(iChannel0, vec2(0.10, 0.25)).x;
    // Noise-y wandert mit der Zeit = Flammen ziehen hoch
    vec2 p = vec2(uv.x * ZUNGEN_BREITE, uv.y * 2.0 - iTime * (ZUG_GRUND + ZUG_VOL * vol));
    float flame = fbm(p) * (FLAMMEN_HOEHE - uv.y);            // oben ausdünnen
    float center = 1.0 - abs(uv.x - 0.5) * (SAEULE_BREITE - BREITE_VOL * vol);
    float f = clamp(flame * center * (1.1 + 1.3 * vol), 0.0, 1.0);
    // Feuer-Rampe: f / f² / f³ = Glut-Farbstufen
    vec3 col = vec3(f * 1.6, f * f * 1.1, f * f * f * 0.7);
    fragColor = vec4(col, 1.0);
}
