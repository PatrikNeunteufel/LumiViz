shader_body
{
    // Halb scharf, halb aus der ersten Blur-Stufe — weiche Nachzieher.
    ret = lerp(tex2D(sampler_main, uv).xyz, GetBlur1(uv), 0.5);
    ret *= 0.98;
}
