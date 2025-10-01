struct DispatchSkin {
	float3 skel_space_mesh_origin;
    uint start_rig_idx;
    uint start_vert_idx;
    uint start_skinned_vert_idx;
    uint vert_count;
    uint start_joint_mat_idx;
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
[[vk::binding(1)]] StructuredBuffer<float3, ScalarDataLayout> verts;
[[vk::binding(2)]] StructuredBuffer<JointMat, ScalarDataLayout> joint_mats;
[[vk::binding(3)]] StructuredBuffer<Rig> rigs;
[[vk::binding(4)]] RWStructuredBuffer<float3, ScalarDataLayout> skinned_verts;

[numthreads(128, 1, 1)]
[shader("compute")]
void comp_main(uint3 dispatch_thread_id : SV_DispatchThreadID, uniform uint dispatch_idx)
{
    uint vert_idx = dispatch_thread_id.x;

    DispatchSkin to_skin = to_skins[dispatch_idx];

    if (vert_idx >= to_skin.vert_count)
    {
        return;
    }

    uint idx_scratch = to_skin.start_vert_idx + vert_idx;
    uint idx_skinned = to_skin.start_skinned_vert_idx + vert_idx;

    float3 vert = verts[idx_scratch];

	// ============================================================================================
	// ============================================================================================
	// verts need to be in skel space, as thats the space joint matrices are intended
	// ============================================================================================
	// ============================================================================================
	{
		vert += to_skin.skel_space_mesh_origin;
	}

	// ============================================================================================
	// ============================================================================================
	// ============================================================================================
	// ============================================================================================
    float3 pos = float3(0, 0, 0);
    {
		Rig rig = rigs[to_skin.start_rig_idx + vert_idx];

		// if (length(rig.joint_weights) != 1)
		// {
		// 	pos = vert;
		// }
		// else 
		{
			for (int j = 0; j < 4; j++)
			{
				int joint_idx = int(rig.joint_idcs[j]);
				float weight = rig.joint_weights[j];
				
				float4x4 joint_pos_mat = joint_mats[to_skin.start_joint_mat_idx + joint_idx].pos_mat;
	
				pos += weight * mul(joint_pos_mat, float4(vert, 1.0)).xyz;
			}
		}
	}

	// ============================================================================================
	// ============================================================================================
	// back to mesh space now
	// ============================================================================================
	// ============================================================================================
	{
		pos -= to_skin.skel_space_mesh_origin;
	}

    skinned_verts[idx_skinned] = pos;
}
