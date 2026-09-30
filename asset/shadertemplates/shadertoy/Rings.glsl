// Concentric rings travelling outwards.
void mainImage(out vec4 fragColor, in vec2 fragCoord)
{
    vec2 p = (fragCoord - 0.5 * iResolution.xy) / iResolution.y;
    float d = length(p);
    float v = 0.5 + 0.5 * sin(d * 40.0 - iTime * 4.0);
    fragColor = vec4(vec3(v) * vec3(0.3, 0.7, 1.0), 1.0);
}
