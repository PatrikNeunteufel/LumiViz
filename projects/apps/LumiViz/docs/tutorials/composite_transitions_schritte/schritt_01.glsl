// ============================================================================
// COMPOSITE: TRANSITIONS - Schritt 1: Welt A als Skelett (Kristall-Terrain)
// Vollausbau und Herleitung: Crystal-Lights-Shader-Tutorial (gleicher Ordner)
// ============================================================================

// ---- STELLSCHRAUBEN --------------------------------------------------------
const float A_TIEFE = -1.8;   // y der Lichtebene unter dem Terrain
const float A_ZELLE = 1.7;    // Rasterabstand der Leuchtkoerper
// ----------------------------------------------------------------------------

#define R(a) mat2(cos(a), sin(a), -sin(a), cos(a))

// ---- gemeinsamer Zufalls-Baukasten (den teilen sich spaeter BEIDE Welten) --
float hash21(vec2 p) { return fract(sin(dot(p, vec2(127.1, 311.7))) * 43758.5453); }

vec2 hash22(vec2 p)
{
    return fract(sin(vec2(dot(p, vec2(127.1, 311.7)),
                          dot(p, vec2(269.5, 183.3)))) * 43758.5453);
}

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

// ---- WELT A: Kristall-Terrain (Skelett) ------------------------------------

float a_terrain(vec2 p) { return (fbm(p * 0.35) - 0.45) * 2.4; }

float a_march(vec3 ro, vec3 rd)
{
    float t = 0.0;
    for (int i = 0; i < 90; i++) {              // Skelett: 90 statt 150 Schritte
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
    return 0.55 + 0.45 * cos(6.28318 * (t + vec3(0.0, 0.33, 0.67)));
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

// liefert LINEARE Farbe - das Tonemapping macht am Ende das Composite
vec3 a_render(vec3 ro, vec3 rd)
{
    float t = a_march(ro, rd);
    vec3 col;
    if (t > 0.0) {
        vec3 p = ro + rd * t;
        vec3 n = a_normal(p.xz, t);

        vec3 rr = refract(rd, n, 1.0 / 1.45);       // Brechung in den Kristall
        if (dot(rr, rr) < 0.5) rr = rd;
        float tE = (A_TIEFE - p.y) / min(rr.y, -0.05);
        vec2 q = (p + rr * tE).xz;

        float dicke = max(p.y - A_TIEFE, 0.0);
        vec3 T = exp(-dicke * vec3(0.85, 0.30, 0.16) * 0.55);   // Beer-Lambert

        col = a_lights(q) * T;                       // gebrochenes Lampenlicht

        float fres = pow(1.0 - max(dot(n, -rd), 0.0), 3.0);
        col += fres * vec3(0.35, 0.50, 0.65) * 0.5;  // Himmel-Spiegelung
        col += max(dot(n, normalize(vec3(0.4, 0.75, -0.5))), 0.0)
             * vec3(0.10, 0.14, 0.20);               // Mond-Schimmer

        col = mix(col, vec3(0.05, 0.07, 0.12),       // Distanznebel
                  1.0 - exp(-0.0018 * t * t));
    } else {
        col = a_himmel(rd);
    }
    return col;
}

void a_kamera(vec2 uv, out vec3 ro, out vec3 rd)
{
    ro = vec3(0.0, 2.8, iTime * 0.8);               // Vorwaertsflug wie im Original
    rd = normalize(vec3(uv, 1.3));
    rd.yz *= R(-0.12);
}

void mainImage(out vec4 fragColor, in vec2 fragCoord)
{
    vec2 uv = (fragCoord - 0.5 * iResolution.xy) / iResolution.y;

    vec3 ro, rd;
    a_kamera(uv, ro, rd);
    vec3 col = a_render(ro, rd);

    // Abschluss - vorerst je Welt eigener; wird in Schritt 9 EINER fuer beide
    col = 1.0 - exp(-col * 1.5);
    col = pow(col, vec3(1.0 / 2.2));
    col *= 1.0 - 0.33 * dot(uv, uv);

    fragColor = vec4(col, 1.0);
}
