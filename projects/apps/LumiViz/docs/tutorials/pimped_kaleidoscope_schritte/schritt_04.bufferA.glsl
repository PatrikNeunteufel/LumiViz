// ---- STELLSCHRAUBEN --------------------------------------------------------
const float SEED_HELL  = 1.0;    // Gesamthelligkeit der Lichtsaat
const float BAHN_WEITE = 0.35;   // Radius der Lissajous-Bahn
const float DECAY      = 0.90;   // NEU: Daempfung je Frame - MUSS < 1 bleiben!
// ----------------------------------------------------------------------------

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

    vec3 alt = texture(iChannel0, fragCoord / iResolution.xy).rgb;

    // GEAENDERT: der Gegenspieler - jeder alte Wert verliert 10% pro Frame
    vec3 neu = alt * DECAY + seeds(uv);

    fragColor = vec4(neu, 1.0);
}
