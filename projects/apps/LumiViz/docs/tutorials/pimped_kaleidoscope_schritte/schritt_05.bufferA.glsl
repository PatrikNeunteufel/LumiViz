// ---- LumiViz-Anpassung (NICHT im Tutorial-Text) ----------------------------
// Die Buffer-Ping-Pongs des Shadertoy-Nodes filtern derzeit NEAREST
// (Qt-FBO-Default, MultiEffectVisualizer::runShadertoy) - der bilineare
// Tiefpass, den Schritt 6 als Stabilisator des Sharpen voraussetzt (und den
// Shadertoy-Buffer mitbringen), fehlt damit: das System kocht in Pixelgriess
// hoch (exakt die "Probe aufs Exempel" aus Schritt 6). lesBilinear() stellt
// die Shadertoy-Lesesemantik im Shader selbst her. Auf shadertoy.com ist
// diese Funktion UNNOETIG - dort steht im Tutorial schlicht texture().
vec3 lesBilinear(vec2 st)
{
    vec2 p  = st * iResolution.xy - 0.5;
    vec2 i  = floor(p);
    vec2 fr = p - i;
    vec2 px = 1.0 / iResolution.xy;
    vec2 b  = (i + 0.5) * px;
    vec3 c00 = texture(iChannel0, b).rgb;
    vec3 c10 = texture(iChannel0, b + vec2(px.x, 0.0)).rgb;
    vec3 c01 = texture(iChannel0, b + vec2(0.0, px.y)).rgb;
    vec3 c11 = texture(iChannel0, b + px).rgb;
    return mix(mix(c00, c10, fr.x), mix(c01, c11, fr.x), fr.y);
}
// ---- Ende LumiViz-Anpassung ------------------------------------------------

// ---- STELLSCHRAUBEN --------------------------------------------------------
const float SEED_HELL  = 1.0;    // Gesamthelligkeit der Lichtsaat
const float BAHN_WEITE = 0.35;   // Radius der Lissajous-Bahn
const float DECAY      = 0.90;   // Daempfung je Frame (< 1!)
const float ZOOM       = 1.010;  // NEU: > 1 = Inhalt waechst nach aussen
const float DREH       = 0.006;  // NEU: Drehung in Radiant PRO FRAME
// ----------------------------------------------------------------------------

#define R(a) mat2(cos(a), sin(a), -sin(a), cos(a))

// NEU: zentrierte Koordinaten -> 0..1-Texturkoordinaten (fuers Sampling)
vec2 uvZuTex(vec2 uv)
{
    return uv * vec2(iResolution.y / iResolution.x, 1.0) + 0.5;
}

vec3 palette(float t)
{
    return 0.55 + 0.45 * cos(6.28318 * (t + vec3(0.0, 0.33, 0.67)));
}

vec3 seeds(vec2 uv)
{
    vec3 acc = vec3(0.0);

    vec2 pos = vec2(sin(iTime * 0.31), sin(iTime * 0.23))
             * BAHN_WEITE * vec2(1.0, 0.7);
    vec3 farbe = palette(iTime * 0.021);

    vec2 d1 = uv - pos;
    vec2 d2 = uv + pos;
    acc += farbe     * 0.0006 / (0.0004 + dot(d1, d1));
    acc += farbe.bgr * 0.0006 / (0.0004 + dot(d2, d2));

    float r  = 0.26 + 0.10 * sin(iTime * 0.171);
    float dr = length(uv) - r;
    acc += palette(iTime * 0.021 + 0.5) * 0.0012 / (0.0008 + 8.0 * dr * dr);

    return acc * SEED_HELL;
}

void mainImage(out vec4 fragColor, in vec2 fragCoord)
{
    vec2 uv = (fragCoord - 0.5 * iResolution.xy) / iResolution.y;

    // GEAENDERT - Lese-UV: WO im Vorframe schaut dieser Pixel nach?
    // Teilen durch ZOOM liest naeher an der Mitte -> der Inhalt
    // erscheint jeden Frame ein Stueck weiter AUSSEN.
    vec2 lese = R(-DREH) * (uv / ZOOM);

    vec3 alt = lesBilinear(uvZuTex(lese));

    vec3 neu = alt * DECAY + seeds(uv);
    fragColor = vec4(neu, 1.0);
}
