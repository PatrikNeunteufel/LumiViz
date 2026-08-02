// 43 Zell-Kolonie, Image — Patrik Neunteufel (LumiViz, 2026). Eigenwerk.
// "Image": iChannel0 = Buffer A. Dichtefeld (.g) als Membran-Farbverlauf:
// Koloniekern petrol, Membran leuchtet giftgrün.

// ---- STELLSCHRAUBEN --------------------------------------------------------
const float MEMBRAN_SCHWELLE = 0.35; // Dichte, an der die Membran liegt
const float MEMBRAN_BREITE   = 0.12;
// ----------------------------------------------------------------------------

void mainImage(out vec4 fragColor, in vec2 fragCoord)
{
    vec2 uv = fragCoord / iResolution.xy;
    vec4 z = texture(iChannel0, uv);
    float dichte = z.g;
    // Membran: schmale Zone um die Schwelle glüht
    float membran = exp(-pow((dichte - MEMBRAN_SCHWELLE) / MEMBRAN_BREITE, 2.0));
    vec3 col = vec3(0.01, 0.02, 0.03);
    col += z.r * vec3(0.05, 0.25, 0.22) * (0.5 + dichte);   // Koloniekörper
    col += membran * vec3(0.35, 1.0, 0.25) * 0.8;           // Membran-Glow
    fragColor = vec4(col, 1.0);
}
