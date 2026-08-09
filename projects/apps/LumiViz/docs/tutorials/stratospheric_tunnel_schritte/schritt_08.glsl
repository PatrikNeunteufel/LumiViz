#define R(a) mat2(cos(a), sin(a), -sin(a), cos(a))

// ---- STELLSCHRAUBEN --------------------------------------------------------
const float RADIUS        = 1.0;    // Grundradius des Tunnels
const float ROEHREN       = 14.0;   // Roehren um den Umfang (ganzzahlig!)
const float ROEHREN_TIEFE = 0.10;   // Woelbung der Roehren
const float SPANT_ABSTAND = 4.0;    // Abstand der Spanten-Ringe (z)
const float SPANT_TIEFE   = 0.05;   // Hoehe der Spanten
const float RELIEF        = 0.05;   // organisches FBM-Relief
const float FENSTER_SPALTEN = 6.0;   // Fensterspalten um den Umfang (ganzzahlig!)
const float FENSTER_ABSTAND = 5.0;   // Fensterabstand entlang z
const float FENSTER_DICHTE  = 0.55;  // Anteil der Zellen mit Fenster (0..1)
const float FENSTER_DYN     = 0.35;  // dynamisches Oeffnen/Schliessen (0 = statisch)
// ---- STELLSCHRAUBEN (erweitert) --------------------------------------------
const float NEON_ANTEIL  = 0.45;   // Anteil beleuchteter Fugen (0..1)
const float NEON_STAERKE = 0.012;  // Helligkeit der Neon-Streifen
const float SCHEINWERFER = 1.6;    // Kamera-Scheinwerfer (ersetzt die 1.6 inline)
// ----------------------------------------------------------------------------

const float TAU = 6.28318530;

// Hash: Gitterpunkt -> deterministische "Zufallszahl" 0..1
float hash21(vec2 p) { return fract(sin(dot(p, vec2(127.1, 311.7))) * 43758.5453); }

vec2 hash22(vec2 p)
{
    return fract(sin(vec2(dot(p, vec2(127.1, 311.7)),
                          dot(p, vec2(269.5, 183.3)))) * 43758.5453);
}

// Value-Noise: weich interpolierte Zufallswerte auf einem Einheitsgitter
float vnoise(vec2 p)
{
    vec2 i = floor(p), f = fract(p);
    vec2 u = f * f * (3.0 - 2.0 * f);
    return mix(mix(hash21(i),              hash21(i + vec2(1, 0)), u.x),
               mix(hash21(i + vec2(0, 1)), hash21(i + vec2(1, 1)), u.x), u.y);
}

// Fraktales Rauschen: vier Oktaven genuegen fuer Wandstruktur
float fbm(vec2 p)
{
    float v = 0.0, a = 0.5;
    for (int i = 0; i < 4; i++) { v += a * vnoise(p); p = p * 2.03 + 11.7; a *= 0.5; }
    return v;
}

// NEU: Cosinus-Palette - kraeftige, langsam rotierende Farben
vec3 pal(float t)
{
    return 0.5 + 0.5 * cos(TAU * (t + vec3(0.0, 0.33, 0.67)));
}

float wandRelief(float w, float z)
{
    float roehre = ROEHREN_TIEFE * (0.5 - 0.5 * cos(w * ROEHREN));

    float sz    = abs(fract(z / SPANT_ABSTAND) - 0.5) * SPANT_ABSTAND;
    float spant = SPANT_TIEFE * smoothstep(0.35, 0.0, sz);

    float n = fbm(vec2(cos(w) * 1.6 + z * 0.45, sin(w) * 1.6 + z * 0.31));

    return roehre + spant + RELIEF * (n - 0.5) * 2.0;
}

float fensterMask(float w, float z)
{
    float wu = fract(w / TAU + 0.5);              // 0..1 um den Umfang (nahtfrei)
    vec2 zelle = vec2(wu * FENSTER_SPALTEN, z / FENSTER_ABSTAND);
    vec2 id = floor(zelle);

    if (hash21(id + 3.1) > FENSTER_DICHTE) return 0.0;

    vec2 c = fract(zelle) - 0.5;
    c.x *= TAU * RADIUS / FENSTER_SPALTEN;
    c.y *= FENSTER_ABSTAND;

    float o = 0.8 + FENSTER_DYN *
              sin(iTime * (0.15 + 0.25 * hash21(id + 9.4)) + TAU * hash21(id));
    o = clamp(o, 0.0, 1.0);

    vec2 halb = vec2(0.30, 0.85) * o;
    float d = max(abs(c.x) - halb.x, abs(c.y) - halb.y);
    return smoothstep(0.06, -0.06, d);
}

float mapTunnel(vec3 p)
{
    float w = atan(p.y, p.x);
    return RADIUS - wandRelief(w, p.z) - length(p.xy);
}

