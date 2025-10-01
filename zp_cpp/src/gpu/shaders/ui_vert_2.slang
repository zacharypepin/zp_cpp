struct PerVert {
	float2 pos;
};

struct PerInst {
	float4 inst_colour;
	float2 inst_pos;
	float2 inst_scale;
	float2 inst_tex_coord_start;
	float2 inst_tex_coord_size;
	int inst_is_text;
	int inst_tex_idx;
	float inst_rot;
	float inst_screen_px_range;
};

struct PerDraw {
	PerVert* p_per_verts;
	PerInst* p_per_insts;
};

struct vs_output {
	float2 frag_tex_coord;
	nointerpolation float4 frag_colour;
	nointerpolation int frag_is_text;
	nointerpolation int frag_tex_idx;
	nointerpolation float frag_screen_px_range;
};

float2 rotatePoint(float2 point, float2 pivot, float angle) {
	float2 translatedPoint = point - pivot;
	float cosAngle = cos(angle);
	float sinAngle = sin(angle);
	float2x2 rotationMatrix = float2x2(
		cosAngle, -sinAngle,
		sinAngle, cosAngle
	);
	float2 rotatedPoint = mul(rotationMatrix, translatedPoint);
	return rotatedPoint + pivot;
}

[shader("vertex")]
vs_output vert_main(uint vert_id : SV_VertexID, uint inst_id : SV_InstanceID, uniform PerDraw* pc, out float4 out_pos : SV_Position)
{
	PerVert per_vert = pc->p_per_verts[vert_id];
	PerInst inp = pc->p_per_insts[inst_id];

	float2 interm = per_vert.pos * inp.inst_tex_coord_size;
	interm.y = inp.inst_tex_coord_size.y - interm.y;

	vs_output outp;
	outp.frag_tex_coord = inp.inst_tex_coord_start + interm;
	outp.frag_colour = inp.inst_colour;
	outp.frag_is_text = inp.inst_is_text;
	outp.frag_tex_idx = inp.inst_tex_idx;
	outp.frag_screen_px_range = inp.inst_screen_px_range;

	// todo
	// decide if want screen data passed in or no
	float2 real_scale = inp.inst_scale * float2(2560, 1440);
	float2 real_inst_pos = inp.inst_pos * float2(2560, 1440);
	float2 real_vert_pos = (per_vert.pos * real_scale) + real_inst_pos;
	float2 real_center = (float2(0.5) * real_scale) + real_inst_pos;

	float2 final_real = rotatePoint(real_vert_pos, real_center, inp.inst_rot);

	float2 final_pos = final_real / float2(2560, 1440);
	final_pos = final_pos * 2 - float2(1);

	out_pos = float4(final_pos, 0, 1);

	return outp;
}
