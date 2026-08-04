// Anhang A1 - Das Beat-Gate: der Rock-The-House-Trick in GLSL
// Voll-Listing aus CrystalLights-tutorial.md (SSOT dort).
// iChannel0: Music

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
    float gate = smoothstep(0.60, 0.75, bass);    // DAS Beat-Gate

    vec3 color = vec3(0.02);

    // links: der rohe Bass-Pegel  |  rechts: das Gate (aus oder an)
    if (uv.x < 0.47) color = uv.y < bass ? vec3(0.9, 0.3, 0.3) : color;
    if (uv.x > 0.53) color = uv.y < gate ? vec3(0.3, 0.9, 1.0) : color;

    // und der ganze Hintergrund blitzt, wenn das Gate offen ist
    color += gate * vec3(0.10, 0.06, 0.02);

    fragColor = vec4(color, 1.0);
}
