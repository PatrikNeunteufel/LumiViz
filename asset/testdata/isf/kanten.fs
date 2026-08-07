/*{
    "CATEGORIES": [
        "Stylize"
    ],
    "CREDIT": "LumiViz-Testfixture",
    "DESCRIPTION": "Nachbar-Koordinaten kommen aus dem .vs-Begleiter - die Bauart der 38 Vidvox-Dateien mit Vertex-Shader.",
    "INPUTS": [
        {
            "NAME": "inputImage",
            "TYPE": "image"
        },
        {
            "DEFAULT": 1.0,
            "MAX": 4.0,
            "MIN": 0.0,
            "NAME": "staerke",
            "TYPE": "float"
        }
    ],
    "ISFVSN": "2"
}
*/

#if __VERSION__ <= 120
varying vec2 left_coord;
varying vec2 right_coord;
#else
in vec2 left_coord;
in vec2 right_coord;
#endif

float grau(vec4 n)
{
	return (n.r + n.g + n.b) / 3.0;
}

void main()
{
	vec4 mitte = IMG_THIS_PIXEL(inputImage);
	vec4 links = IMG_NORM_PIXEL(inputImage, left_coord);
	vec4 rechts = IMG_NORM_PIXEL(inputImage, right_coord);
	float kante = abs(grau(links) - grau(rechts)) * staerke;
	gl_FragColor = vec4(vec3(kante), mitte.a);
}
