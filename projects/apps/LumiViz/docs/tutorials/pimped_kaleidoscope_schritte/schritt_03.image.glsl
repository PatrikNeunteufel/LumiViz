void mainImage(out vec4 fragColor, in vec2 fragCoord)
{
    // vorerst reine Anzeige: Buffer A unveraendert auf den Schirm
    fragColor = vec4(texture(iChannel0, fragCoord / iResolution.xy).rgb, 1.0);
}
