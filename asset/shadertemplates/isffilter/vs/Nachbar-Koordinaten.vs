// Rechnet die Koordinaten der Nachbarpixel einmal je Vertex vor.
// Der Fragment-Shader liest sie als gleichnamige `in vec2 ...;`
// (GLSL 330: `out` hier, `in` dort; `varying` gibt es nicht mehr).
out vec2 links;
out vec2 rechts;
out vec2 oben;
out vec2 unten;

void main()
{
    isf_vertShaderInit();
    vec2 px = 1.0 / RENDERSIZE;
    links = isf_FragNormCoord - vec2(px.x, 0.0);
    rechts = isf_FragNormCoord + vec2(px.x, 0.0);
    oben = isf_FragNormCoord + vec2(0.0, px.y);
    unten = isf_FragNormCoord - vec2(0.0, px.y);
}
