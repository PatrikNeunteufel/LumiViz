shader_body
{
    // Helligkeit folgt dem geglaetteten Bass.
    ret = tex2D(sampler_main, uv).xyz * (0.8 + 0.4 * bass_att);
}
