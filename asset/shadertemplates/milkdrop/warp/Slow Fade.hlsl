shader_body
{
    // The image stands still and fades slowly (decay).
    ret = tex2D(sampler_main, uv).xyz;
    ret -= 0.004;
}
