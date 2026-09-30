// Generator ohne Bildquelle — laeuft auch im frischen Knoten.
void main()
{
    vec2 uv = isf_FragNormCoord;
    gl_FragColor = vec4(uv, 0.5 + 0.5 * sin(TIME), 1.0);
}
