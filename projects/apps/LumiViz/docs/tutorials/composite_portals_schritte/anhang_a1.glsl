// iChannel0: Music
float bandLevel(float lo, float hi)
{
    float sum = 0.0;
    const int N = 12;
    for (int i = 0; i < N; i++)
        sum += texture(iChannel0, vec2(mix(lo, hi, (float(i) + 0.5) / float(N)), 0.25)).x;
    return sum / float(N);
}

void mainImage(out vec4 fragColor, in vec2 fragCoord)
{
    vec2 uv = fragCoord / iResolution.xy;
    float bass = bandLevel(0.00, 0.05);
    float gate = smoothstep(0.60, 0.75, bass);      // DAS Beat-Gate

    vec3 col = vec3(0.02);
    if (uv.x < 0.47) col = uv.y < bass ? vec3(0.9, 0.3, 0.3) : col;   // roher Pegel
    if (uv.x > 0.53) col = uv.y < gate ? vec3(0.3, 0.9, 1.0) : col;   // das Gate
    fragColor = vec4(col + gate * vec3(0.08, 0.05, 0.02), 1.0);
}
