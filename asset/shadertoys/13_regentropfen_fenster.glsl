// 13 Regentropfen-Fenster — Patrik Neunteufel (LumiViz, 2026). Eigenwerk.
// Shadertoy: iChannel0 = Music (512x2: y=0.25 FFT, y=0.75 Waveform).
//
// IDEE: Zwei Tropfen-Raster laufen die Scheibe hinunter; jeder Tropfen
// bricht das Farb-Bokeh dahinter (Sample-Offset). Lautstärke = Brechung.

// ---- STELLSCHRAUBEN --------------------------------------------------------
const float FALL_GRUND    = 0.22;  // Fallgeschwindigkeit Grundwert
const float FALL_STREUUNG = 0.45;  //  … Zufalls-Streuung je Tropfen
const float TROPFEN_GROESSE = 0.08;
const float BRECHUNG_GRUND = 0.5;  // Brechungsstärke
const float BRECHUNG_VOL   = 0.4;  //  … Lautstärke-Zuschlag
const float BOKEH_TEMPO    = 0.6;  // Fließtempo des Hintergrunds
const float RASTER_X      = 14.0;  // Tropfenraster Schicht 1
const float RASTER_Y      = 10.0;
const float SCHICHT2_FAKTOR = 1.7; // Schicht 2 = Raster × Faktor (feiner)
// ----------------------------------------------------------------------------

vec2 h22(vec2 p)
{
    p = vec2(dot(p, vec2(127.1, 311.7)), dot(p, vec2(269.5, 183.3)));
    return fract(sin(p) * 43758.5453);
}
void mainImage(out vec4 fragColor, in vec2 fragCoord)
{
    vec2 uv = fragCoord / iResolution.xy;
    float vol = texture(iChannel0, vec2(0.15, 0.25)).x;

    vec2 offset = vec2(0.0);
    vec2 grid = vec2(RASTER_X, RASTER_Y);
    for (int layer = 0; layer < 2; ++layer)
    {
        vec2 g = uv * grid + float(layer) * 3.7;
        vec2 id = floor(g);                        // Zellen-Id
        vec2 f = fract(g);                         // Position in der Zelle
        vec2 rnd = h22(id + float(layer) * 17.0);  // Zufall je Zelle
        // Fallphase 0..1, wrappt — endloser Regen, Tempo je Zelle
        float t = fract(iTime * (FALL_GRUND + FALL_STREUUNG * rnd.y) + rnd.x);
        vec2 d = f - vec2(rnd.x, 1.0 - t);
        d.x *= grid.y / grid.x;                    // rund statt oval
        float drop = smoothstep(TROPFEN_GROESSE, 0.0, length(d));
        offset += drop * d * 2.0;                  // Brech-Richtung sammeln
        grid *= SCHICHT2_FAKTOR;
    }
    // Hintergrund an der GEBROCHENEN Position sampeln
    vec2 p = uv + offset * (BRECHUNG_GRUND + BRECHUNG_VOL * vol);
    vec3 col = 0.5 + 0.5 * cos(vec3(p.x * 4.0 + iTime * BOKEH_TEMPO,
                                    p.y * 5.0 + 2.0 - iTime * 0.4,
                                    (p.x + p.y) * 3.0 + 4.0 + iTime * 0.3));
    col *= 0.55 + 0.45 * vol;
    col *= 1.0 - 0.5 * length(uv - 0.5);  // Vignette
    fragColor = vec4(col, 1.0);
}
