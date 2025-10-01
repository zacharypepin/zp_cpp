struct PerInst
{
	float2 offset;
	float2 size;
};

struct PerDraw
{
	PerInst* p_per_insts;
};

struct vs_output
{
	float2 frag_tex_coord;
	uint frag_panel_idx;
};

static const float2 kPos[6] = {
    float2(0,0),
    float2(1,0),
    float2(1,1),
    float2(0,0),
    float2(0,1),
    float2(1,1),
};

static const float2 kUV[6] = {
    float2(0.0, 0.0),
    float2(1.0, 0.0),
    float2(1.0, 1.0),
    float2(0.0, 0.0),
    float2(0.0, 1.0),
    float2(1.0, 1.0),
};

[shader("vertex")]
vs_output vert_main(uint vert_id : SV_VertexID, uint inst_id : SV_InstanceID, uniform PerDraw* pc, out float4 out_pos : SV_Position)
{
    PerInst per_inst = pc->p_per_insts[inst_id];

    float2 pos = ((kPos[vert_id] * per_inst.size) + per_inst.offset) * 2 - 1;

    out_pos = float4(pos, 0.0, 1.0);

    vs_output outp;
    outp.frag_tex_coord = kUV[vert_id];
    outp.frag_panel_idx = inst_id;
    return outp;
}
