// ============================================================================
// SKELETT 1: Stratospheric Tunnel, kondensiert.
// Behalten:   Roehrenwand, Neonfugen, Fenster, Scheinwerfer, Vortrieb.
// Gestrichen: Pfad/Gabel, Spanten, Relief, Ringlichter, Fensterlicht,
//             Banking, Kamera-Choreografie, Politur (bis auf Notbelichtung).
// ============================================================================

// ---- STELLSCHRAUBEN: TUNNEL ------------------------------------------------
const float T_RADIUS  = 1.0;    // Grundradius der Roehre
const float T_ROEHREN = 12.0;   // Roehren um den Umfang (ganzzahlig!)
const float T_TIEFE   = 0.10;   // Woelbung der Roehren
const float T_SPALTEN = 6.0;    // Fensterspalten um den Umfang (ganzzahlig!)
const float T_ABSTAND = 5.0;    // Fensterabstand entlang z
const float T_DICHTE  = 0.55;   // Anteil der Zellen mit Fenster
const float T_NEON    = 0.010;  // Helligkeit der Neonfugen
const float T_LICHT   = 1.4;    // Kamera-Scheinwerfer
// ----------------------------------------------------------------------------

const float TAU = 6.28318530;

float hash21(vec2 p) { return fract(sin(dot(p, vec2(127.1, 311.7))) * 43758.5453); }

vec3 pal(float t) { return 0.5 + 0.5 * cos(TAU * (t + vec3(0.0, 0.33, 0.67))); }

float tunnelMap(vec3 p)
{
    float w = atan(p.y, p.x);
    float relief = T_TIEFE * (0.5 - 0.5 * cos(w * T_ROEHREN));
    return T_RADIUS - relief - length(p.xy);
}

float tunnelMarch(vec3 ro, vec3 rd)
{
    float t = 0.02;
    for (int i = 0; i < 90; i++) {
        float d = tunnelMap(ro + rd * t);
        if (d < 0.0015 + 0.001 * t) break;
        t += d * 0.7;
        if (t > 40.0) break;
    }
    return min(t, 40.0);
}

vec3 tunnelNormale(vec3 p)
{
    vec2 e = vec2(0.004, 0.0);
    return normalize(vec3(tunnelMap(p + e.xyy) - tunnelMap(p - e.xyy),
                          tunnelMap(p + e.yxy) - tunnelMap(p - e.yxy),
                          tunnelMap(p + e.yyx) - tunnelMap(p - e.yyx)));
}

float tunnelFenster(float w, float z)   // 0 = Wand, 1 = Fensteroeffnung
{
    vec2 zelle = vec2(fract(w / TAU + 0.5) * T_SPALTEN, z / T_ABSTAND);
    vec2 id = floor(zelle);
    if (hash21(id + 3.1) > T_DICHTE) return 0.0;
    vec2 c = fract(zelle) - 0.5;
    c.x *= TAU * T_RADIUS / T_SPALTEN;
    c.y *= T_ABSTAND;
    float d = max(abs(c.x) - 0.30, abs(c.y) - 0.85);
    return smoothstep(0.05, -0.05, d);
}

vec3 tunnelNeon(float w, float z)
{
    float fu  = w * T_ROEHREN / TAU;
    float gid = mod(floor(fu + 0.5), T_ROEHREN);
    float ad  = abs(fract(fu + 0.5) - 0.5) * TAU * T_RADIUS / T_ROEHREN;
    return pal(hash21(vec2(gid, 2.6)) * 0.4 + 0.55) * T_NEON / (0.0015 + ad * ad * 60.0);
}

vec3 tunnelShade(vec3 p, vec3 n, vec3 ro, float w)
{
    vec3 basis = vec3(0.06, 0.07, 0.10);
    vec3 zk = ro - p;
    float dk = max(length(zk), 1e-3);
    vec3 col = basis * (T_LICHT * max(dot(n, zk / dk), 0.0) / (1.0 + dk * dk * 0.12) + 0.02);
    col += tunnelNeon(w, p.z);
    return col;
}

void mainImage(out vec4 fragColor, in vec2 fragCoord)
{
    vec2 uv = (fragCoord - 0.5 * iResolution.xy) / iResolution.y;

    vec3 ro = vec3(0.0, 0.0, iTime * 1.5);      // Vortrieb pur, gerade Achse
    vec3 rd = normalize(vec3(uv, 1.4));

    float t = tunnelMarch(ro, rd);
    vec3 p = ro + rd * t;
    float w = atan(p.y, p.x);

    vec3 col = tunnelShade(p, tunnelNormale(p), ro, w);
    col = mix(col, vec3(0.010, 0.014, 0.030), 1.0 - exp(-0.0016 * t * t));

    // PLATZHALTER: hier oeffnet in Schritt 4 das Portal in die Debris-Welt
    float F = tunnelFenster(w, p.z) * exp(-0.001 * t * t);
    col = mix(col, vec3(0.02, 0.03, 0.07), F);

    col = 1.0 - exp(-col * 1.6);                // Notbelichtung (s. Regel 3)
    fragColor = vec4(pow(col, vec3(1.0 / 2.2)), 1.0);
}
