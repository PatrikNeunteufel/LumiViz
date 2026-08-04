float hash21(vec2 p) { return fract(sin(dot(p, vec2(127.1, 311.7))) * 43758.5453); }

// Wuerfelflaechen-Projektion: Blickrichtung -> 2D-Koordinate fuer das Sterngitter
vec2 richtungsUv(vec3 rd)
{
    vec3 a = abs(rd);
    if (a.z >= a.x && a.z >= a.y) return rd.xy / a.z;
    if (a.x >= a.y)               return rd.zy / a.x;
    return rd.xz / a.y;
}

// Drei Sternschichten: je feiner das Gitter, desto schwaecher die Sterne
vec3 sterne(vec3 rd)
{
    vec3 acc = vec3(0.0);
    for (int s = 0; s < 3; s++) {
        float fs = float(s);
        vec2 su = richtungsUv(rd) * (24.0 + 30.0 * fs) + 13.7 * fs;
        float h = hash21(floor(su));
        float stern = smoothstep(0.988 + 0.004 * fs, 1.0, h);
        acc += stern * (0.30 + 0.70 * fract(h * 41.7)) * (1.0 - 0.28 * fs);
    }
    return acc * vec3(0.80, 0.87, 1.00);
}

void mainImage(out vec4 fragColor, in vec2 fragCoord)
{
    // Ursprung in die Bildmitte, Teilen durch die HOEHE (unverzerrt)
    vec2 uv = (fragCoord - 0.5 * iResolution.xy) / iResolution.y;

    vec3 rd = normalize(vec3(uv, 1.4));   // 1.4 = Brennweite

    // Weltraum: fast schwarz, minimal blaeulich ...
    vec3 color = vec3(0.008, 0.010, 0.018);
    // ... unten ein warmer Vorbote des Planeten (kommt in Schritt 7 wirklich)
    color += vec3(0.05, 0.02, 0.008) * clamp(-uv.y * 1.5, 0.0, 1.0);

    color += sterne(rd);

    fragColor = vec4(color, 1.0);
}
