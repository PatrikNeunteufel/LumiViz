void mainImage(out vec4 fragColor, in vec2 fragCoord)
{
    // Ursprung in die Mitte, Division durch die HOEHE (nur y!)
    vec2 uv = (fragCoord - 0.5 * iResolution.xy) / iResolution.xy;
    uv = (fragCoord - 0.5 * iResolution.xy) / iResolution.y;

    // Abstand zur Mitte als Helligkeit anzeigen
    fragColor = vec4(vec3(length(uv)), 1.0);
    fragColor = vec4(vec3(step(0.3, length(uv))), 1.0);
}