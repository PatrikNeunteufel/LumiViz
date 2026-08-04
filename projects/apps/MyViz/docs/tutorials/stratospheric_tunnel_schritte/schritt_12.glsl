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
const float NEON_ANTEIL  = 0.45;   // Anteil beleuchteter Fugen (0..1)
const float NEON_STAERKE = 0.012;  // Helligkeit der Neon-Streifen
const float SCHEINWERFER = 1.6;    // Kamera-Scheinwerfer
const float RING_ABSTAND = 9.0;    // Abstand der Ring-Lichter (z)
const float RING_STAERKE = 0.35;   // Helligkeit der Ring-Lichter
const float FENSTERLICHT = 0.8;    // einfallendes Aussenlicht
const float KURVE = 0.8;    // seitliche Auslenkung des Pfads
const float TEMPO = 1.0;    // Gesamttempo der Fahrt
const float ROLL = 1.2;    // Banking-Staerke in Kurven
// ---- STELLSCHRAUBEN (erweitert) --------------------------------------------
const float GABEL_PERIODE = 40.0;   // Abstand der Vergabelungen (z)
const float GABEL_WEITE   = 1.6;    // maximale Ast-Auslenkung
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

// Cosinus-Palette - kraeftige, langsam rotierende Farben
vec3 pal(float t)
{
    return 0.5 + 0.5 * cos(TAU * (t + vec3(0.0, 0.33, 0.67)));
}

// NEU: das Tunnelzentrum am Ort z - zwei inkommensurable Sinus-Wellen
vec2 pfad(float z)
{
    return vec2(sin(z * 0.18), sin(z * 0.121)) * KURVE;
}

// NEU: wie weit die beiden Aeste am Ort z auseinanderliegen (0 = ein Tunnel)
float gabel(float z)
{
    float zz = fract(z / GABEL_PERIODE);
    return GABEL_WEITE * smoothstep(0.12, 0.38, zz)
                       * (1.0 - smoothstep(0.62, 0.88, zz));
}

// NEU: welchen Ast die Kamera in dieser Gabel-Zelle nimmt (-1 oder +1)
float gabelSeite(float z)
{
    return hash21(vec2(floor(z / GABEL_PERIODE), 5.2)) < 0.5 ? -1.0 : 1.0;
}

