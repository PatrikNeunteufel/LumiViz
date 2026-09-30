shader_body
{
    // The preset's four corner colours (hue_shader) over the image.
    ret = tex2D(sampler_main, uv).xyz * hue_shader;
}
