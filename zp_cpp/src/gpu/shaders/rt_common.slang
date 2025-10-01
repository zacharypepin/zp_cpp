module rt_common;

public interface IMatData {}

public struct MatData1 : IMatData
{
	public uint32_t base_col_tex_idx;
}

public struct MatData2 : IMatData
{
	public float4 colour;
	public float4 colour_2;
}

public struct MatDataSphere : IMatData
{
	public float3 colour;
	public int type;
}

public struct PerMaterial<T> where T : IMatData
{
	public T* p_mat_data;
	float _pad[2];
}

public struct UboData<T> where T : IMatData
{
	public float4x4 model;
	public PerMaterial<T>* p_per_mat_buff;
	public float near_plane_width;
	public float near_plane_height;
	public float focus_dist;
	float[3] _pad;
}

public struct HitInfo {
	public float3 hitPoint;
	public float3 N;
	public float2 tex_coord;
	public float dist;
	public float3 base_col;
}

public struct Payload {
	public bool didHit;

	public HitInfo hitInfo;
}

public struct PerInstanceData
{
	public uint geom_start_idx;
	public uint mat_start_idx;
}

public struct PerGeometryData
{
	public uint prim_start_idx;
}

public struct PerPrimitiveData
{
	public float4x4 tbn_a;
	public float4x4 tbn_b;
	public float4x4 tbn_c;
	public float2 uv0_a;
	public float2 uv0_b;
	public float2 uv0_c;
	public float2 uv1_a;
	public float2 uv1_b;
	public float2 uv1_c;
	public float2 uv2_a;
	public float2 uv2_b;
	public float2 uv2_c;
	public float2 uv3_a;
	public float2 uv3_b;
	public float2 uv3_c;
	public float2 uv4_a;
	public float2 uv4_b;
	public float2 uv4_c;
}