void mainImage(out vec4 fragColor, in vec2 fragCoord)
{
    vec2 uv = fragCoord / iResolution.xy;
    vec4 zust  = texelFetch(iChannel0, ivec2(0, 0), 0);
    vec4 audio = texelFetch(iChannel0, ivec2(1, 0), 0);

    // Hintergrund = Blendwert: dunkelblau haelt A, dunkelrot haelt B
    float kurve = smoothstep(0.0, 1.0, zust.x);
    float t = mix(kurve, 1.0 - kurve, zust.y);
    vec3 col = mix(vec3(0.04, 0.07, 0.16), vec3(0.14, 0.03, 0.03), t);

    // Balken: geglaetteter Bass | Beat-Envelope | Blende-Phase
    if (uv.x < 0.31)                col = uv.y < audio.x ? vec3(0.9, 0.6, 0.2) : col;
    if (uv.x > 0.35 && uv.x < 0.65) col = uv.y < audio.y ? vec3(0.3, 0.9, 1.0) : col;
    if (uv.x > 0.69)                col = uv.y < zust.x  ? vec3(0.8, 0.3, 0.9) : col;

    fragColor = vec4(col, 1.0);
}
