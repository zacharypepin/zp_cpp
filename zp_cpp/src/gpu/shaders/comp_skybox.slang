public struct Params
{
	public SamplerState samp;
	public Texture2D panels[8];
	public RWTexture2D rw_panels[8];
	public Texture2D textures[1024];
};
	
struct PerDispatch
{
	float4x4 model;
    float near_plane_width;
	float near_plane_height;
	float focus_dist;
	uint panel_idx;
	uint target_width;
	uint target_height;
	float _pad[2];
};

float3 irradiance_map(float3 view_dir)
{
	float3 sh0 = float3( 0.497159332036972,  0.568179786205292,  0.651987850666046); // L00, irradiance, pre-scaled base
	float3 sh1 = float3( 0.680305004119873,  0.692692518234253,  0.914044857025146); // L1-1, irradiance, pre-scaled base
	float3 sh2 = float3( 0.026012787595391,  0.034283291548491,  0.064989469945431); // L10, irradiance, pre-scaled base
	float3 sh3 = float3( 0.046574305742979,  0.040207717567682,  0.033510252833366); // L11, irradiance, pre-scaled base
	float3 sh4 = float3( 0.079555228352547,  0.080893978476524,  0.072609834372997); // L2-2, irradiance, pre-scaled base
	float3 sh5 = float3( 0.036155164241791,  0.038112685084343,  0.048442911356688); // L2-1, irradiance, pre-scaled base
	float3 sh6 = float3(-0.094137996435165, -0.093526534736156, -0.091276690363884); // L20, irradiance, pre-scaled base
	float3 sh7 = float3( 0.010022201575339,  0.014026166871190,  0.028116937726736); // L21, irradiance, pre-scaled base
	float3 sh8 = float3(-0.286742448806763, -0.289886355400085, -0.298329889774323); // L22, irradiance, pre-scaled base

	float3[9] sphericalHarmonics = float3[9](sh0, sh1, sh2, sh3, sh4, sh5, sh6, sh7, sh8);

	float3 irradiance =  
		sphericalHarmonics[0]
		+ sphericalHarmonics[1] * (view_dir.z)
		+ sphericalHarmonics[2] * (view_dir.y)
		+ sphericalHarmonics[3] * (view_dir.x)
		+ sphericalHarmonics[4] * (view_dir.z * view_dir.x)
		+ sphericalHarmonics[5] * (view_dir.z * view_dir.y)
		+ sphericalHarmonics[6] * (3.0 * view_dir.y * view_dir.y - 1.0)
		+ sphericalHarmonics[7] * (view_dir.y * view_dir.x)
		+ sphericalHarmonics[8] * (view_dir.x * view_dir.x - view_dir.z * view_dir.z);

	return irradiance;
}

[numthreads(16, 16, 1)]
[shader("compute")]
void comp_main(ParameterBlock<Params> params, uint3 dispatch_thread_id : SV_DispatchThreadID, uniform PerDispatch* p_per_dispatch)
{
    const uint2 coord = dispatch_thread_id.xy;	
	const float2 pixel_center = (float2)coord + float2(0.5);

	float2 screen_uv = pixel_center / float2(p_per_dispatch->target_width, p_per_dispatch->target_height);
	screen_uv.y = 1 - screen_uv.y;

	float3 focus_point_local = float3(
		(screen_uv.x - 0.5) * p_per_dispatch->near_plane_width,
		p_per_dispatch->focus_dist,
		(screen_uv.y - 0.5) * p_per_dispatch->near_plane_height,
	);
	float3 focus_point = mul(p_per_dispatch->model, float4(focus_point_local, 1)).xyz;
	float3 ray_orig = float3(p_per_dispatch->model[0][3], p_per_dispatch->model[1][3], p_per_dispatch->model[2][3]);
	float3 ray_dir = normalize(focus_point - ray_orig);

   	float4 colour = float4(irradiance_map(ray_dir), 1);

	params.rw_panels[p_per_dispatch->panel_idx].Store(coord, colour);
}
