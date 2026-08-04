void mainImage(out vec4 fragColor, in vec2 fragCoord)
{
    // Ursprung in die Bildmitte, Teilen durch die HOEHE (unverzerrt)
    vec2 uv = (fragCoord - 0.5 * iResolution.xy) / iResolution.y;

    // Nachthimmel: unten leicht aufgehellt (Dunst), oben fast schwarz
    vec3 col = mix(vec3(0.030, 0.028, 0.045), vec3(0.010, 0.012, 0.022),
                   clamp(uv.y * 1.5 + 0.5, 0.0, 1.0));

    // Der Moloch als Silhouette: Mittelpunkt UEBER der Bildmitte,
    // Radius groesser als die halbe Bildhoehe -> der Kreis sprengt das Bild
    vec2 zentrum = vec2(0.0, 0.55);
    float r = length(uv - zentrum);
    float silhouette = smoothstep(0.725, 0.715, r);

    // Gegenlicht-Saum: ein schmaler, kalter Rand um die Silhouette
    float saum = smoothstep(0.79, 0.72, r) - silhouette;
    col += saum * vec3(0.30, 0.38, 0.55) * 0.7;

    // die Silhouette selbst: fast schwarz, nur eine Spur heller als nichts
    col = mix(col, vec3(0.012, 0.013, 0.020), silhouette);

    fragColor = vec4(col, 1.0);
}
