shader_body
{
    // Half sharp, half from the first blur level — soft trails.
    ret = lerp(tex2D(sampler_main, uv).xyz, GetBlur1(uv), 0.5);
    ret *= 0.98;
}
