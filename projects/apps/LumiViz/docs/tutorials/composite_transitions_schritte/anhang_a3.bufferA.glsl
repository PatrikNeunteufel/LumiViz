// ============================================================================
// "Composite: Transitions" - Kristall-Terrain <-> Juggernaut, mit Maske,
// Morph und Zustandsmaschine. Endstand des Tutorials (Schritt 12).
// Welten: Skelette aus Crystal-Lights- und Juggernaut-Shader-Tutorial.
// Kern-Modus braucht keine iChannels ausser Buffer A (Anhang A: + Music).
// ============================================================================

// ---- STELLSCHRAUBEN --------------------------------------------------------
const float HALTEDAUER  = 24.0;  // GEAENDERT: Timer ist jetzt Fallback bei Stille
const float MIN_HALTE   = 3.0;   // NEU: Mindest-Verweilzeit (Hysterese)
const float BLENDEDAUER = 4.0;   // s je Uebergang
const float COOLDOWN    = 2.0;   // s Sperrzeit nach der Blende
const int   BLENDART    = 1;     // 0 Crossfade  1 Noise-Wipe  2 radial  3 Richtung
const float SAUM        = 0.06;  // halbe Breite des Misch-Saums
const float GLUT        = 1.2;   // Staerke des Gluehsaums an der Front
const float KICK        = 0.35;  // Belichtungs-Kick im Wechselmoment
const float A_TIEFE     = -1.8;  // Welt A: y der Lichtebene
const float A_ZELLE     = 1.7;   // Welt A: Rasterabstand der Leuchtkoerper
const float B_RADIUS    = 6.0;   // Welt B: Radius des Molochs
const float B_ZELLE1    = 2.6;   // Welt B: grosse Platten
const float B_ZELLE2    = 0.9;   // Welt B: Raster der Positionslichter
const float B_ZELLE3    = 0.32;  // Welt B: feine Rillen
// ----------------------------------------------------------------------------

#define R(a) mat2(cos(a), sin(a), -sin(a), cos(a))

// ---- Meta-Zustand: EIN Satz Regler fuer beide Welten -----------------------
// (setzt das Image je Frame aus dem Blendwert t - VOR Kamera- und Welt-Aufrufen)
float gMorph      = 0.0;                       // 0 = Welt A haelt .. 1 = Welt B haelt
float gDunst      = 0.0018;                    // Dunstdichte (geteilt)
vec3  gDunstFarbe = vec3(0.05, 0.07, 0.12);    // Dunstfarbe  (geteilt)
vec3  b_sonne     = vec3(0.0, 0.3, 1.0);       // setzt kamera() je Frame
float gLautheit   = 0.0;                       // traege Lautheit (setzt das Image aus Buffer A)

// ---- gemeinsamer Zufalls-Baukasten -----------------------------------------
float hash21(vec2 p) { return fract(sin(dot(p, vec2(127.1, 311.7))) * 43758.5453); }

vec2 hash22(vec2 p)
{
    return fract(sin(vec2(dot(p, vec2(127.1, 311.7)),
                          dot(p, vec2(269.5, 183.3)))) * 43758.5453);
}

float hash31(vec3 p) { return fract(sin(dot(p, vec3(127.1, 311.7, 74.7))) * 43758.5453); }

float vnoise(vec2 p)
{
    vec2 i = floor(p), f = fract(p);
    vec2 u = f * f * (3.0 - 2.0 * f);
    return mix(mix(hash21(i),              hash21(i + vec2(1, 0)), u.x),
               mix(hash21(i + vec2(0, 1)), hash21(i + vec2(1, 1)), u.x), u.y);
}

float fbm(vec2 p)
{
    float v = 0.0, a = 0.5;
    for (int i = 0; i < 4; i++) { v += a * vnoise(p); p = p * 2.03 + 11.7; a *= 0.5; }
    return v;
}

float smin(float a, float b, float k)
{
    float h = max(k - abs(a - b), 0.0) / k;
    return min(a, b) - h * h * k * 0.25;
}
float smax(float a, float b, float k) { return -smin(-a, -b, k); }

// ============================================================================
// WELT A: Kristall-Terrain (Skelett; Vollausbau: Crystal-Lights-Tutorial)
// ============================================================================

float a_terrain(vec2 p) { return (fbm(p * 0.35) - 0.45) * 2.4; }

