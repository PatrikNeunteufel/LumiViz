shader_body
{
    // Bild bleibt stehen und verblasst langsam (Decay).
    ret = tex2D(sampler_main, uv).xyz;
    ret -= 0.004;
}
