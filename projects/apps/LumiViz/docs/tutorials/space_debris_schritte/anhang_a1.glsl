// iChannel0: Music

float hash21(vec2 p) { return fract(sin(dot(p, vec2(127.1, 311.7))) * 43758.5453); }

float bandLevel(float lo, float hi)
{
    float sum = 0.0;
    const int N = 12;
    for (int i = 0; i < N; i++) {
        float x = mix(lo, hi, (float(i) + 0.5) / float(N));
        sum += texture(iChannel0, vec2(x, 0.25)).x;
    }
    return sum / float(N);
}

vec3 signalFarbe(float x)   // dieselbe scol-Formel wie im Haupt-Shader
{
    return 0.1 + 0.9 * clamp(0.5 + sin(3.14159 / 6.0 *
        (12.0 * x + iTime / 4.0 + vec3(3.0, -1.0, -5.0))), 0.0, 1.0);
}

void mainImage(out vec4 fragColor, in vec2 fragCoord)
{
    vec2 uv = fragCoord / iResolution.xy;

    float bass = bandLevel(0.00, 0.05);
    float gate = smoothstep(0.60, 0.75, bass);        // DAS Beat-Gate

    vec3 color = vec3(0.012);

    // unten: der rohe Bass-Pegel als Kontrollbalken
    if (uv.y < 0.12) color = uv.x < bass ? vec3(0.85, 0.30, 0.20) : color;

    // darueber: 8x4 "Positionslichter" - Eigenblinken ODER gemeinsamer Beat
    if (uv.y > 0.18) {
        vec2 id = floor(vec2(uv.x * 8.0, (uv.y - 0.18) * 5.0));
        float eigen = smoothstep(0.90, 0.97, 0.5 + 0.5 *
            sin(6.28318 * (iTime * (0.3 + 0.5 * hash21(id + 7.0)) + hash21(id + 2.0))));
        float hell = max(eigen * 0.5, gate * (0.4 + 0.6 * hash21(id + 3.1)));
        color += hell * signalFarbe(hash21(id));
    }

    fragColor = vec4(color, 1.0);
}
