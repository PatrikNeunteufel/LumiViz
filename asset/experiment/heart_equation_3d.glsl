// Heart Equation 3D - zwei Herz-Platten senkrecht zueinander (gemeinsame y-Achse),
// Rotation um alle drei Achsen mit zufaellig wechselnden Geschwindigkeiten
// (inkl. Stillstand-Phasen), Bewegung im Raum, einstellbare Farben.
// Formel: y = |x|^(2/3) + 0.9*sin(k*x)*sqrt(3-x^2), k pendelt 0..100.
//
// SHADERTOY-EINRICHTUNG (zwei Tabs):
//   Buffer A: Code aus dem Abschnitt "TAB: BUFFER A"; iChannel0 = Buffer A
//             (Selbstreferenz), iChannel1 = Music (optional - ohne Musik
//             rotieren die Achsen mit BASIS-Tempo).
//   Image:    Code aus dem Abschnitt "TAB: IMAGE"; iChannel0 = Buffer A.
// In LumiViz: heart_equation_3d.lvfx laedt beide Paesse fertig verdrahtet.

// ============================= TAB: BUFFER A ================================
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

// =============================== TAB: IMAGE =================================
// Raymarcht die zwei Herz-Platten; liest die Winkel aus Buffer A (iChannel0).

// ---- STELLSCHRAUBEN --------------------------------------------------------
const float K_TEMPO    = 0.35;                  // Pendeltempo der Sinus-Fuellung
const float DICKE      = 0.06;                  // Plattendicke
const vec3  POS_AMP    = vec3(0.9, 0.5, 1.6);   // Bewegung im Raum: Amplitude je Achse
const vec3  POS_FREQ   = vec3(0.21, 0.17, 0.13);// ... und Tempo (inkommensurabel)
const float FARBE_A    = 0.95;                  // Basisfarbton Herz A (Palette 0..1, ~rosa)
const float FARBE_B    = 0.60;                  // Basisfarbton Herz B (~blau)
const float FARB_DRIFT = 0.02;                  // Farbton-Drift pro Sekunde (0 = fest)
// ----------------------------------------------------------------------------

#define R(a) mat2(cos(a), sin(a), -sin(a), cos(a))

vec3 palette(float t) { return 0.55 + 0.45*cos(6.28318*(t + vec3(0.0, 0.33, 0.67))); }

// 2D-Herzregion (<0 innen): Band um die Grundlinie, auf den Definitionsbereich geklemmt
float herzRegion(vec2 q)
{
    float base = pow(abs(q.x), 2.0/3.0);
    float env  = 0.9*sqrt(max(3.0 - q.x*q.x, 0.0));
    float d    = abs(q.y + 0.3 - base) - env;    // Welt-y ist um -0.3 zentriert
    return max(d, abs(q.x) - 1.7320508);
}

vec3 gWinkel;
vec3 gPos;

vec3 dreh(vec3 p)                                // Welt -> Objekt (inverse Rotation)
{
    p.xy *= R(-gWinkel.z);
    p.xz *= R(-gWinkel.y);
    p.yz *= R(-gWinkel.x);
    return p;
}

float map(vec3 p)
{
    p = dreh(p - gPos);
    float dA = max(herzRegion(p.xy),           abs(p.z) - DICKE);  // Herz A: xy-Ebene
    float dB = max(herzRegion(vec2(p.z, p.y)), abs(p.x) - DICKE);  // Herz B: zy-Ebene
    return min(dA, dB) * 0.5;                    // Region ist Schranke -> Drossel
}

void mainImage(out vec4 fragColor, in vec2 fragCoord)
{
    vec2 uv = (fragCoord - 0.5*iResolution.xy)/iResolution.y;
    gWinkel = texelFetch(iChannel0, ivec2(0,0), 0).xyz;
    gPos    = POS_AMP * sin(iTime * POS_FREQ);   // Bewegung im Raum (glatte Bahn)

    float kk = 50.0 - 50.0*cos(iTime*K_TEMPO);

    vec3 ro = vec3(0.0, 0.0, -7.5);
    vec3 rd = normalize(vec3(uv, 1.4));

    float t = 0.0, glow = 0.0, hit = -1.0;
    for (int i = 0; i < 90; i++)
    {
        float d = map(ro + rd*t);
        glow += 0.002/(0.02 + abs(d));
        if (d < 0.002) { hit = t; break; }
        if (t > 22.0) break;
        t += d;
    }

    float drift = iTime * FARB_DRIFT;
    vec3 col = vec3(0.0);
    if (hit > 0.0)
    {
        vec3 p = dreh(ro + rd*hit - gPos);
        float istA = step(abs(p.z), abs(p.x));   // A, wenn der Punkt in der xy-Platte liegt
        float lx   = mix(p.z, p.x, istA);        // lokale Querkoordinate der Platte
        vec2  q    = vec2(lx, p.y);

        float streifen = 0.6 + 0.4*sin(kk*lx);   // die Sinus-Fuellung als Textur
        float saum     = smoothstep(-0.25, 0.0, herzRegion(q));  // Herzrand betonen
        vec3  basis    = palette(mix(FARBE_B, FARBE_A, istA) + drift + 0.03*lx);

        col = basis * (0.25 + 0.75*streifen) + basis * saum * 0.8;
    }
    col += glow * 0.06 * palette(0.5*(FARBE_A + FARBE_B) + drift);

    col = 1.0 - exp(-col*1.3);
    fragColor = vec4(pow(col, vec3(1.0/2.2)), 1.0);
}
