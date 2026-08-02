// 14 Spektrogramm-Rad, Image — Patrik Neunteufel (LumiViz, 2026). Eigenwerk.
// Shadertoy-Tab "Image": iChannel0 = Buffer A.
//
// IDEE: Historie polar aufrollen: Winkel = Zeit (Rad dreht), Radius =
// Frequenz (innen Bass, außen Höhen), Helligkeit = damalige Lautstärke.

// ---- STELLSCHRAUBEN --------------------------------------------------------
const float DREH_TEMPO  = 0.02; // Rotation des Rades
const float FARB_ZYKLEN = 0.8;  // Farbverlauf über den Radius
const float HELLIGKEIT  = 1.6;
const float AUSSEN_FADE = 1.25; // ab hier ausblenden (bis 1.0 voll)
const float MITTE_FREI  = 0.08; // dunkles Zentrum
// ----------------------------------------------------------------------------

void mainImage(out vec4 fragColor, in vec2 fragCoord)
{
    vec2 uv = (2.0 * fragCoord - iResolution.xy) / iResolution.y;
    float r = length(uv);
    float a = atan(uv.y, uv.x) / 6.28318 + 0.5;  // Winkel 0..1
    // Winkel -> Zeitspalte (rotierend), Radius -> Frequenzzeile
    float hist = texture(iChannel0,
                         vec2(1.0 - fract(a + iTime * DREH_TEMPO),
                              clamp(r, 0.0, 1.0))).x;
    vec3 col = (0.5 + 0.5 * cos(6.28318 * (r * FARB_ZYKLEN - iTime * 0.05) +
                                vec3(0.0, 2.1, 4.2))) * hist * HELLIGKEIT;
    col *= smoothstep(AUSSEN_FADE, 1.0, r);
    col *= smoothstep(0.0, MITTE_FREI, r);
    fragColor = vec4(col, 1.0);
}
