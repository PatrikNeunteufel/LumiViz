// 50 DNA-Helix — Patrik Neunteufel (LumiViz, 2026). Eigenwerk.
// Shadertoy: iChannel0 = Music. BIOLOGIE: rotierende Doppelhelix.
//
// IDEE: Zwei Sinus-Stränge (um 180° versetzt) laufen vertikal; die
// x-Position je Höhe ist die Projektion der rotierenden Helix. Sprossen
// (Basenpaare) verbinden die Stränge in festen Abständen; die Tiefe
// (cos-Anteil) steuert Größe + Helligkeit — vorne dick, hinten dünn.

// ---- STELLSCHRAUBEN --------------------------------------------------------
const float WINDUNGEN   = 3.0;   // Windungen im Bild
const float DREH_TEMPO  = 0.8;
const float RADIUS      = 0.35;  // Helix-Radius
const float STRANG_DICKE = 0.02;
const int   SPROSSEN    = 24;    // Basenpaare im Bild
const float BASS_DREH   = 1.0;   // Bass beschleunigt die Rotation
// ----------------------------------------------------------------------------

void mainImage(out vec4 fragColor, in vec2 fragCoord)
{
    vec2 uv = (2.0 * fragCoord - iResolution.xy) / iResolution.y;
    float bass = texture(iChannel0, vec2(0.05, 0.25)).x;
    float dreh = iTime * (DREH_TEMPO + BASS_DREH * bass);

    vec3 col = vec3(0.01, 0.012, 0.03);
    // Phase entlang der Höhe
    float phase = uv.y * WINDUNGEN * 6.28318 + dreh;

    // die beiden Stränge: x = R·sin(phase [+π]); Tiefe = cos(...)
    for (int s = 0; s < 2; ++s)
    {
        float ph = phase + float(s) * 3.14159;
        float x = RADIUS * sin(ph);
        float tiefe = 0.5 + 0.5 * cos(ph);  // 1 = vorn
        float d = abs(uv.x - x);
        float dicke = STRANG_DICKE * (0.5 + tiefe);
        float strang = smoothstep(dicke, dicke * 0.3, d);
        vec3 farbe = (s == 0) ? vec3(0.3, 0.7, 1.0) : vec3(1.0, 0.5, 0.7);
        col += strang * farbe * (0.35 + 0.65 * tiefe);
    }
    // Sprossen: horizontale Verbinder an quantisierten Höhen
    for (int i = 0; i < SPROSSEN; ++i)
    {
        float y = -1.0 + 2.0 * (float(i) + 0.5) / float(SPROSSEN);
        // y-Spanne des Bildes ist ±(Bildhöhe/2·Aspekt-normiert) — hier ±1
        float ph = y * WINDUNGEN * 6.28318 + dreh;
        float x1 = RADIUS * sin(ph);
        float x2 = RADIUS * sin(ph + 3.14159);
        float tiefe = 0.5 + 0.5 * cos(ph);
        // Punkt-zu-Strecke-Abstand (horizontales Segment auf Höhe y)
        float xa = min(x1, x2), xb = max(x1, x2);
        vec2 p = vec2(clamp(uv.x, xa, xb), y);
        float d = length(uv - p);
        float sprosse = smoothstep(0.012, 0.004, d);
        // Basenfarbe wechselt je Sprosse (2 Paare: gelbgrün / orangeblau)
        vec3 basis = (mod(float(i), 2.0) < 1.0) ? vec3(0.6, 0.9, 0.3)
                                                : vec3(1.0, 0.7, 0.2);
        col += sprosse * basis * (0.25 + 0.75 * tiefe);
    }
    fragColor = vec4(col, 1.0);
}
