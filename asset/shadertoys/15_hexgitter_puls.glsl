// 15 Hexgitter-Puls — Patrik Neunteufel (LumiViz, 2026). Eigenwerk.
// Shadertoy: iChannel0 = Music (512x2: y=0.25 FFT, y=0.75 Waveform).
//
// IDEE: Hex-Gitter; jede Zelle hört auf "ihren" FFT-Bin (Hash wählt ihn)
// und glüht danach; dazu ein pulsierender Ring je Zelle. Gitter driftet.

// ---- STELLSCHRAUBEN --------------------------------------------------------
const float ZOOM        = 4.0;   // Zellgröße (mehr = kleinere Zellen)
const vec2  DRIFT       = vec2(0.25, 0.15); // Gitter-Drift (Richtung × Tempo)
const float HELL_GRUND  = 0.08;  // Zell-Grundhelligkeit
const float HELL_AUDIO  = 1.4;   // Audio-Aufhellung
const float RING_RADIUS = 0.32;  // Ring-Grundradius
const float RING_PULS   = 0.15;  // Puls-Amplitude des Rings
const float RING_DICKE  = 0.06;
const float FUGE        = 0.03;  // Fugenbreite zwischen den Zellen
// ----------------------------------------------------------------------------

float hexDist(vec2 p)
{
    // Abstand im Hex-Metrik: max aus Kantennormale und x
    p = abs(p);
    return max(dot(p, vec2(0.8660254, 0.5)), p.x);
}
vec4 hexCoords(vec2 p)
{
    // zwei versetzte Rechteck-Gitter; das nähere gewinnt.
    // xy = Vektor zur Zellmitte, zw = Zell-Id
    vec2 r = vec2(1.0, 1.7320508);
    vec2 h = r * 0.5;
    vec2 a = mod(p, r) - h;
    vec2 b = mod(p - h, r) - h;
    vec2 g = dot(a, a) < dot(b, b) ? a : b;
    return vec4(g, p - g);
}
float n21(vec2 p) { return fract(sin(dot(p, vec2(41.3, 289.1))) * 43758.5453); }
void mainImage(out vec4 fragColor, in vec2 fragCoord)
{
    vec2 uv = (2.0 * fragCoord - iResolution.xy) / iResolution.y * ZOOM;
    uv += iTime * DRIFT;  // Bewegung auch ohne Audio
    vec4 h = hexCoords(uv);
    float rnd = n21(h.zw);  // stabiler Zufall je Zelle
    float fft = texture(iChannel0, vec2(0.03 + 0.65 * rnd, 0.25)).x;
    float d = hexDist(h.xy);
    float cell = smoothstep(0.50, 0.50 - FUGE, d);  // Zellfläche mit Fuge
    float ring = smoothstep(RING_DICKE, 0.0,
                            abs(d - (RING_RADIUS + RING_PULS *
                                     sin(iTime + rnd * 6.28318))));
    vec3 base = 0.5 + 0.5 * cos(rnd * 6.28318 + iTime * 0.4 + vec3(0.0, 2.1, 4.2));
    vec3 col = cell * base * (HELL_GRUND + HELL_AUDIO * fft) + ring * base * 0.35;
    fragColor = vec4(col, 1.0);
}
