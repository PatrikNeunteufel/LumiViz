// 66 Opal-Schimmer — Patrik Neunteufel (LumiViz, 2026). Eigenwerk.
// Shadertoy: iChannel0 = Music. KRISTALL: opalisierendes Farbspiel.
//
// IDEE: Opal = Farbflecken (Voronoi-Patches), deren Farbe von der
// BLICKRICHTUNG abhängt — hier simuliert durch eine Interferenz-Palette,
// deren Phase je Patch und mit einer wandernden "Neigung" driftet.
// Weiche Patch-Grenzen (zweite Voronoi-Distanz) = milchiges Schimmern.

// ---- STELLSCHRAUBEN --------------------------------------------------------
const float PATCH_DICHTE = 3.2;
const float NEIGUNG_TEMPO = 0.4;  // wanderndes "Kippen" des Steins
const float INTERFERENZ  = 3.0;   // Farbzyklen je Patch
const float MILCH        = 0.45;  // milchige Grundtrübung
const float VOL_LEUCHTEN = 0.8;
// ----------------------------------------------------------------------------

vec2 hash2(vec2 p)
{
    p = vec2(dot(p, vec2(127.1, 311.7)), dot(p, vec2(269.5, 183.3)));
    return fract(sin(p) * 43758.5453);
}
void mainImage(out vec4 fragColor, in vec2 fragCoord)
{
    vec2 uv = (2.0 * fragCoord - iResolution.xy) / iResolution.y;
    float vol = texture(iChannel0, vec2(0.15, 0.25)).x;

    vec2 p = uv * PATCH_DICHTE;
    vec2 g = floor(p);
    vec2 f = fract(p);
    float d1 = 8.0, d2 = 8.0;
    vec2 id = vec2(0.0);
    for (int y = -1; y <= 1; ++y)
    for (int x = -1; x <= 1; ++x)
    {
        vec2 o = vec2(float(x), float(y));
        vec2 h = hash2(g + o);
        vec2 c = o + h - f;
        float d = dot(c, c);
        if (d < d1) { d2 = d1; d1 = d; id = h; }
        else if (d < d2) d2 = d;
    }
    float grenzWeiche = smoothstep(0.0, 0.25, d2 - d1);  // weiche Patchgrenzen

    // "Neigung": wandernde Phase simuliert den Blickwinkel-Effekt
    float neigung = sin(iTime * NEIGUNG_TEMPO + uv.x * 1.5) +
                    cos(iTime * NEIGUNG_TEMPO * 0.7 + uv.y * 1.2);
    // Interferenz-Farbe je Patch (Id = Grundphase)
    vec3 spiel = 0.5 + 0.5 * cos(id.x * 6.28318 * INTERFERENZ + neigung * 2.0 +
                                 vec3(0.0, 2.1, 4.2));
    // Milchgrund + Farbspiel nur in den Patch-Kernen
    vec3 milch = vec3(0.85, 0.88, 0.92) * MILCH;
    vec3 col = milch + spiel * grenzWeiche * (0.5 + VOL_LEUCHTEN * vol);
    // Stein-Vignette (oval)
    col *= smoothstep(1.25, 0.7, length(uv * vec2(0.85, 1.15)));
    fragColor = vec4(col, 1.0);
}
