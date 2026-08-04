void mainImage(out vec4 fragColor, in vec2 fragCoord)
{
    // Ursprung in die Bildmitte, Teilen durch die HOEHE (unverzerrt)
    vec2 uv = (fragCoord - 0.5 * iResolution.xy) / iResolution.y;

    float r = length(uv);            // Abstand zur Bildmitte
    float w = atan(uv.y, uv.x);      // Winkel um die Bildmitte: -pi..pi

    // Polar-Mapping wie im Vorbild: Tiefe ~ 1/Radius, Umfang ~ Winkel
    float tief0 = 0.5 / max(r, 1e-3);        // Wandtiefe dieses Pixels
    float tiefe = tief0 + iTime * 1.5;       // ... und die Fahrt nach vorn

    // 14 Roehren-Baender um den Umfang, Spanten-Ringe in der Tiefe
    float roehren = 0.5 - 0.5 * cos(w * 14.0);
    float spanten = smoothstep(0.4, 0.0, abs(fract(tiefe / 4.0) - 0.5) * 4.0);

    vec3 wandfarbe = mix(vec3(0.05, 0.06, 0.10), vec3(0.15, 0.18, 0.26), roehren);
    wandfarbe += vec3(0.25, 0.30, 0.45) * spanten * 0.5;

    // Ferne (= Bildmitte) versinkt im Dunkel
    float schleier = exp(-0.10 * tief0);

    fragColor = vec4(wandfarbe * schleier, 1.0);
}
