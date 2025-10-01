struct vs_input
{
	[[vk::location(0)]] float2 vert_tex_coord;
	[[vk::location(1)]] float3 vert_pos;
	[[vk::location(2)]] float3 vert_nrm;
};

struct vs_output
{
	[[vk::location(0)]] float3 frag_pos;
	[[vk::location(1)]] float3 frag_nrm;
};

[shader("vertex")]
vs_output vert_main(vs_input inp, out float4 out_pos : SV_Position)
{
	float2 final_pos = inp.vert_tex_coord * 2 - 1;
	out_pos = float4(final_pos, 0, 1);

	vs_output outp;
	outp.frag_pos = inp.vert_pos;
	outp.frag_nrm = inp.vert_nrm;
	return outp;
}
