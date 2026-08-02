// 07 Reaktions-Diffusion, Image — Patrik Neunteufel (LumiViz, 2026). Eigenwerk.
// Shadertoy-Tab "Image": iChannel0 = Buffer A.
//
// IDEE: B-Konzentration (.g) durch eine Cosinus-Palette; Hintergrund bleibt
// ausgeblendet.

// ---- STELLSCHRAUBEN --------------------------------------------------------
const float FARB_ZYKLEN  = 1.5;  // Farbzyklen über den B-Bereich
const float FARB_DRIFT   = 0.02; // langsame Palettendrift
const float SICHTBAR_AB  = 0.02; // Konzentration, ab der etwas erscheint
const float VOLL_AB      = 0.25; //  … voll sichtbar
// ----------------------------------------------------------------------------

void mainImage(out vec4 fragColor, in vec2 fragCoord)
{
    vec2 uv = fragCoord / iResolution.xy;
    float b = texture(iChannel0, uv).g;
    vec3 col = 0.5 + 0.5 * cos(6.28318 * (b * FARB_ZYKLEN + iTime * FARB_DRIFT) +
                               vec3(0.0, 2.1, 4.2));
    fragColor = vec4(col * smoothstep(SICHTBAR_AB, VOLL_AB, b), 1.0);
}
