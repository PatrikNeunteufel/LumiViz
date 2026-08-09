// iChannel0: Music

// Mittelwert eines Frequenzbands aus der FFT-Zeile der Musik-Textur
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

void mainImage(out vec4 fragColor, in vec2 fragCoord)
{
    vec2 uv = fragCoord / iResolution.xy;
    vec2 cuv = (fragCoord - 0.5 * iResolution.xy) / iResolution.y;

    float bass = bandLevel(0.00, 0.05);
    float gate = smoothstep(0.60, 0.75, bass);    // DAS Beat-Gate

    vec3 color = vec3(0.02);

    // links: roher Bass-Pegel  |  rechts: das Gate (aus oder an)
    if (uv.x < 0.47) color = uv.y < bass ? vec3(0.9, 0.3, 0.3) : color;
    if (uv.x > 0.53) color = uv.y < gate ? vec3(0.3, 0.9, 1.0) : color;

    // und ein Ring in der Mitte zuckt mit dem Gate - die kommende Ringlicht-Logik
    float ring = abs(length(cuv) - 0.28 - 0.04 * gate);
    color += vec3(0.5, 0.7, 1.0) * gate * 0.02 / (0.001 + ring * ring * 40.0);

    fragColor = vec4(color, 1.0);
}
