// Luminance after Rec. 709.
vec4 farbe(vec2 uv, vec4 src)
{
    float l = dot(src.rgb, vec3(0.2126, 0.7152, 0.0722));
    return vec4(vec3(l), src.a);
}
