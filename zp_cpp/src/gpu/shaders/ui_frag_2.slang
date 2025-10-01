public struct Params
{
	public SamplerState samp;
	public Texture2D panels[8];
	public RWTexture2D rw_panels[8];
	public Texture2D textures[1024];
};

struct vs_output {
	float2 frag_tex_coord;
	nointerpolation float4 frag_colour;
	nointerpolation int frag_is_text;
	nointerpolation int frag_tex_idx;
	nointerpolation float frag_screen_px_range;
};

// ====================================================================================================================
// ====================================================================================================================
// ====================================================================================================================
// ====================================================================================================================
float linear_to_srgb(float l)
{
	if (l <= 0.0031308) return l * 12.92;
	else return 1.055 * pow(l, 1.0/2.4) - 0.055;
}

// ====================================================================================================================
// ====================================================================================================================
// ====================================================================================================================
// ====================================================================================================================
float median(float r, float g, float b)
{
    return max(min(r, g), min(max(r, g), b));
}

// ========================================================================================================================================
// ========================================================================================================================================
// ========================================================================================================================================
// ========================================================================================================================================
[shader("fragment")]
float4 frag_main(ParameterBlock<Params> params, vs_output inp) : SV_Target
{	
	if (inp.frag_is_text == 0)
	{
		if (inp.frag_tex_idx != -1)
		{
			float4 sampled = params.textures[NonUniformResourceIndex(inp.frag_tex_idx)].SampleLevel(params.samp, inp.frag_tex_coord, 0);
			float3 srgb_col;
			srgb_col.r = linear_to_srgb(sampled.r);
			srgb_col.g = linear_to_srgb(sampled.g);
			srgb_col.b = linear_to_srgb(sampled.b);
		
			return float4(srgb_col, sampled.a);
		}
		
		return inp.frag_colour;
	}

	float4 sampled = params.textures[NonUniformResourceIndex(inp.frag_tex_idx)].SampleLevel(params.samp, inp.frag_tex_coord, 0);
	float sd = median(sampled.r, sampled.g, sampled.b);
	float screenPxDistance = inp.frag_screen_px_range * (sd - 0.5);
	float opacity = clamp(screenPxDistance + 0.5, 0.0, 1.0);
	return opacity * inp.frag_colour;
}
