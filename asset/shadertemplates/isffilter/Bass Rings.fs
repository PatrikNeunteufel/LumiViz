// Generator: rings whose brightness follows the bass.
void main()
{
    vec2 p = isf_FragNormCoord - 0.5;
    p.x *= RENDERSIZE.x / RENDERSIZE.y;
    float v = 0.5 + 0.5 * sin(length(p) * 40.0 - TIME * 4.0);
    gl_FragColor = vec4(vec3(v) * (0.4 + bass), 1.0);
}
