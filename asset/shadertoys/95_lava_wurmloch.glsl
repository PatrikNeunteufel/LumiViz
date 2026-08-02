// 95 Lava-Wurmloch — Patrik Neunteufel (LumiViz, 2026). Eigenwerk.
// Shadertoy: iChannel0 = Music. KOMBI: Wurmloch × Lava-Metaballs — im
// Torsions-Tunnel schweben glühende Lava-Blasen (Metaballs in Tunnel-
// Koordinaten, mit der Tiefe vorbeiziehend). Bass = Sog + Blasen-Glut.

// ---- STELLSCHRAUBEN --------------------------------------------------------
const float SOG        = 0.7;
const float SOG_BASS   = 1.2;
const float TORSION    = 0.3;
const int   BLASEN     = 6;
const float BLASEN_GROESSE = 0.5;
const float ISO        = 1.0;
const float ISO_KANTE  = 0.06;
// ----------------------------------------------------------------------------

float h1(float n) { return fract(sin(n) * 43758.5453); }
void mainImage(out vec4 fragColor, in vec2 fragCoord)
{
    vec2 uv = (2.0 * fragCoord - iResolution.xy) / iResolution.y;
    float bass = texture(iChannel0, vec2(0.05, 0.25)).x;
    float r = length(uv) + 1e-4;
    float a = atan(uv.y, uv.x);
    float z = 1.0 / r + iTime * (SOG + SOG_BASS * bass);
    a += z * TORSION;

    // Tunnelwand: dunkle Basalt-Streifen
    float wand = 0.5 + 0.5 * sin(a * 6.0 + z * 2.0);
    vec3 col = vec3(0.05, 0.03, 0.03) * (0.4 + 0.4 * wand) * smoothstep(0.0, 0.2, r);

    // Lava-Blasen: Metaball-Feld in (Winkel, Tiefe)-Koordinaten
    float feld = 0.0;
    for (int i = 0; i < BLASEN; ++i)
    {
        float fi = float(i);
        // Blase sitzt bei festem Winkel + wandernder Tiefe (zieht vorbei)
        float bw = h1(fi * 3.7) * 6.28318;
        float bz = fract(h1(fi * 7.1) + iTime * 0.1 * (0.5 + h1(fi))) * 12.0;
        // Abstand im (a, z)-Raum (Winkel wrap-korrekt)
        float da = abs(sin((a - bw) * 0.5)) * 2.0;
        float dz = z - bz;
        // Wrap der Tiefe (Blasen kommen periodisch wieder)
        dz = mod(dz + 6.0, 12.0) - 6.0;
        float d2 = da * da + dz * dz * 0.4;
        feld += BLASEN_GROESSE / (d2 + 0.15);
    }
    float lava = smoothstep(ISO - ISO_KANTE, ISO + ISO_KANTE, feld);
    float kern = clamp((feld - ISO) * 0.6, 0.0, 1.0);
    vec3 lavaFarbe = mix(vec3(0.9, 0.25, 0.05), vec3(1.0, 0.9, 0.4),
                         kern * (0.7 + 0.8 * bass));
    col = mix(col, lavaFarbe, lava * smoothstep(0.0, 0.15, r));
    // Tiefen-Glut im Fluchtpunkt
    col += exp(-r * 4.0) * vec3(1.0, 0.4, 0.1) * (0.4 + 0.6 * bass);
    fragColor = vec4(col, 1.0);
}
