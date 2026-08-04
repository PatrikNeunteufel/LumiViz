// iChannel0: Music

// Mittelwert eines FFT-Bandes (lo..hi in 0..1 der Spektrum-Breite)
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

    float bass = bandLevel(0.00, 0.05);
    float vol  = bandLevel(0.00, 0.70);
    float gate = smoothstep(0.60, 0.75, bass);    // das Beat-Gate

    // Hintergrund: die STIMMUNGs-Vorschau - Lautheit blendet dark -> brighter
    float stimmung = clamp(vol * 1.4 - 0.25, 0.0, 1.0);
    vec3 color = mix(vec3(0.010, 0.012, 0.022), vec3(0.10, 0.13, 0.20), stimmung);

    // links: roher Bass-Pegel  |  rechts: das Gate (aus oder an)
    if (uv.x < 0.47) color = uv.y < bass ? vec3(0.9, 0.3, 0.3) : color;
    if (uv.x > 0.53) color = uv.y < gate ? vec3(0.3, 0.9, 1.0) : color;

    fragColor = vec4(color, 1.0);
}