float a_march(vec3 ro, vec3 rd)
{
    float t = 0.0;
    for (int i = 0; i < 90; i++) {
        vec3 p = ro + rd * t;
        float d = p.y - a_terrain(p.xz);
        if (d < 0.002 + 0.002 * t) return t;
        if (t > 30.0) break;
        t += d * 0.45;
    }
    return -1.0;
}

vec3 a_normal(vec2 p, float t)
{
    vec2 e = vec2(0.014 * (1.0 + t * 0.12), 0.0);
    return normalize(vec3(a_terrain(p - e.xy) - a_terrain(p + e.xy),
                          2.0 * e.x,
                          a_terrain(p - e.yx) - a_terrain(p + e.yx)));
}

vec3 a_lampColor(float t)
{
    vec3 bunt = 0.55 + 0.45 * cos(6.28318 * (t + vec3(0.0, 0.33, 0.67)));
    return mix(bunt, vec3(1.0, 0.16, 0.10), gMorph * 0.7);   // Morph: Richtung Welt B
}

float a_blink(vec2 id)
{
    float ph = hash21(id + 31.7);
    float sp = 0.35 + 0.75 * hash21(id + 17.3);
    float w  = 0.5 + 0.5 * sin(6.28318 * (iTime * sp * 0.25 + ph));
    return smoothstep(0.70, 0.97, w) * (0.2 + 0.8 * hash21(id + 5.1));
}

vec3 a_lights(vec2 q)
{
    vec2 base = floor(q / A_ZELLE);
    vec3 acc = vec3(0.0);
    for (int y = -1; y <= 1; y++)
    for (int x = -1; x <= 1; x++) {
        vec2 id = base + vec2(float(x), float(y));
        vec2 c  = (id + 0.5 + 0.7 * (hash22(id + 7.0) - 0.5)) * A_ZELLE;
        vec2 d  = q - c;
        float hell = a_blink(id) + 0.05;
        acc += a_lampColor(hash21(id)) * hell / (0.02 + dot(d, d) * 14.0);
    }
    return acc * 0.05;
}

vec3 a_himmel(vec3 rd)
{
    return mix(vec3(0.10, 0.12, 0.22), vec3(0.02, 0.03, 0.08),
               clamp(rd.y * 3.0, 0.0, 1.0));
}

vec3 a_render(vec3 ro, vec3 rd)          // liefert LINEARE Farbe
{
    float t = a_march(ro, rd);
    vec3 col;
    if (t > 0.0) {
        vec3 p = ro + rd * t;
        vec3 n = a_normal(p.xz, t);

        vec3 rr = refract(rd, n, 1.0 / 1.45);
        if (dot(rr, rr) < 0.5) rr = rd;
        float tE = (A_TIEFE - p.y) / min(rr.y, -0.05);
        vec2 q = (p + rr * tE).xz;

        float dicke = max(p.y - A_TIEFE, 0.0);
        vec3 T = exp(-dicke * vec3(0.85, 0.30, 0.16) * 0.55);

        col = a_lights(q) * T;

        float fres = pow(1.0 - max(dot(n, -rd), 0.0), 3.0);
        col += fres * vec3(0.35, 0.50, 0.65) * 0.5;
        col += max(dot(n, normalize(vec3(0.4, 0.75, -0.5))), 0.0)
             * vec3(0.10, 0.14, 0.20);

        col = mix(col, gDunstFarbe, 1.0 - exp(-gDunst * t * t));
    } else {
        col = a_himmel(rd);
    }
    return col;
}

// ============================================================================
// WELT B: Juggernaut, dark (Skelett; Vollausbau: Juggernaut-Tutorial)
// ============================================================================

vec3 b_gedreht(vec3 p)
{
    p.yz *= R(0.42);
    p.xz *= R(iTime * 0.02);
    return p;
}

float b_fugen(vec3 p, float zelle)
{
    vec3 q = abs(fract(p / zelle) - 0.5) * zelle;
    return zelle * 0.5 - max(q.x, max(q.y, q.z));
}

float b_map(vec3 p)
{
    vec3 q = b_gedreht(p);
    float d = length(q) - B_RADIUS;

    vec3 z1 = floor(q / B_ZELLE1);
    d -= (hash31(z1) - 0.5) * 0.35;

    float slab   = b_fugen(q, B_ZELLE1) - 0.07;
    float schale = (B_RADIUS - 0.30) - length(q);
    d = smax(d, -max(slab, schale), 0.05);

    vec3 h = pow(abs(2.0 * fract(q / B_ZELLE3) - 1.0), vec3(3.0));
    d += 0.02 * (h.x + h.y + h.z) * 0.33;

    return d;
}

