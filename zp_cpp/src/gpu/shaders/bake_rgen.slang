import rt_common;
import random;

[[vk::binding(0)]] RWTexture2D<float4> pos_img;
[[vk::binding(1)]] RWTexture2D<float4> nrm_img;
[[vk::binding(2)]] RWTexture2D<float4> out_img;
[[vk::binding(3)]] RaytracingAccelerationStructure tlas;

// ====================================================================================================================
// ====================================================================================================================
// ====================================================================================================================
// ====================================================================================================================
static const float MAX_DIST = 5.0;
static const int IMG_SIZE = 4096;
static const int NB_RAYS = 8192;
static const float RAY_OFFSET = 0.001;

// ====================================================================================================================
// ====================================================================================================================
// ====================================================================================================================
// ====================================================================================================================
float4 rt_ao(float3 start_pos, float3 nrm, inout uint rngState)
{
	float3 avg_dir = float3(0.0);
    float ao = 0.0;
	uint32_t num_unoccluded_rays = 0;

    for (int i = 0; i < NB_RAYS; i++) {
        Payload payload;

        float3 dir = normalize(nrm + RandomDirection(rngState));
        float3 adjusted_origin = start_pos + RAY_OFFSET * dir;

		RayDesc ray;
		ray.Origin = adjusted_origin;
		ray.Direction = dir;
		ray.TMin = RAY_OFFSET;
		ray.TMax = MAX_DIST;
		
		TraceRay(tlas, RAY_FLAG_NONE, 0xFF, 0, 1, 0, ray, payload);

		if (!payload.didHit) {
			ao += 1;
			avg_dir += ray.Direction;
			num_unoccluded_rays += 1;
		} 
    }

    ao /= NB_RAYS;
	
	avg_dir /= num_unoccluded_rays;
	avg_dir = normalize(avg_dir);
	avg_dir = (avg_dir * 0.5) + float3(0.5);
    
	return float4(avg_dir, ao);
}

// ====================================================================================================================
// ====================================================================================================================
// ====================================================================================================================
// ====================================================================================================================
void bake_ao(float3 pos, float3 nrm, inout uint rngState)
{
	// ============================================================================================
	// ============================================================================================
	// rt ao
	// ============================================================================================
	// ============================================================================================
	float4 bent_nrm_ao = rt_ao(pos, nrm, rngState);

	out_img.Store(DispatchRaysIndex().xy, bent_nrm_ao);
}

// ====================================================================================================================
// ====================================================================================================================
// ====================================================================================================================
// ====================================================================================================================
struct BleedData
{
	float3 pos;
	float3 nrm;
};
BleedData bleed()
{
	int2 coord = int2(DispatchRaysIndex().xy);

	// ============================================================================================
	// ============================================================================================
	// ============================================================================================
	// ============================================================================================
	{
		float4 read_pos = pos_img.Load(coord);
		float4 read_nrm = nrm_img.Load(coord);

		bool is_relevant_texel = read_pos.a != 0;
		if (is_relevant_texel) {
			BleedData data;
			data.pos = read_pos.xyz;
			data.nrm = read_nrm.xyz;
			return data;
		}
	}

	// ============================================================================================
	// ============================================================================================
	// ============================================================================================
	// ============================================================================================
	{
		const int2 neighbours_to_check[8] = int2[](
			coord + int2(0, 1),
			coord + int2(1, 0),
			coord + int2(0, -1),
			coord + int2(-1, 0),
			coord + int2(1, 1),
			coord + int2(1, -1),
			coord + int2(-1, 1),
			coord + int2(-1, -1)
		);


		for (int i = 0; i < 8; i++) {
			int2 neighbour_coord = neighbours_to_check[i];
			float4 read_neighbour_pos = pos_img.Load(neighbour_coord);
			if (read_neighbour_pos.a != 0) {
				float4 read_neighbour_nrm = nrm_img.Load(neighbour_coord);
				BleedData data;
				data.pos = read_neighbour_pos.xyz;
				data.nrm = read_neighbour_nrm.xyz;
				return data;
			}
		}
	}

	{
		const int2 extended_neighbours_to_check[16] = int2[](
			coord + int2(0, 2),
			coord + int2(2, 0),
			coord + int2(0, -2),
			coord + int2(-2, 0),
			coord + int2(2, 2),
			coord + int2(2, -2),
			coord + int2(-2, 2),
			coord + int2(-2, -2),
			coord + int2(1, 2),
			coord + int2(2, 1),
			coord + int2(2, -1),
			coord + int2(1, -2),
			coord + int2(-1, -2),
			coord + int2(-2, -1),
			coord + int2(-2, 1),
			coord + int2(-1, 2)
		);

		for (int i = 0; i < 16; i++) {
			int2 neighbour_coord = extended_neighbours_to_check[i];
			float4 read_neighbour_pos = pos_img.Load(neighbour_coord);
			if (read_neighbour_pos.a != 0) {
				float4 read_neighbour_nrm = nrm_img.Load(neighbour_coord);
				BleedData data;
				data.pos = read_neighbour_pos.xyz;
				data.nrm = read_neighbour_nrm.xyz;
				return data;
			}
		}
	}

	// ============================================================================================
	// ============================================================================================
	// ============================================================================================
	// ============================================================================================
	{
		BleedData data;
		data.pos = float3(0);
		data.nrm = float3(0);
		return data;
	}
}

// ====================================================================================================================
// ====================================================================================================================
// ====================================================================================================================
// ====================================================================================================================
[shader("raygeneration")]
void rgen_main() {
	const uint2 pixelCoord = DispatchRaysIndex().xy;
	const uint pixelIndex = pixelCoord.y * IMG_SIZE + pixelCoord.x;

	uint rngState = pixelIndex + 719393;

	BleedData bleed_data = bleed();
	if (length(bleed_data.nrm) != 0) {
		bake_ao(bleed_data.pos, bleed_data.nrm, rngState);
	} else {
	 	out_img.Store(pixelCoord, float4(0));
	}
}