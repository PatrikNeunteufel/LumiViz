void mainImage(out vec4 fragColor, in vec2 fragCoord)
{
    // Pixelposition in den Bereich 0..1 bringen
    vec2 uv = fragCoord / iResolution.xy;

    // uv.x als Rot, uv.y als Gruen ausgeben
    // fragColor = vec4(1.0-(uv.x/3.0),1.0-(uv.y/3.0), 1.0-((uv.x)/6.0) + ((uv.y)/6.0), 1.0);
    fragColor = vec4(uv.x, uv.x, uv.x, 1.0);
    fragColor = vec4(fract(uv.y * 10.0),fract(uv.y * 10.0),fract(uv.y * 10.0), 1.0);
    fragColor = vec4(fract(uv.x * 3.0), fract(uv.y * 3.0), fract((1.0-uv.y)*uv.x * 9.0), 1.0);
    fragColor = vec4(uv.x, uv.y, 0.0, 1.0);
}