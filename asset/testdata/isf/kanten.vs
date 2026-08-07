#if __VERSION__ <= 120
varying vec2 left_coord;
varying vec2 right_coord;
#else
out vec2 left_coord;
out vec2 right_coord;
#endif

void main()
{
	isf_vertShaderInit();
	vec2 texc = vec2(isf_FragNormCoord[0], isf_FragNormCoord[1]);
	vec2 d = 1.0 / RENDERSIZE;

	left_coord = clamp(vec2(texc.xy + vec2(-d.x, 0.0)), 0.0, 1.0);
	right_coord = clamp(vec2(texc.xy + vec2(d.x, 0.0)), 0.0, 1.0);
}
