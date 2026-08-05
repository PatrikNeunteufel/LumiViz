// heart_equation_3d — TAB 'Image' (nur DIESEN Code in den Image-Tab!)
// Verdrahtung: iChannel0 = Buffer A

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
