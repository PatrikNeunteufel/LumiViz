void mainImage(out vec4 fragColor, in vec2 fragCoord)
{
    // Ursprung in die Bildmitte, Teilen durch die HOEHE (unverzerrt)
    vec2 uv = (fragCoord - 0.5 * iResolution.xy) / iResolution.y;

    // Lissajous-Bahn: die POSITION ist eine glatte sin-Funktion der Zeit -
    // an den Umkehrpunkten wird das Licht von selbst langsam (cos-Ableitung)
    vec2 pos = vec2(sin(iTime * 0.31), sin(iTime * 0.23)) * vec2(0.35, 0.25);

    // 1/d2-Licht: Helligkeit = Kehrwert des Abstandsquadrats
    vec2 d = uv - pos;
    float licht = 0.0006 / (0.0004 + dot(d, d));

    fragColor = vec4(licht * vec3(0.45, 0.70, 1.00), 1.0);
}