float b_march(vec3 ro, vec3 rd, out float glow)
{
    glow = 0.0;
    float t = 0.0;
    for (int i = 0; i < 110; i++) {
        vec3 p = ro + rd * t;
        float d = b_map(p);
        glow += 0.012 / (0.05 + d * d);
        if (d < 0.001 + 0.001 * t) return t;
        if (t > 40.0) break;
        t += d * 0.5;
    }
    return -1.0;
}

vec3 b_normal(vec3 p)
{
    const vec2 e = vec2(0.004, 0.0);
    return normalize(vec3(b_map(p + e.xyy) - b_map(p - e.xyy),
                          b_map(p + e.yxy) - b_map(p - e.yxy),
                          b_map(p + e.yyx) - b_map(p - e.yyx)));
}

float b_fenster(vec3 q)
{
    vec3 z = floor(q / B_ZELLE2);
    float an = step(0.93, hash31(z + 29.0));
    float sp = 0.10 + 0.25 * hash31(z + 3.0);
    float w  = 0.5 + 0.5 * sin(6.28318 * (iTime * sp + hash31(z + 11.0)));
    float blink = 0.25 + 0.75 * smoothstep(0.55, 0.95, w);
    vec3 lokal = (fract(q / B_ZELLE2) - 0.5) * B_ZELLE2;
    float punkt = 1.0 - smoothstep(0.06, 0.24, length(lokal));
    return an * blink * punkt;
}

vec3 b_himmel(vec3 rd)
{
    vec3 col = mix(vec3(0.030, 0.028, 0.045), vec3(0.010, 0.012, 0.022),
                   clamp(rd.y * 1.5 + 0.5, 0.0, 1.0));
    float s = max(dot(rd, b_sonne), 0.0);
    col += pow(s, 30.0) * vec3(0.35, 0.42, 0.60) * 1.2;
    col += pow(s, 5.0)  * vec3(0.35, 0.42, 0.60) * 0.12;
    return col;
}

vec3 b_shade(vec3 p, vec3 rd)
{
    vec3 n = b_normal(p);
    vec3 albedo = vec3(0.16, 0.17, 0.19);

    float dif = max(dot(n, b_sonne), 0.0);
    float amb = 0.5 + 0.5 * n.y;
    vec3 col = albedo * (dif * vec3(0.30, 0.38, 0.55) * 0.6
                       + amb * vec3(0.020, 0.025, 0.045));

    float rim = pow(1.0 - max(dot(n, -rd), 0.0), 3.0);
    col += rim * vec3(0.30, 0.38, 0.55) * 0.55;

    // Positionslichter - der Morph laesst sie in Richtung Welt A schielen
    vec3 lichtFarbe = mix(vec3(0.45, 0.75, 1.00), vec3(1.0, 0.12, 0.08), gMorph);
    col += b_fenster(b_gedreht(p)) * lichtFarbe * 1.4;

    return col;
}

vec3 b_render(vec3 ro, vec3 rd)          // liefert LINEARE Farbe
{
    float glow;
    float t = b_march(ro, rd, glow);
    vec3 col;
    if (t > 0.0) {
        col = b_shade(ro + rd * t, rd);
        col = mix(col, gDunstFarbe, 1.0 - exp(-gDunst * t * t));
    } else {
        col = b_himmel(rd);
    }
    col += glow * 0.05 * vec3(0.28, 0.34, 0.55);
    return col;
}

// ============================================================================
// EINE Kamera fuer beide Welten
// ============================================================================

void kamera(vec2 uv, out vec3 ro, out vec3 rd)
{
    float zt = iTime;

    float wink = zt * 0.03 + 1.2 * sin(zt * 0.021);   // gemeinsame Orbit-Basis

    float dist  = mix(7.0, 11.5, gMorph);             // Welt-Ziele, mitgeblendet
    float hoehe = mix(3.2, -1.4, gMorph);
    vec3  ta    = vec3(0.0, mix(0.0, 1.6, gMorph), 0.0);
    float brenn = mix(1.4, 1.1, gMorph);

    ro = vec3(sin(wink) * dist, hoehe, cos(wink) * dist);
    ro.y = max(ro.y, mix(a_terrain(ro.xz) + 1.5, -4.0, gMorph));   // Boden-Anker

    vec3 fw = normalize(ta - ro);
    vec3 rt = normalize(cross(vec3(0.0, 1.0, 0.0), fw));
    vec3 up = cross(fw, rt);
    rd = normalize(fw * brenn + rt * uv.x + up * uv.y);

    b_sonne = normalize(fw + vec3(0.0, 0.35, 0.0));   // Gegenlicht der Welt B
}

