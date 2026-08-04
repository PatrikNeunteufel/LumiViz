#define R(a) mat2(cos(a), sin(a), -sin(a), cos(a))

const float RADIUS = 1.0;    // Grundradius des Tunnels

// Innen-Abstand zur Tunnelwand: positiv im Inneren, 0 auf der Wand
float mapTunnel(vec3 p)
{
    return RADIUS - length(p.xy);
}

// Marsch: laeuft am Strahl entlang, bis er die Wand beruehrt
float march(vec3 ro, vec3 rd)
{
    float t = 0.02;
    for (int i = 0; i < 120; i++) {
        float d = mapTunnel(ro + rd * t);
        if (d < 0.0015 + 0.001 * t) break;   // aufgesetzt -> Treffer
        t += d;                              // exakter Zylinder: voller Schritt
        if (t > 60.0) break;                 // tief genug -> aufgeben
    }
    return min(t, 60.0);
}

void mainImage(out vec4 fragColor, in vec2 fragCoord)
{
    vec2 uv = (fragCoord - 0.5 * iResolution.xy) / iResolution.y;

    vec3 ro = vec3(0.0, 0.0, iTime * 2.0);   // Kamera auf der Achse, fliegt +z
    vec3 rd = normalize(vec3(uv, 1.4));      // 1.4 = Brennweite (groesser = Tele)

    float t = march(ro, rd);
    vec3 p = ro + rd * t;

    // Tiefe als Helligkeit + z-Streifen, damit die Fahrt sichtbar ist
    float streifen = 0.6 + 0.4 * cos(p.z * 3.0);
    vec3 color = vec3(streifen) * exp(-0.10 * t);

    fragColor = vec4(color, 1.0);
}
