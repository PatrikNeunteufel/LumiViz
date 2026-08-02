// 47 Funken-Fontäne, Image — Patrik Neunteufel (LumiViz, 2026). Eigenwerk.
// "Image": iChannel0 = Buffer A. Anzeige + warmer Boden-Reflex.

void mainImage(out vec4 fragColor, in vec2 fragCoord)
{
    vec2 uv = fragCoord / iResolution.xy;
    vec3 c = texture(iChannel0, uv).rgb;
    // Bodenreflex: unteres Bild leicht gespiegelt und abgedunkelt
    if (uv.y < 0.08)
    {
        vec3 spiegel = texture(iChannel0, vec2(uv.x, 0.16 - uv.y)).rgb;
        c += spiegel * 0.25 * (uv.y / 0.08);
    }
    fragColor = vec4(c + vec3(0.008, 0.004, 0.006), 1.0);
}
