/*{
    "CATEGORIES": [
        "Wipe"
    ],
    "CREDIT": "LumiViz-Testfixture",
    "DESCRIPTION": "Blendet zwischen ZWEI Bildern - Uebergang, kein Filter.",
    "INPUTS": [
        {
            "NAME": "startImage",
            "TYPE": "image"
        },
        {
            "NAME": "endImage",
            "TYPE": "image"
        },
        {
            "DEFAULT": 0,
            "MAX": 1,
            "MIN": 0,
            "NAME": "progress",
            "TYPE": "float"
        }
    ],
    "ISFVSN": "2"
}
*/

void main() {
	vec4 a = IMG_THIS_PIXEL(startImage);
	vec4 b = IMG_THIS_PIXEL(endImage);
	gl_FragColor = mix(a, b, progress);
}
