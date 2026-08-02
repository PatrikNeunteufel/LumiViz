// 41 Leben-Neon, Image — Patrik Neunteufel (LumiViz, 2026). Eigenwerk.
// "Image": iChannel0 = Buffer A. Junge Zellen gleißen weiß, alte kühlen
// durch die Neon-Palette ab; dazu 4-Tap-Glow.

// ---- STELLSCHRAUBEN --------------------------------------------------------
const float GLOW_RADIUS = 0.004;
const float GLOW = 0.6;
const float ALTER_FARBE = 6.0;  // wie schnell das Alter die Farbe dreht
// ----------------------------------------------------------------------------

vec3 lebenFarbe(vec2 uv)
{
    vec4 z = texture(iChannel0, uv);
    if (z.r < 0.5) return vec3(0.0);
    float alter = z.g;  // 0 = frisch
    vec3 jung = vec3(1.0);
    vec3 farbe = 0.5 + 0.5 * cos(alter * ALTER_FARBE + vec3(0.0, 2.1, 4.2));
    return mix(jung, farbe * 0.8, smoothstep(0.0, 0.15, alter));
}
void mainImage(out vec4 fragColor, in vec2 fragCoord)
{
    vec2 uv = fragCoord / iResolution.xy;
    vec3 c = lebenFarbe(uv);
    c += GLOW * 0.25 * (lebenFarbe(uv + vec2(GLOW_RADIUS, 0.0)) +
                        lebenFarbe(uv - vec2(GLOW_RADIUS, 0.0)) +
                        lebenFarbe(uv + vec2(0.0, GLOW_RADIUS)) +
                        lebenFarbe(uv - vec2(0.0, GLOW_RADIUS)));
    fragColor = vec4(c + vec3(0.01, 0.01, 0.02), 1.0);
}
