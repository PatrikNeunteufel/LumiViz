shader_body
{
    // Das Bild wandert schraeg und blendet dabei aus.
    ret = tex2D(sampler_main, uv + float2(0.002, 0.002)).xyz;
    ret *= 0.99;
}
