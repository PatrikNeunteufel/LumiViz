// Schritt 3 - Hoehenfeld-Raymarching: die Ebene wird Landschaft
// Voll-Listing aus CrystalLights-tutorial.md (SSOT dort).
#define R(a) mat2(cos(a), sin(a), -sin(a), cos(a))

// Hoehe der Landschaft am Ort (x,z) - vorerst simple Sinus-Huegel
float terrain(vec2 p)
{
    return 0.55 * sin(p.x * 0.8) * sin(p.y * 0.6);
}

// Terrain-Marsch: laeuft am Strahl entlang, bis er unter die Landschaft taucht
float marchTerrain(vec3 ro, vec3 rd)
{
    float t = 0.0;
    for (int i = 0; i < 150; i++) {
        vec3 p = ro + rd * t;
        float d = p.y - terrain(p.xz);       // Hoehe UEBER dem Terrain
        if (d < 0.001 + 0.0015 * t) return t; // aufgesetzt -> Treffer
        if (t > 45.0) break;                  // Horizont -> aufgeben
        t += d * 0.4;                         // vorsichtiger Schritt
    }
    return -1.0;
}

void mainImage(out vec4 fragColor, in vec2 fragCoord)
{
    vec2 uv = (fragCoord - 0.5 * iResolution.xy) / iResolution.y;

    vec3 ro = vec3(0.0, 2.8, iTime * 0.8);
    vec3 rd = normalize(vec3(uv, 1.3));
    rd.yz *= R(-0.12);

    float t = marchTerrain(ro, rd);

    vec3 color;
    if (t > 0.0) {
        color = vec3(clamp(1.0 - t * 0.035, 0.0, 1.0)); // Tiefe als Helligkeit
    } else {
        color = mix(vec3(0.10, 0.12, 0.22), vec3(0.02, 0.03, 0.08),
                    clamp(rd.y * 3.0, 0.0, 1.0));
    }

    fragColor = vec4(color, 1.0);
}