float march(vec3 ro, vec3 rd)
{
    float t = 0.02;
    for (int i = 0; i < 120; i++) {
        float d = mapTunnel(ro + rd * t);
        if (d < 0.0015 + 0.001 * t) break;
        t += d * 0.7;
        if (t > 60.0) break;
    }
    return min(t, 60.0);
}

vec3 wandNormale(vec3 p)
{
    vec2 e = vec2(0.004, 0.0);
    return normalize(vec3(mapTunnel(p + e.xyy) - mapTunnel(p - e.xyy),
                          mapTunnel(p + e.yxy) - mapTunnel(p - e.yxy),
                          mapTunnel(p + e.yyx) - mapTunnel(p - e.yyx)));
}

vec3 aussenRaum(vec3 rd)
{
    float h = rd.y;

    vec3 col = mix(vec3(0.03, 0.05, 0.11), vec3(0.005, 0.01, 0.03),
                   clamp(h * 2.0 + 0.5, 0.0, 1.0));

    col += vec3(0.30, 0.14, 0.34) * exp(-abs(h + 0.12) * 8.0);
    col += vec3(0.95, 0.45, 0.15) * exp(-abs(h + 0.15) * 40.0) * 0.7;

    vec2 su = rd.xy / (abs(rd.z) + 0.4);
    float s = hash21(floor(su * 48.0));
    col += vec3(0.9) * smoothstep(0.994, 1.0, s) * clamp(h * 3.0 + 0.5, 0.0, 1.0);

    for (int i = 0; i < 3; i++) {
        float dist = fract(float(i) / 3.0 - iTime * 0.06);
        vec2 uv4 = su * (1.5 + 26.0 * dist) + hash22(vec2(float(i), 3.7)) * 37.0;
        float sn = hash21(floor(uv4));
        col += vec3(0.55, 0.70, 1.00) * (1.0 - dist)
             * smoothstep(0.90, 1.0, sn)
             * smoothstep(0.32, 0.0, length(fract(uv4) - 0.5));
    }
    return col;
}

// NEU: Neon-Emission am Wandort (w, z)
vec3 neonEmission(float w, float z)
{
    float fu  = w * ROEHREN / TAU;                 // Fugen bei ganzzahligem fu
    float gid = mod(floor(fu + 0.5), ROEHREN);     // Fugen-Index (nahtfrei)
    float dg  = abs(fract(fu + 0.5) - 0.5);        // Abstand zur Fuge (0..0.5)
    float ad  = dg * TAU * RADIUS / ROEHREN;       // ... in Wand-Einheiten

    // nur ein Teil der Fugen traegt Neon; dazu langsames Flackern je Fuge
    float aktiv = step(hash21(vec2(gid, 1.3)), NEON_ANTEIL);
    aktiv *= 0.6 + 0.4 * sin(iTime * (0.4 + 0.7 * hash21(vec2(gid, 8.2)))
                             + TAU * hash21(vec2(gid, 4.4)));

    // Farbe je Fuge aus der Palette, die Palette selbst rotiert langsam
    vec3 farbe = pal(hash21(vec2(gid, 2.6)) * 0.4 + 0.55 + iTime * 0.015);

    // laengs unterbrochene Baender (Masken-Zitat: nicht ueberall leuchtet es)
    float band = 0.35 + 0.65 * smoothstep(0.25, 0.6, vnoise(vec2(gid * 7.3, z * 0.25)));

    return farbe * aktiv * band * NEON_STAERKE / (0.0015 + ad * ad * 60.0);
}

// NEU: das komplette Wand-Material (ersetzt das Shading in mainImage)
vec3 shadeWand(vec3 p, vec3 ro, vec3 n, float w)
{
    float tex = fbm(vec2(cos(w) * 3.1 + p.z * 0.9, sin(w) * 3.1 + p.z * 0.63));
    vec3 basis = mix(vec3(0.045, 0.055, 0.085), vec3(0.10, 0.12, 0.17), tex);

    // (1) Scheinwerfer der Kamera
    vec3 zk = ro - p;
    float dk = max(length(zk), 1e-3);
    float dif = max(dot(n, zk / dk), 0.0);
    vec3 col = basis * (SCHEINWERFER * dif / (1.0 + dk * dk * 0.12) + 0.02);

    // (2) Neon-Streifen in den Fugen (Emission)
    col += neonEmission(w, p.z);

    return col;
}

void mainImage(out vec4 fragColor, in vec2 fragCoord)
{
    vec2 uv = (fragCoord - 0.5 * iResolution.xy) / iResolution.y;

    vec3 ro = vec3(0.0, 0.0, iTime * 2.0);
    vec3 rd = normalize(vec3(uv, 1.4));

    float t = march(ro, rd);
    vec3 p = ro + rd * t;

    float w = atan(p.y, p.x);

    vec3 n = wandNormale(p);
    vec3 color = shadeWand(p, ro, n, w);

    float F = fensterMask(w, p.z);
    color = mix(color, aussenRaum(rd), F);

    fragColor = vec4(color, 1.0);
}
