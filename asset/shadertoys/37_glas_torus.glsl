// 37 Glas-Torus — Patrik Neunteufel (LumiViz, 2026). Eigenwerk.
// Shadertoy: iChannel0 = Music. RAYMARCHING + FAKE-GLAS (Reflexion/Brechung).
//
// IDEE: Ein rotierender Torus wird geraymarcht; "Glas" entsteht ohne echten
// zweiten Strahl: die Umgebung (prozeduraler Streifen-Himmel) wird einmal
// über den Reflexionsvektor und einmal über einen gebrochenen Vektor
// gesampelt und per Fresnel gemischt. Bass = Torus-Dicke.

// ---- STELLSCHRAUBEN --------------------------------------------------------
const float RING_RADIUS = 0.75;  // großer Radius
const float DICKE_GRUND = 0.24;  // Rohr-Radius
const float DICKE_BASS  = 0.10;
const float DREH_TEMPO  = 0.5;
const float BRECHUNG    = 0.8;   // Brechungsstärke des "Glases"
const int   MARSCH      = 72;
// ----------------------------------------------------------------------------

float g_dicke;
vec2 rot2(vec2 p, float a) { return vec2(p.x * cos(a) - p.y * sin(a),
                                         p.x * sin(a) + p.y * cos(a)); }
float torus(vec3 p)
{
    vec2 q = vec2(length(p.xz) - RING_RADIUS, p.y);
    return length(q) - g_dicke;
}
// prozedurale Umgebung: Verlaufs-Himmel + farbige Lichtstreifen
vec3 umgebung(vec3 rd)
{
    vec3 sky = mix(vec3(0.04, 0.03, 0.08), vec3(0.15, 0.1, 0.25),
                   rd.y * 0.5 + 0.5);
    float streifen = pow(0.5 + 0.5 * sin(atan(rd.z, rd.x) * 6.0 + iTime * 0.4),
                         6.0);
    sky += streifen * (0.5 + 0.5 * cos(rd.y * 4.0 + iTime * 0.3 +
                                       vec3(0.0, 2.1, 4.2))) * 0.8;
    return sky;
}
void mainImage(out vec4 fragColor, in vec2 fragCoord)
{
    vec2 uv = (2.0 * fragCoord - iResolution.xy) / iResolution.y;
    float bass = texture(iChannel0, vec2(0.05, 0.25)).x;
    g_dicke = DICKE_GRUND + DICKE_BASS * bass;

    vec3 ro = vec3(0.0, 0.0, -2.6);
    vec3 rd = normalize(vec3(uv, 1.5));
    // Torus rotiert — wir drehen stattdessen den Strahl (billiger)
    float wa = iTime * DREH_TEMPO;
    ro.yz = rot2(ro.yz, 0.7);  rd.yz = rot2(rd.yz, 0.7);
    ro.xz = rot2(ro.xz, wa);   rd.xz = rot2(rd.xz, wa);

    float t = 0.0;
    float d = 1.0;
    for (int i = 0; i < MARSCH; ++i)
    {
        d = torus(ro + rd * t);
        if (d < 0.001 || t > 6.0) break;
        t += d;
    }
    vec3 col = umgebung(rd);
    if (d < 0.001)
    {
        vec3 p = ro + rd * t;
        vec2 e = vec2(0.002, 0.0);
        vec3 n = normalize(vec3(torus(p + e.xyy) - torus(p - e.xyy),
                                torus(p + e.yxy) - torus(p - e.yxy),
                                torus(p + e.yyx) - torus(p - e.yyx)));
        float fresnel = pow(1.0 - max(dot(n, -rd), 0.0), 3.0);
        vec3 spiegel = umgebung(reflect(rd, n));
        // "Brechung": Blickstrahl zur Normalen hin verbogen weiter-gesampelt
        vec3 gebrochen = umgebung(normalize(mix(rd, -n, BRECHUNG * 0.3)));
        col = mix(gebrochen * vec3(0.85, 0.95, 1.0), spiegel,
                  0.2 + 0.8 * fresnel);
        // Kantenlicht
        col += fresnel * fresnel * vec3(0.4, 0.6, 1.0) * 0.6;
    }
    fragColor = vec4(col, 1.0);
}
