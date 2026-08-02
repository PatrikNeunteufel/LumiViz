// 82 Kaleido-Tunnel — Patrik Neunteufel (LumiViz, 2026). Eigenwerk.
// Shadertoy: iChannel0 = Music. KOMBI: Kaleidoskop × Tunnel — der Winkel
// wird ERST in Spiegel-Segmente gefaltet, DANN läuft die Tunneltiefe:
// ein facettierter Spiegelschacht. Bass = Vortrieb, Segmentzahl schaltet.

// ---- STELLSCHRAUBEN --------------------------------------------------------
const float SEGMENTE    = 8.0;
const float SEGMENT_BASS = 4.0;  // Bass-Stufen dazu
const float VORTRIEB    = 0.5;
const float VORTRIEB_BASS = 1.0;
const float TIEFE       = 0.35;
const float MUSTER_FREQ = 7.0;
const float KANTE       = 0.08;  // Hell/Dunkel-Kante des Musters
// ----------------------------------------------------------------------------

void mainImage(out vec4 fragColor, in vec2 fragCoord)
{
    vec2 uv = (2.0 * fragCoord - iResolution.xy) / iResolution.y;
    float bass = texture(iChannel0, vec2(0.05, 0.25)).x;
    float r = length(uv) + 1e-4;
    float a = atan(uv.y, uv.x);
    // Kaleidoskop-Faltung ZUERST
    float seg = SEGMENTE + floor(bass * SEGMENT_BASS) * 2.0;
    a = abs(mod(a, 6.28318 / seg) - 3.14159 / seg);
    // dann der Tunnel
    float drive = iTime * (VORTRIEB + VORTRIEB_BASS * bass);
    float z = TIEFE / r + drive;

    float muster = sin(z * 6.28318) * sin(a * MUSTER_FREQ * seg * 0.5 + z * 0.7);
    muster = smoothstep(-KANTE, KANTE, muster);  // harte Facetten
    float fft = texture(iChannel0, vec2(fract(a * seg / 6.28318), 0.25)).x;
    vec3 col = (0.5 + 0.5 * cos(z * 0.8 + a * 3.0 + vec3(0.0, 2.1, 4.2))) * muster;
    col *= 0.4 + 1.2 * fft;
    col *= smoothstep(0.0, 0.12, r);  // Fluchtpunkt
    col += exp(-r * 3.0) * vec3(0.3, 0.2, 0.5) * bass;  // Bass-Glut in der Tiefe
    fragColor = vec4(col, 1.0);
}