// NEU: der Kamera-Pfad = Haupt-Pfad + gewaehlter Ast
vec2 kameraPfad(float z)
{
    return pfad(z) + vec2(gabelSeite(z) * gabel(z), 0.0);
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

// GEAENDERT: die Spiegel-Faltung im SDF - eine Zeile macht zwei Tunnel
float mapTunnel(vec3 p)
{
    vec2 q = p.xy - pfad(p.z);
    q.x = abs(q.x) - gabel(p.z);      // Faltung: Aeste bei x = +-gabel(z)
    float w = atan(q.y, q.x);
    return RADIUS - wandRelief(w, p.z) - length(q);
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

vec3 neonEmission(float w, float z)
{
    float fu  = w * ROEHREN / TAU;                 // Fugen bei ganzzahligem fu
    float gid = mod(floor(fu + 0.5), ROEHREN);     // Fugen-Index (nahtfrei)
    float dg  = abs(fract(fu + 0.5) - 0.5);        // Abstand zur Fuge (0..0.5)
    float ad  = dg * TAU * RADIUS / ROEHREN;       // ... in Wand-Einheiten

    float aktiv = step(hash21(vec2(gid, 1.3)), NEON_ANTEIL);
    aktiv *= 0.6 + 0.4 * sin(iTime * (0.4 + 0.7 * hash21(vec2(gid, 8.2)))
                             + TAU * hash21(vec2(gid, 4.4)));

    vec3 farbe = pal(hash21(vec2(gid, 2.6)) * 0.4 + 0.55 + iTime * 0.015);

    float band = 0.35 + 0.65 * smoothstep(0.25, 0.6, vnoise(vec2(gid * 7.3, z * 0.25)));

    return farbe * aktiv * band * NEON_STAERKE / (0.0015 + ad * ad * 60.0);
}

float ringPuls(float id)
{
    float ph = hash21(vec2(id, 31.7));                // eigene Phase
    float sp = 0.4 + 0.8 * hash21(vec2(id, 17.3));    // eigenes Tempo
    float wv = 0.5 + 0.5 * sin(TAU * (iTime * sp * 0.20 + ph));
    return smoothstep(0.55, 0.95, wv) * (0.3 + 0.7 * hash21(vec2(id, 5.1)));
}

vec3 ringEmission(float z)
{
    float rz = z / RING_ABSTAND;
    float id = floor(rz + 0.5);                       // Index des naechsten Rings
    float dz = (fract(rz + 0.5) - 0.5) * RING_ABSTAND; // Abstand zu ihm (z)

    vec3 farbe = pal(hash21(vec2(id, 2.7)) * 0.5 + iTime * 0.01);
    return farbe * ringPuls(id) * RING_STAERKE / (0.03 + dz * dz * 14.0);
}

vec3 shadeWand(vec3 p, vec3 ro, vec3 n, float w)
{
    float tex = fbm(vec2(cos(w) * 3.1 + p.z * 0.9, sin(w) * 3.1 + p.z * 0.63));
    vec3 basis = mix(vec3(0.045, 0.055, 0.085), vec3(0.10, 0.12, 0.17), tex);

    // (1) Scheinwerfer der Kamera
    vec3 zk = ro - p;
    float dk = max(length(zk), 1e-3);
    float dif = max(dot(n, zk / dk), 0.0);
    vec3 col = basis * (SCHEINWERFER * dif / (1.0 + dk * dk * 0.12) + 0.02);

    // (2) Neon-Streifen in den Fugen
    col += neonEmission(w, p.z);

    // (3) Ring-Lichter in Intervallen
    col += ringEmission(p.z);

    // (4) Aussenlicht durch das GEGENUEBERLIEGENDE Fenster
    float einfall = fensterMask(w + 3.14159265, p.z);
    col += vec3(0.30, 0.38, 0.60) * FENSTERLICHT * einfall;

    return col;
}

// GEAENDERT: die Kamera bekommt ihre Choreografie
void kamera(vec2 uv, out vec3 ro, out vec3 rd)
{
    float zt = iTime * TEMPO;

    // VORTRIEB: Grundtempo + Sinus-Anteil.
    // Geschwindigkeit = 1.1 + 1.4*cos(zt*0.2) => wird zeitweise negativ:
    // die Fahrt bremst weich ab, setzt kurz zurueck, zieht wieder an
    float zpos = zt * 1.1 + sin(zt * 0.20) * 7.0;

    ro = vec3(kameraPfad(zpos), zpos);

    float za = zpos + 2.0;
    vec3 ta = vec3(kameraPfad(za), za);
    vec3 fw = normalize(ta - ro);

    // BANKING: seitliche Pfad-Aenderung kippt die Kamera in die Kurve,
    // dazu ein langsamer Eigen-Roll als Ballett-Anteil
    float kruemm = kameraPfad(zpos + 1.5).x - kameraPfad(zpos - 1.5).x;
    float roll = -kruemm * ROLL + 0.12 * sin(zt * 0.13);

    // die "Hoch"-Richtung wird um roll gekippt - der Rest bleibt gleich
    vec3 up0 = vec3(sin(roll), cos(roll), 0.0);
    vec3 rt = normalize(cross(up0, fw));
    vec3 up = cross(fw, rt);

    rd = normalize(fw * 1.4 + rt * uv.x + up * uv.y);
}

void mainImage(out vec4 fragColor, in vec2 fragCoord)
{
    vec2 uv = (fragCoord - 0.5 * iResolution.xy) / iResolution.y;

    vec3 ro, rd;
    kamera(uv, ro, rd);

    float t = march(ro, rd);
    vec3 p = ro + rd * t;

    // Wand-Koordinaten im Pfad-System (dieselbe Verschiebung wie mapTunnel!)
    vec2 q = p.xy - pfad(p.z);
    q.x = abs(q.x) - gabel(p.z);      // dieselbe Faltung wie mapTunnel
    float w = atan(q.y, q.x);

    vec3 n = wandNormale(p);
    vec3 color = shadeWand(p, ro, n, w);

    float F = fensterMask(w, p.z);
    color = mix(color, aussenRaum(rd), F);

    fragColor = vec4(color, 1.0);
}