// ==== Ende Common - ab hier der Pass-eigene Code =========================

// ============================================================================
// BUFFER A (Pruefstand A1) - Zustandsmaschine + Audio-Zustand
// Pixel (0,0): x = phase, y = welt, z = haltezeit, w = cooldown
// Pixel (1,0): x = glatterBass, y = beatEnv, z = energieVorrat, w = glatteLautheit
// iChannel0: Buffer A (Selbstreferenz), iChannel1: Music
// ============================================================================


float bandLevel(float lo, float hi)
{
    float sum = 0.0;
    const int N = 12;
    for (int i = 0; i < N; i++) {
        float x = mix(lo, hi, (float(i) + 0.5) / float(N));
        sum += texture(iChannel1, vec2(x, 0.25)).x;
    }
    return sum / float(N);
}

void mainImage(out vec4 fragColor, in vec2 fragCoord)
{
    vec4 zust  = texelFetch(iChannel0, ivec2(0, 0), 0);
    vec4 audio = texelFetch(iChannel0, ivec2(1, 0), 0);
    if (iFrame == 0) { zust = vec4(0.0); audio = vec4(0.0); }

    float dt = clamp(iTimeDelta, 1.0 / 240.0, 1.0 / 24.0);

    // ---- AUDIO-ZUSTAND: das B3-Muster der Schablone, voll ausgefuehrt ------
    float bass  = bandLevel(0.00, 0.05);
    float vol   = bandLevel(0.00, 0.70);

    float glatt = mix(audio.x, bass, 0.10);            // Tiefpass ueber die Zeit
    // LumiViz-Anpassung (Regel aus Anhang B1, NICHT der Shadertoy-Text):
    // Die dB-FFT-Zeile des Standalone-Testsignals saettigt bei 1.0
    // (Sonde S67) - `bass` steht damit konstant auf 1.0, und der
    // adaptive Trigger `bass > glatt*1.35 + 0.02` kann nie feuern
    // (im Probe-Lauf blieb nur der Timer-Fallback). B1 nennt den
    // Ausweg: schlag aus dem App-Uniform `beat` speisen (0/1 je Frame,
    // BeatEstimator der App). Auf shadertoy.com gilt weiter der
    // Tutorial-Text (adaptiver Trigger auf der Music-Textur).
    float schlag = step(0.5, beat);
    float env   = max(audio.y * 0.90, schlag);         // zuendet hart, klingt weich aus

    float vorrat = audio.z + bass * dt * 0.4;          // laedt kontinuierlich ...
    float drop   = schlag * step(1.5, vorrat);         // ... Kick nach viel Ruhe = DROP
    vorrat *= 1.0 - schlag * 0.8;                      // jeder Schlag entlaedt

    float lautheit = mix(audio.w, vol, 0.05);          // sehr traege Lautheit

    // ---- WECHSEL-ZUSTAND: die Maschine aus Schritt 11, Trigger = Beat ------
    float phase = zust.x, welt = zust.y, halte = zust.z, cool = zust.w;

    if (phase <= 0.0) {
        halte += dt;
        // DER Trigger-Tausch: Beat schaltet den Welt-Wechsel (Timer als Fallback)
        bool wechsel = (schlag > 0.5 && halte > MIN_HALTE) || halte > HALTEDAUER;
        if (wechsel && cool <= 0.0) phase = 0.0001;
    } else {
        // Mapping [3]: das Blendtempo atmet mit dem (geglaetteten!) Bass
        phase += dt / BLENDEDAUER * (0.6 + 1.2 * min(glatt * 2.5, 1.5));
        if (phase >= 1.0) {
            phase = 0.0; welt = 1.0 - welt;
            halte = 0.0; cool = COOLDOWN;
        }
    }
    cool = max(cool - dt, 0.0);

    // ---- beide Zustandspixel schreiben -------------------------------------
    if (ivec2(fragCoord) == ivec2(1, 0))
        fragColor = vec4(glatt, env, vorrat, lautheit);
    else
        fragColor = vec4(phase, welt, halte, cool);
}
