// 56 Fraktal-Baum — Patrik Neunteufel (LumiViz, 2026). Eigenwerk.
// Shadertoy: iChannel0 = Music. VERZWEIGUNGS-FRAKTAL im Wind.
//
// IDEE: Ein binärer Baum über Faltung: je Tiefe wird der Raum an der
// Stammachse gespiegelt, um den Astwinkel gedreht und verkürzt — der
// Abstand zum Stamm-Segment jeder Ebene zeichnet Äste. Der Astwinkel
// schwankt mit der Zeit (Wind) und den Mitten (Böen im Takt).

// ---- STELLSCHRAUBEN --------------------------------------------------------
const int   TIEFEN      = 9;
const float AST_WINKEL  = 0.55;  // Grund-Verzweigungswinkel
const float WIND        = 0.08;  // Windschwanken
const float WIND_MITTEN = 0.12;  // Böen aus den Mitten
const float VERKUERZUNG = 0.72;  // Astlänge je Tiefe
const float AST_DICKE   = 0.012;
// ----------------------------------------------------------------------------

vec2 rot2(vec2 p, float a) { return vec2(p.x * cos(a) - p.y * sin(a),
                                         p.x * sin(a) + p.y * cos(a)); }
void mainImage(out vec4 fragColor, in vec2 fragCoord)
{
    vec2 uv = (2.0 * fragCoord - iResolution.xy) / iResolution.y;
    float mid = texture(iChannel0, vec2(0.30, 0.25)).x;
    uv.y += 0.85;  // Baum wurzelt unten
    float wind = WIND * sin(iTime * 0.8) + WIND_MITTEN * mid * sin(iTime * 5.0);

    vec3 col = vec3(0.012, 0.01, 0.03);
    vec2 p = uv;
    float s = 1.0;
    float laenge = 0.55;
    for (int i = 0; i < TIEFEN; ++i)
    {
        // Stamm-Segment dieser Ebene: Strecke (0,0)–(0,laenge)
        vec2 q = vec2(p.x, p.y - clamp(p.y, 0.0, laenge));
        float d = length(q) / s;
        float dicke = AST_DICKE * pow(0.75, float(i));
        float ast = smoothstep(dicke, dicke * 0.3, d);
        // Farbverlauf Stamm → Blattwerk; äußere Ebenen glühen grünlich
        vec3 farbe = mix(vec3(0.45, 0.25, 0.12), vec3(0.35, 0.9, 0.35),
                         float(i) / float(TIEFEN - 1));
        col += ast * farbe * (0.9 - 0.06 * float(i));

        // zum nächsten Ast: an die Spitze, spiegeln, drehen, strecken
        p.y -= laenge;
        p.x = abs(p.x);
        p = rot2(p, -(AST_WINKEL + wind * (1.0 + 0.4 * float(i))));
        p /= VERKUERZUNG;
        s /= VERKUERZUNG;
        laenge *= 0.85;
    }
    fragColor = vec4(col, 1.0);
}
