public struct Params
{
	public SamplerState samp;
	public Texture2D panels[8];
	public RWTexture2D rw_panels[8];
	public Texture2D textures[1024];
};

struct vs_output
{
	float2 frag_tex_coord;
	nointerpolation uint frag_panel_idx;
};

// ========================================================================================================================================
// ========================================================================================================================================
// note
// premultiplies alpha
// ========================================================================================================================================
// ========================================================================================================================================
[shader("fragment")]
float4 frag_main(ParameterBlock<Params> params, vs_output inp) : SV_Target
{
	float4 c = params.panels[NonUniformResourceIndex(inp.frag_panel_idx)].SampleLevel(params.samp, inp.frag_tex_coord, 0);
    c.rgb *= c.a;
    return c;
}