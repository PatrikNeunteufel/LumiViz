// 81 Leben-Feuerwerk, Image — Patrik Neunteufel (LumiViz, 2026). Eigenwerk.
// "Image": iChannel0 = Buffer A. Zellen kaltweiß, die Todes-Funken darüber
// in Feuerwerksfarben (Position färbt).

void mainImage(out vec4 fragColor, in vec2 fragCoord)
{
    vec2 uv = fragCoord / iResolution.xy;
    vec4 z = texture(iChannel0, uv);
    vec3 col = vec3(0.008, 0.008, 0.02);
    col += z.r * vec3(0.75, 0.85, 1.0) * 0.8;  // lebende Zellen
    // Funken: Farbe aus der Bildposition (wie verschiedene Effekt-Sätze)
    vec3 funkenFarbe = 0.5 + 0.5 * cos(uv.x * 9.0 + uv.y * 5.0 + vec3(0.0, 2.1, 4.2));
    col += z.b * funkenFarbe * 1.3;
    fragColor = vec4(col, 1.0);
}
