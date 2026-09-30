// Precomputes the neighbour pixel coordinates once per vertex.
// The fragment shader reads them as `in vec2 ...;` of the same name
// (GLSL 330: `out` here, `in` there; `varying` no longer exists).
out vec2 left;
out vec2 right;
out vec2 above;
out vec2 below;

void main()
{
    isf_vertShaderInit();
    vec2 px = 1.0 / RENDERSIZE;
    left = isf_FragNormCoord - vec2(px.x, 0.0);
    right = isf_FragNormCoord + vec2(px.x, 0.0);
    above = isf_FragNormCoord + vec2(0.0, px.y);
    below = isf_FragNormCoord - vec2(0.0, px.y);
}
