// 03 Lissajous-Glühspur, Buffer A — Patrik Neunteufel (LumiViz, 2026). Eigenwerk.
// Shadertoy-Tab "Buffer A": iChannel0 = Buffer A (SELBST = Vorframe!),
//                           iChannel1 = Music.
//
// IDEE: Der Buffer ist das Gedächtnis: Vorframe × Decay + wandernder
// Lichtpunkt = Leuchtspur. Die Bahn ist eine Lissajous-Figur (x/y schwingen
// mit verschiedenen, audio-verstimmten Frequenzen).

// ---- STELLSCHRAUBEN --------------------------------------------------------
const float DECAY        = 0.965; // näher an 1.0 = längere Spur
const float FREQ_X       = 1.3;   // Grundfrequenz x (Verhältnis formt die Figur)
const float FREQ_Y       = 1.9;   // Grundfrequenz y
const float BASS_VERSTIMM = 1.0;  // Bass verstimmt x (Figur kippt)
const float HOEHEN_VERSTIMM = 0.5; // Höhen verstimmen y
const float AMP_X        = 0.40;  // Bahn-Amplitude x
const float AMP_Y        = 0.38;  // Bahn-Amplitude y
const float PUNKT_GROESSE = 0.045;
// ----------------------------------------------------------------------------

void mainImage(out vec4 fragColor, in vec2 fragCoord)
{
    vec2 uv = fragCoord / iResolution.xy;
    vec3 prev = texture(iChannel0, uv).rgb * DECAY;  // der Trail-Mechanismus

    float bass = texture(iChannel1, vec2(0.05, 0.25)).x;
    float treb = texture(iChannel1, vec2(0.70, 0.25)).x;

    vec2 p = 0.5 + vec2(AMP_X * cos(iTime * (FREQ_X + BASS_VERSTIMM * bass)),
                        AMP_Y * sin(iTime * (FREQ_Y + HOEHEN_VERSTIMM * treb)));
    // x mit Seitenverhältnis korrigiert, damit der Punkt rund ist
    float d = length((uv - p) * vec2(iResolution.x / iResolution.y, 1.0));
    vec3 inject = smoothstep(PUNKT_GROESSE, 0.0, d) *
                  (0.6 + 0.4 * cos(iTime + vec3(0.0, 2.1, 4.2)));
    // max statt +: der Punkt überschreibt die Spur (keine Überbelichtung
    // an Kreuzungen)
    fragColor = vec4(max(prev, inject), 1.0);
}
