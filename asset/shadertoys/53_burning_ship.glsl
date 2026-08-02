// 53 Burning Ship — Patrik Neunteufel (LumiViz, 2026). Eigenwerk.
// Shadertoy: iChannel0 = Music. FRAKTAL: das "brennende Schiff".
//
// IDEE: Wie Mandelbrot, aber mit abs() vor dem Quadrieren — das erzeugt die
// charakteristischen brennenden Masten. Feurige Palette, atmender Zoom auf
// die Schiffs-Silhouette; der Bass facht die Glut an.

// ---- STELLSCHRAUBEN --------------------------------------------------------
const vec2  ZENTRUM     = vec2(-1.755, -0.03);  // die kleine Schiffs-Kopie
const float ZOOM_MITTE  = 3.2;
const float ZOOM_HUB    = 1.2;
const float ATEM_TEMPO  = 0.12;
const int   ITERATIONEN = 120;
const float GLUT_BASS   = 0.5;
// ----------------------------------------------------------------------------

void mainImage(out vec4 fragColor, in vec2 fragCoord)
{
    vec2 uv = (2.0 * fragCoord - iResolution.xy) / iResolution.y;
    uv.y = -uv.y;  // das Schiff "richtig herum"
    float bass = texture(iChannel0, vec2(0.05, 0.25)).x;

    float zoom = exp(ZOOM_MITTE + ZOOM_HUB * sin(iTime * ATEM_TEMPO));
    vec2 c = ZENTRUM + uv / zoom;
    vec2 z = vec2(0.0);
    float m = 0.0;
    bool drin = true;
    for (int i = 0; i < ITERATIONEN; ++i)
    {
        z = abs(z);  // DER Burning-Ship-Trick
        z = vec2(z.x * z.x - z.y * z.y, 2.0 * z.x * z.y) + c;
        if (dot(z, z) > 64.0) { drin = false; break; }
        m += 1.0;
    }
    vec3 col = vec3(0.0);
    if (!drin)
    {
        float glatt = m - log2(log2(dot(z, z)) * 0.5);
        float t = glatt / float(ITERATIONEN);
        // Feuer-Palette: schwarz → rot → orange → weißgelb
        float f = pow(t, 0.5) * (1.2 + GLUT_BASS * bass);
        col = vec3(f * 1.5, f * f * 1.2, f * f * f * 0.8);
    }
    fragColor = vec4(col, 1.0);
}
