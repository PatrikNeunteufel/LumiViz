// Schritt 1 - Die Buehne: Bildaufteilung
// Voll-Listing aus CrystalLights-tutorial.md (SSOT dort).
void mainImage(out vec4 fragColor, in vec2 fragCoord)
{
    // Ursprung in die Bildmitte, Teilen durch die HOEHE (unverzerrt)
    vec2 uv = (fragCoord - 0.5 * iResolution.xy) / iResolution.y;

    // Horizont knapp UEBER der Mitte: Terrain bekommt etwas mehr als
    // die halbe Bildflaeche - Reserve fuer Huegel kommt spaeter von selbst
    const float HORIZONT = 0.15;

    vec3 himmel = mix(vec3(0.10, 0.12, 0.22), vec3(0.02, 0.03, 0.08),
                      clamp((uv.y - HORIZONT) * 2.5, 0.0, 1.0));
    vec3 boden  = mix(vec3(0.30, 0.55, 0.65), vec3(0.04, 0.09, 0.13),
                      clamp((HORIZONT - uv.y) * 1.6, 0.0, 1.0));

    fragColor = vec4(uv.y > HORIZONT ? himmel : boden, 1.0);
}
