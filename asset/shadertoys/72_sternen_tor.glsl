// 72 Sternen-Tor — Patrik Neunteufel (LumiViz, 2026). Eigenwerk.
// Shadertoy: iChannel0 = Music. GLOW: Portalring mit Ereignisfläche.
//
// IDEE: Ein glühender Ring (Chevron-Zacken über den Winkel) um eine
// wabernde "Wasserfläche" (sin-Interferenz, radial nach innen gezogen).
// Beim Bass wallt die Fläche auf und der Ring gleißt. Sterne dahinter.

// ---- STELLSCHRAUBEN --------------------------------------------------------
const float RING_RADIUS = 0.62;
const float RING_DICKE  = 0.05;
const float CHEVRONS    = 9.0;   // Zacken auf dem Ring
const float WALLEN      = 0.5;   // Bass-Aufwallen der Fläche
const float FLAECHE_TEMPO = 1.2;
// ----------------------------------------------------------------------------

float n21(vec2 p) { return fract(sin(dot(p, vec2(41.3, 289.1))) * 43758.5453); }
void mainImage(out vec4 fragColor, in vec2 fragCoord)
{
    vec2 uv = (2.0 * fragCoord - iResolution.xy) / iResolution.y;
    float bass = texture(iChannel0, vec2(0.05, 0.25)).x;
    float r = length(uv);
    float a = atan(uv.y, uv.x);

    // Sternenhimmel hinter allem
    vec3 col = vec3(0.005, 0.005, 0.015);
    col += step(0.998, n21(floor(uv * 80.0))) * 0.5;

    if (r < RING_RADIUS - RING_DICKE * 0.5)
    {
        // Ereignisfläche: Interferenz zweier Radialwellen, zur Mitte gesogen
        float welle = sin(r * 18.0 - iTime * FLAECHE_TEMPO * 3.0) +
                      sin(a * 3.0 + r * 9.0 - iTime * FLAECHE_TEMPO * 2.0);
        float tiefe = 1.0 - r / RING_RADIUS;
        vec3 wasser = mix(vec3(0.05, 0.15, 0.3), vec3(0.3, 0.6, 0.95),
                          0.5 + 0.25 * welle * (1.0 + WALLEN * bass));
        col = wasser * (0.5 + 0.5 * tiefe);
        // Sog-Glanz in der Mitte
        col += exp(-r * 6.0) * vec3(0.6, 0.8, 1.0) * (0.4 + 0.6 * bass);
    }
    // Ring: Grundtorus + Chevron-Zacken glühen umlaufend
    float ringD = abs(r - RING_RADIUS);
    float ring = smoothstep(RING_DICKE, RING_DICKE * 0.3, ringD);
    float chevron = pow(0.5 + 0.5 * cos(a * CHEVRONS - iTime * 0.6), 6.0);
    col += ring * (vec3(0.35, 0.3, 0.2) +
                   chevron * vec3(1.0, 0.6, 0.2) * (0.7 + 0.8 * bass));
    col += exp(-ringD * 18.0) * vec3(0.9, 0.6, 0.3) * 0.35;  // Ring-Halo
    fragColor = vec4(col, 1.0);
}
