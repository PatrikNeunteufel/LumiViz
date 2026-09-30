shader_body
{
    // Die vier Eckfarben des Presets (hue_shader) ueber dem Bild.
    ret = tex2D(sampler_main, uv).xyz * hue_shader;
}
