// 29 Fluss-Feld, Image — Patrik Neunteufel (LumiViz, 2026). Eigenwerk.
// Shadertoy-Tab "Image": iChannel0 = Buffer A. Anzeige + leichte Aufsteilung.

// ---- STELLSCHRAUBEN --------------------------------------------------------
const float SAETTIGUNG = 1.15; // >1 = kräftigere Farben
const float GAMMA      = 0.9;  // <1 = heller
// ----------------------------------------------------------------------------

void mainImage(out vec4 fragColor, in vec2 fragCoord)
{
    vec2 uv = fragCoord / iResolution.xy;
    vec3 c = texture(iChannel0, uv).rgb;
    float grau = dot(c, vec3(0.333));
    c = mix(vec3(grau), c, SAETTIGUNG);       // Sättigung anheben
    c = pow(max(c, 0.0), vec3(GAMMA));        // Gamma
    fragColor = vec4(c, 1.0);
}
