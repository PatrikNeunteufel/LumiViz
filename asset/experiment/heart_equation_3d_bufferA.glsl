// heart_equation_3d — TAB 'Buffer A' (nur DIESEN Code in den Buffer-A-Tab!)
// Verdrahtung: iChannel0 = Buffer A (Selbstreferenz), iChannel1 = Music (optional)

// Integriert die drei Rotationswinkel als Zustand in Pixel (0,0).

// ---- ROTATIONS-STELLSCHRAUBEN (je Achse separat einstellbar) ---------------
const vec3  BASIS      = vec3(0.25, 0.35, 0.15); // Grundtempo je Achse [rad/s]
const vec3  GAIN       = vec3(1.20, 1.00, 0.80); // Audio-Anteil je Achse
const float SEG_DAUER  = 3.0;   // alle N Sekunden wuerfelt jede Achse neu
const float STILLSTAND = 0.35;  // Chance je Achse und Segment, NICHT zu rotieren
// ----------------------------------------------------------------------------

float hash11(float p) { return fract(sin(p*127.1)*43758.5453); }

float bandLevel(float lo, float hi)
{
    float s = 0.0;
    for (int i = 0; i < 8; i++)
        s += texture(iChannel1, vec2(mix(lo, hi, (float(i)+0.5)/8.0), 0.25)).x;
    return s/8.0;
}

// Zufallsgeschwindigkeit der Achse a im Zeit-Segment seg: -1..1 oder exakt 0
float omega(float seg, float a)
{
    float r1 = hash11(seg*3.7 + a*17.3);
    float r2 = hash11(seg*9.1 + a*31.7 + 5.0);
    return (r1 < STILLSTAND) ? 0.0 : (r2*2.0 - 1.0);
}

void mainImage(out vec4 fragColor, in vec2 fragCoord)
{
    vec4 z = texelFetch(iChannel0, ivec2(0,0), 0);
    if (iFrame == 0) z = vec4(0.0);

    float dt  = clamp(iTimeDelta, 1.0/240.0, 1.0/24.0);
    float seg = floor(iTime / SEG_DAUER);
    vec3 band = vec3(bandLevel(0.00,0.05), bandLevel(0.05,0.25), bandLevel(0.25,0.70));
    vec3 w    = vec3(omega(seg,0.0), omega(seg,1.0), omega(seg,2.0)) * (BASIS + GAIN*band);

    z.xyz += w * dt;                      // Winkel = integrierte Geschwindigkeit
    fragColor = z;
}

// ==============================
