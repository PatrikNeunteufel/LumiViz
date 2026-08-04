// SDF: Abstand vom Punkt p zur Oberflaeche einer Kugel mit Radius 1
float map(vec3 p)
{
    // return length(p) - 1.0;
    return length(p) - 1.3;
}

void mainImage(out vec4 fragColor, in vec2 fragCoord)
{
    vec2 uv = (fragCoord - 0.5 * iResolution.xy) / iResolution.y;
    vec3 ro, rd, color;
    // Kamera: Position (ray origin) und Blickrichtung (ray direction)
    ro = vec3(0.0, 0.0, -3.0);          // 3 Einheiten vor der Szene
    ro = vec3(0.0, 0.0, -6.0);          // 6 Einheiten vor der Szene
    rd = normalize(vec3(uv, 1.0));      // durch "unseren" Pixel nach vorn
    rd = normalize(vec3(uv, 2.0));

    float dist = 0.0;                        // bisher zurueckgelegte Strecke
    color = vec3(0.0);                  // Hintergrund: schwarz

    for (int i = 0; i < 80; i++)
    {
        vec3 p = ro + rd * dist;             // aktuelle Position auf dem Strahl
        float d = map(p);                    // Abstand zur naechsten Oberflaeche

        if (d < 0.001) {                     // nah genug -> Treffer!
            color = vec3(1.0);
            color = vec3(float(i) / 80.0);
            break;
        }
        if (dist > 15.0) break;              // zu weit -> ins Leere gelaufen

        dist += d;                           // sicheren Schritt vorwaerts gehen
    }

    fragColor = vec4(color, 1.0);
}