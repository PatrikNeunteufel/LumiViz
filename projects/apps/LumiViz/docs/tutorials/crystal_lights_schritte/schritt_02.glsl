// Schritt 2 - Die Kamera: Strahlen auf eine Bodenebene
// Voll-Listing aus CrystalLights-tutorial.md (SSOT dort).
#define R(a) mat2(cos(a), sin(a), -sin(a), cos(a))

void mainImage(out vec4 fragColor, in vec2 fragCoord)
{
    vec2 uv = (fragCoord - 0.5 * iResolution.xy) / iResolution.y;

    vec3 ro = vec3(0.0, 2.8, iTime * 0.8);   // Kamera: 2.8 Einheiten Hoehe, fliegt in +z
    vec3 rd = normalize(vec3(uv, 1.3));      // 1.3 = Brennweite (groesser = Tele)
    rd.yz *= R(-0.12);                        // Blick leicht nach unten kippen

    vec3 color;
    if (rd.y < 0.0) {
        // Strahl trifft die Ebene y=0 nach t Einheiten (reine Algebra, kein Marsch)
        float t = -ro.y / rd.y;
        vec2 q = (ro + rd * t).xz;

        float schach = mod(floor(q.x) + floor(q.y), 2.0);
        color = mix(vec3(0.08, 0.16, 0.20), vec3(0.25, 0.45, 0.55), schach);
        color *= exp(-t * 0.05);             // Ferne abdunkeln (Mini-Nebel)
    } else {
        color = mix(vec3(0.10, 0.12, 0.22), vec3(0.02, 0.03, 0.08),
                    clamp(rd.y * 3.0, 0.0, 1.0));
    }

    fragColor = vec4(color, 1.0);
}
