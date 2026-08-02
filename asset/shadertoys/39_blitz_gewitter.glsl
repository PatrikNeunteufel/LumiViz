// 39 Blitz-Gewitter — Patrik Neunteufel (LumiViz, 2026). Eigenwerk.
// Shadertoy: iChannel0 = Music. PROZEDURALE BLITZE über Wolken-FBM.
//
// IDEE: Ein Blitz ist eine vertikale Linie, deren x-Verlauf über die Höhe
// per FBM verbogen wird (Zickzack); 1/d²-Glow macht ihn gleißend. Die
// Blitz-Saat wechselt in Zeitfenstern — jeder Schlag sieht anders aus.
// Der Bass triggert die Sichtbarkeit (Gewitter im Takt), Wolken flackern mit.

// ---- STELLSCHRAUBEN --------------------------------------------------------
const float SCHLAG_RATE  = 1.2;   // Zeitfenster pro Sekunde (Saat-Wechsel)
const float BASS_TRIGGER = 0.35;  // ab dieser Bass-Stärke zündet der Blitz voll
const float ZACKEN       = 0.35;  // Zickzack-Amplitude
const float ZACKEN_FEIN  = 3.0;   // Zickzack-Frequenz
const float GLOW         = 0.0025; // Blitz-Glühen (größer = breiter)
const float VERZWEIGUNG  = 0.5;   // Stärke des Neben-Asts
const int   OKTAVEN      = 4;
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
    for (int i = 0; i < OKTAVEN; ++i) { v += a * vnoise(p); p *= 2.19; a *= 0.5; }
    return v;
}
// Abstand zum Blitzpfad: x(y) = FBM-verbogene Senkrechte zur Saat `seed`
float blitz(vec2 uv, float seed, float staerke)
{
    float pfadX = (n21(vec2(seed, 3.7)) - 0.5) * 1.2;  // Einschlags-Spur
    float zick = (fbm(vec2(uv.y * ZACKEN_FEIN + seed * 91.0, seed * 17.0)) - 0.5) *
                 ZACKEN * (1.0 - uv.y * 0.3);
    float d = abs(uv.x - pfadX - zick);
    return staerke * GLOW / (d * d + 0.0004);
}
void mainImage(out vec4 fragColor, in vec2 fragCoord)
{
    vec2 uv = (2.0 * fragCoord - iResolution.xy) / iResolution.y;
    vec2 p01 = fragCoord / iResolution.xy;
    float bass = texture(iChannel0, vec2(0.05, 0.25)).x;

    // Zeitfenster: floor(iTime·Rate) = Saat des aktuellen Schlags;
    // fract() = Alter des Schlags (frisch = hell, dann verlöschen)
    float fenster = iTime * SCHLAG_RATE;
    float seed = floor(fenster);
    float alter = fract(fenster);
    float zuend = smoothstep(BASS_TRIGGER, BASS_TRIGGER + 0.2, bass);
    float staerke = zuend * exp(-alter * 6.0);  // schneller Abfall nach dem Schlag

    // Wolkendecke: dunkles FBM, vom Blitz von innen angeleuchtet
    float wolken = fbm(p01 * vec2(3.0, 1.5) + vec2(iTime * 0.05, 0.0));
    vec3 col = mix(vec3(0.02, 0.02, 0.05), vec3(0.10, 0.10, 0.16), wolken);
    col += wolken * staerke * vec3(0.25, 0.28, 0.4);  // Wolken-Flackern

    // Hauptblitz + schwächerer Nebenast (andere Saat, versetzt)
    float b = blitz(uv, seed, staerke);
    b += VERZWEIGUNG * blitz(uv + vec2(0.05, 0.0), seed + 0.31, staerke);
    // nur unterhalb der Wolkenkante sichtbar
    b *= smoothstep(0.9, 0.4, p01.y);
    col += b * vec3(0.75, 0.8, 1.0);
    // Boden-Reflex beim Schlag
    col += staerke * smoothstep(0.15, 0.0, p01.y) * vec3(0.1, 0.12, 0.2);
    fragColor = vec4(col, 1.0);
}
