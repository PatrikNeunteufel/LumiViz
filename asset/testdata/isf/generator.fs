/*{
    "CATEGORIES": [
        "Generator"
    ],
    "CREDIT": "LumiViz-Testfixture",
    "DESCRIPTION": "Erzeugt ein Bild aus dem Nichts - hat KEIN inputImage und muss darum abgelehnt werden.",
    "INPUTS": [
        {
            "DEFAULT": [0.0, 0.0, 1.0, 1.0],
            "NAME": "farbe1",
            "TYPE": "color"
        }
    ],
    "ISFVSN": "2"
}
*/

void main() {
	vec2 uvKoord = isf_FragNormCoord.xy;
	gl_FragColor = mix(farbe1, vec4(1.0), uvKoord.y);
}
