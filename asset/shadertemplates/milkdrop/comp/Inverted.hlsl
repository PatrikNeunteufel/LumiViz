shader_body
{
    ret = 1 - tex2D(sampler_main, uv).xyz;
}
