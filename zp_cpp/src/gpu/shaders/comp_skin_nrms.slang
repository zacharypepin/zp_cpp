struct DispatchSkin {
    uint start_rig_idx;
    uint start_per_tris_idx;
    uint start_skinned_per_tris_idx;
    uint tri_count;
    uint start_joint_mat_idx;
    uint start_idx_idx;
    float _pad1;
    float _pad2;
};

struct PerTri
{
	float4x4 tbn_a;
	float4x4 tbn_b;
	float4x4 tbn_c;
	float2 uv0_a;
	float2 uv0_b;
	float2 uv0_c;
	float2 uv1_a;
	float2 uv1_b;
	float2 uv1_c;
	float2 uv2_a;
	float2 uv2_b;
	float2 uv2_c;
	float2 uv3_a;
	float2 uv3_b;
	float2 uv3_c;
	float2 uv4_a;
	float2 uv4_b;
	float2 uv4_c;
};

struct JointMat {
	float4x4 pos_mat;
	float3x3 nrm_mat;
};

struct Rig {
    float4 joint_idcs;
    float4 joint_weights;
};

[[vk::binding(0)]] StructuredBuffer<DispatchSkin> to_skins;
[[vk::binding(1)]] StructuredBuffer<uint, ScalarDataLayout> idcs_buff;
[[vk::binding(2)]] StructuredBuffer<PerTri, ScalarDataLayout> per_tri_datas;
[[vk::binding(3)]] StructuredBuffer<JointMat, ScalarDataLayout> joint_mats;
[[vk::binding(4)]] StructuredBuffer<Rig> rigs;
[[vk::binding(5)]] RWStructuredBuffer<PerTri, ScalarDataLayout> skinned_per_tri_datas;

[numthreads(128, 1, 1)]
[shader("compute")]
void comp_main(uint3 dispatch_thread_id : SV_DispatchThreadID, uniform uint dispatch_idx)
{
    uint tri_idx = dispatch_thread_id.x;

    DispatchSkin to_skin = to_skins[dispatch_idx];

    if (tri_idx >= to_skin.tri_count)
    {
        return;
    }

    uint scratch_tri_idx = to_skin.start_per_tris_idx + tri_idx;
    uint skinned_tri_idx = to_skin.start_skinned_per_tris_idx + tri_idx;

    PerTri per_tri_data = per_tri_datas[scratch_tri_idx];

	float3 nrm_a = mul(per_tri_data.tbn_a, float4(0,0,1,1)).xyz;
	float3 nrm_b = mul(per_tri_data.tbn_b, float4(0,0,1,1)).xyz;
	float3 nrm_c = mul(per_tri_data.tbn_c, float4(0,0,1,1)).xyz;

	// vert a
    {
		uint idx_idx = to_skin.start_idx_idx + (tri_idx * 3 + 0);
		uint rig_idx = idcs_buff[idx_idx];
		Rig rig = rigs[to_skin.start_rig_idx + rig_idx];

		// if (length(rig.joint_weights) == 1)
		{
			float3 nrm = float3(0, 0, 0);
			for (int j = 0; j < 4; j++)
			{
				int joint_idx = int(rig.joint_idcs[j]);
				float weight = rig.joint_weights[j];
				float3x3 joint_nrm_mat = joint_mats[to_skin.start_joint_mat_idx + joint_idx].nrm_mat;
				nrm += weight * normalize(mul(joint_nrm_mat, nrm_a));
			}
			nrm_a = normalize(nrm);
		}
	}

	// vert b
    {
		uint idx_idx = to_skin.start_idx_idx + (tri_idx * 3 + 1);
		uint rig_idx = idcs_buff[idx_idx];
		Rig rig = rigs[to_skin.start_rig_idx + rig_idx];

		// if (length(rig.joint_weights) == 1)
		{
			float3 nrm = float3(0, 0, 0);
			for (int j = 0; j < 4; j++)
			{
				int joint_idx = int(rig.joint_idcs[j]);
				float weight = rig.joint_weights[j];
				float3x3 joint_nrm_mat = joint_mats[to_skin.start_joint_mat_idx + joint_idx].nrm_mat;
				nrm += weight * normalize(mul(joint_nrm_mat, nrm_b));
			}
			nrm_b = normalize(nrm);
		}
	}

	// vert c
    {
		uint idx_idx = to_skin.start_idx_idx + (tri_idx * 3 + 2);
		uint rig_idx = idcs_buff[idx_idx];
		Rig rig = rigs[to_skin.start_rig_idx + rig_idx];

		// if (length(rig.joint_weights) == 1)
		{
			float3 nrm = float3(0, 0, 0);
			for (int j = 0; j < 4; j++)
			{
				int joint_idx = int(rig.joint_idcs[j]);
				float weight = rig.joint_weights[j];
				float3x3 joint_nrm_mat = joint_mats[to_skin.start_joint_mat_idx + joint_idx].nrm_mat;
				nrm += weight * normalize(mul(joint_nrm_mat, nrm_c));
			}
			nrm_c = normalize(nrm);
		}
	}

	per_tri_data.tbn_a[0][2] = nrm_a.x;
	per_tri_data.tbn_a[1][2] = nrm_a.y;
	per_tri_data.tbn_a[2][2] = nrm_a.z;

	per_tri_data.tbn_b[0][2] = nrm_b.x;
	per_tri_data.tbn_b[1][2] = nrm_b.y;
	per_tri_data.tbn_b[2][2] = nrm_b.z;

	per_tri_data.tbn_c[0][2] = nrm_c.x;
	per_tri_data.tbn_c[1][2] = nrm_c.y;
	per_tri_data.tbn_c[2][2] = nrm_c.z;

	skinned_per_tri_datas[skinned_tri_idx] = per_tri_data;
}
