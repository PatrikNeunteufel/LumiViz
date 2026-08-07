/*{
    "CATEGORIES": [
        "Test"
    ],
    "CREDIT": "LumiViz-Testfixture",
    "DESCRIPTION": "Deckt jeden INPUT-Typ der ISF-Spec genau einmal ab.",
    "INPUTS": [
        {
            "NAME": "inputImage",
            "TYPE": "image"
        },
        {
            "NAME": "zweitesBild",
            "TYPE": "image"
        },
        {
            "DEFAULT": 0.5,
            "MAX": 1.0,
            "MIN": 0.0,
            "NAME": "zahl",
            "TYPE": "float"
        },
        {
            "DEFAULT": true,
            "NAME": "schalter",
            "TYPE": "bool"
        },
        {
            "DEFAULT": 2,
            "LABELS": ["Grob", "Mittel", "Fein"],
            "NAME": "modus",
            "TYPE": "long",
            "VALUES": [0, 1, 2]
        },
        {
            "DEFAULT": [0.25, 0.75],
            "NAME": "punkt",
            "TYPE": "point2D"
        },
        {
            "DEFAULT": [1.0, 0.5, 0.0, 1.0],
            "NAME": "tonung",
            "TYPE": "color"
        },
        {
            "NAME": "ausloeser",
            "TYPE": "event"
        },
        {
            "NAME": "tonspur",
            "TYPE": "audio"
        }
    ],
    "ISFVSN": "2"
}
*/

void main() {
	vec4 quelle = IMG_THIS_PIXEL(inputImage);
	vec4 fremd = IMG_THIS_PIXEL(zweitesBild);
	vec2 versatz = punkt * zahl;
	vec4 nachbar = IMG_NORM_PIXEL(inputImage, isf_FragNormCoord.xy + versatz);
	vec4 misch = mix(quelle, nachbar, zahl);
	if (schalter)
		misch *= tonung;
	if (modus > 1)
		misch.rgb = floor(misch.rgb * 8.0) / 8.0;
	gl_FragColor = misch + fremd * 0.0;
}
