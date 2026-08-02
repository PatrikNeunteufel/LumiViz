// 85 DNA-Daten — Patrik Neunteufel (LumiViz, 2026). Eigenwerk.
// Shadertoy: iChannel0 = Music. KOMBI: DNA-Helix × Daten-Regen — die
// Helixstränge bestehen aus fallenden Glyphen-Spalten (die Erbinformation
// "regnet" die Helix hinunter), Sprossen als Datenbrücken.

// ---- STELLSCHRAUBEN --------------------------------------------------------
const float WINDUNGEN  = 2.5;
const float DREH_TEMPO = 0.6;
const float RADIUS     = 0.4;
const float FALL_TEMPO = 0.35;   // Glyphen fallen die Stränge entlang
const float ZEILEN     = 36.0;   // Glyphen-Raster vertikal
const float STRANG_BREITE = 0.075;
// ----------------------------------------------------------------------------

float n21(vec2 p) { return fract(sin(dot(p, vec2(41.3, 289.1))) * 43758.5453); }
void mainImage(out vec4 fragColor, in vec2 fragCoord)
{
    vec2 uv = (2.0 * fragCoord - iResolution.xy) / iResolution.y;
    float bass = texture(iChannel0, vec2(0.05, 0.25)).x;
    float dreh = iTime * (DREH_TEMPO + bass * 0.8);
    vec3 col = vec3(0.005, 0.012, 0.008);

    float phase = uv.y * WINDUNGEN * 6.28318 + dreh;
    for (int s = 0; s < 2; ++s)
    {
        float ph = phase + float(s) * 3.14159;
        float x = RADIUS * sin(ph);
        float tiefe = 0.5 + 0.5 * cos(ph);
        float d = abs(uv.x - x);
        if (d < STRANG_BREITE)
        {
            // Glyphen-Zelle im Strang: 3x3-Bitmuster, fällt mit der Zeit
            float zeile = floor((uv.y + iTime * FALL_TEMPO) * ZEILEN);
            vec2 sub = floor(vec2((uv.x - x) / STRANG_BREITE * 0.5 + 0.5,
                                  fract((uv.y + iTime * FALL_TEMPO) * ZEILEN)) * 3.0);
            float bit = step(0.45, n21(vec2(zeile, float(s) * 13.0) + sub * 0.37));
            vec3 farbe = (s == 0) ? vec3(0.25, 1.0, 0.45) : vec3(0.3, 0.75, 1.0);
            col += bit * farbe * (0.25 + 0.75 * tiefe);
        }
    }
    // Sprossen: Datenbrücken in festen Höhen, Helligkeit pulsiert
    for (int i = 0; i < 14; ++i)
    {
        float y = -1.0 + 2.0 * (float(i) + 0.5) / 14.0;
        float ph = y * WINDUNGEN * 6.28318 + dreh;
        float x1 = RADIUS * sin(ph);
        float x2 = RADIUS * sin(ph + 3.14159);
        float tiefe = 0.5 + 0.5 * cos(ph);
        vec2 p = vec2(clamp(uv.x, min(x1, x2), max(x1, x2)), y);
        float d = length(uv - p);
        float puls = 0.5 + 0.5 * sin(iTime * 3.0 + float(i) * 1.7);
        col += smoothstep(0.008, 0.003, d) * vec3(0.5, 0.9, 0.7) *
               tiefe * puls * 0.8;
    }
    fragColor = vec4(col, 1.0);
}
