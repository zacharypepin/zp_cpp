struct PerVert {
	float2 pos;
};

struct PerInst {
	float4 inst_colour;
};

struct PerDraw {
	PerVert* p_per_verts;
	PerInst* p_per_insts;
};

struct vs_output {
	nointerpolation float4 frag_colour;
};

[shader("vertex")]
vs_output vert_main(uint vert_id : SV_VertexID, uint inst_id : SV_InstanceID, uniform PerDraw* pc, out float4 out_pos : SV_Position)
{
	PerVert per_vert = pc->p_per_verts[vert_id];
	PerInst per_inst = pc->p_per_insts[inst_id];

	vs_output outp;
	outp.frag_colour = per_inst.inst_colour;

	float2 final_pos = per_vert.pos * 2 - float2(1);

	out_pos = float4(final_pos, 0, 1);

	return outp;
}
