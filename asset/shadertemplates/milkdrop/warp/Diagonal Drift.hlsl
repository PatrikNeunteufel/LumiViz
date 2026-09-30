shader_body
{
    // The image moves diagonally and fades while doing so.
    ret = tex2D(sampler_main, uv + float2(0.002, 0.002)).xyz;
    ret *= 0.99;
}
