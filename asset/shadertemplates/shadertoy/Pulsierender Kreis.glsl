// Kreis, der mit dem Bass waechst. bass/mid/treb sind LumiViz-Extras —
// auf shadertoy.com laufen sie nicht (dort die Audio-Textur benutzen).
void mainImage(out vec4 fragColor, in vec2 fragCoord)
{
    vec2 p = (fragCoord - 0.5 * iResolution.xy) / iResolution.y;
    float r = 0.15 + 0.2 * bass;
    float kreis = smoothstep(r, r - 0.01, length(p));
    vec3 col = mix(vec3(0.02, 0.02, 0.05), vec3(0.9, 0.4, 0.2 + treb), kreis);
    fragColor = vec4(col, 1.0);
}
