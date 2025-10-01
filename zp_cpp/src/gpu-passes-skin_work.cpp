#include "zp_cpp/gpu.hpp"

#include "zp_cpp/gpu/shader_hex.hpp"

// ====================================================================================================================
// ====================================================================================================================
// ====================================================================================================================
// ====================================================================================================================
void zp::gpu::passes::skin_work::Instance::record_init(VkCommandBuffer cmd_buff)
{
    // ========================================================================================
    // ========================================================================================
    // dispatch buff
    // ========================================================================================
    // ========================================================================================
    {
        state.dispatch_skins_verts_buff.init(setup.device, setup.phys_dev, sizeof(DispatchSkinVerts), setup.max_dispatches, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT);

        state.dispatch_skins_nrms_buff.init(setup.device, setup.phys_dev, sizeof(DispatchSkinNrms), setup.max_dispatches, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT);
    }

    // ========================================================================================
    // ========================================================================================
    // create descriptor set layout
    // ========================================================================================
    // ========================================================================================
    {
        zp::gpu::shortcuts::standard_desc_set_layout(
            setup.device,
            {
                {VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1, VK_SHADER_STAGE_COMPUTE_BIT},
                {VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1, VK_SHADER_STAGE_COMPUTE_BIT},
                {VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1, VK_SHADER_STAGE_COMPUTE_BIT},
                {VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1, VK_SHADER_STAGE_COMPUTE_BIT},
                {VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1, VK_SHADER_STAGE_COMPUTE_BIT}
        },
            &state.desc_set_layout_verts
        );

        zp::gpu::shortcuts::standard_desc_set_layout(
            setup.device,
            {
                {VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1, VK_SHADER_STAGE_COMPUTE_BIT},
                {VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1, VK_SHADER_STAGE_COMPUTE_BIT},
                {VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1, VK_SHADER_STAGE_COMPUTE_BIT},
                {VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1, VK_SHADER_STAGE_COMPUTE_BIT},
                {VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1, VK_SHADER_STAGE_COMPUTE_BIT},
                {VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1, VK_SHADER_STAGE_COMPUTE_BIT}
        },
            &state.desc_set_layout_nrms
        );
    }

    // ========================================================================================
    // ========================================================================================
    // setup descriptor sets
    // ========================================================================================
    // ========================================================================================
    {
        zp::gpu::shortcuts::standard_setup_desc_set(
            setup.device,
            state.desc_set_layout_verts,
            setup.desc_pool,
            &state.desc_set_verts,
            {
                {VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, {state.dispatch_skins_verts_buff.desc_info()}, {}, {}},
                {VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,             {setup.p_verts_buff->desc_info()}, {}, {}},
                {VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,        {setup.p_joint_mats_buff->desc_info()}, {}, {}},
                {VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,              {setup.p_rigs_buff->desc_info()}, {}, {}},
                {VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,     {setup.p_skinned_verts_buff->desc_info()}, {}, {}}
        }
        );

        zp::gpu::shortcuts::standard_setup_desc_set(
            setup.device,
            state.desc_set_layout_nrms,
            setup.desc_pool,
            &state.desc_set_nrms,
            {
                {VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, {state.dispatch_skins_nrms_buff.desc_info()}, {}, {}},
                {VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,             {setup.p_idcs_buff->desc_info()}, {}, {}},
                {VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,         {setup.p_per_tris_buff->desc_info()}, {}, {}},
                {VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,       {setup.p_joint_mats_buff->desc_info()}, {}, {}},
                {VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,             {setup.p_rigs_buff->desc_info()}, {}, {}},
                {VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, {setup.p_skinned_per_tris_buff->desc_info()}, {}, {}}
        }
        );
    }

    // ========================================================================================
    // ========================================================================================
    // create shader module
    // ========================================================================================
    // ========================================================================================
    {
        zp::gpu::util::create_shader_module(setup.device, state.shader_verts, comp_skin_verts_comp_spv, comp_skin_verts_comp_spv_len);

        zp::gpu::util::create_shader_module(setup.device, state.shader_nrms, comp_skin_nrms_comp_spv, comp_skin_nrms_comp_spv_len);
    }

    // ========================================================================================
    // ========================================================================================
    // create compute pipeline
    // ========================================================================================
    // ========================================================================================
    {
        zp::gpu::shortcuts::standard_compute_pipeline(setup.device, state.shader_verts, {sizeof(uint32_t)}, &state.desc_set_layout_verts, &state.pipeline_layout_verts, &state.pipeline_verts);

        zp::gpu::shortcuts::standard_compute_pipeline(setup.device, state.shader_nrms, {sizeof(uint32_t)}, &state.desc_set_layout_nrms, &state.pipeline_layout_nrms, &state.pipeline_nrms);
    }
}

// ====================================================================================================================
// ====================================================================================================================
// ====================================================================================================================
// ====================================================================================================================
void zp::gpu::passes::skin_work::Instance::update_buffs()
{
    state.dispatch_skins_verts_buff.reset();
    state.dispatch_skins_verts_buff.push(shared.verts_dispatches.data(), shared.verts_dispatches.size());

    state.dispatch_skins_nrms_buff.reset();
    state.dispatch_skins_nrms_buff.push(shared.nrms_dispatches.data(), shared.nrms_dispatches.size());
}

// ====================================================================================================================
// ====================================================================================================================
// ====================================================================================================================
// ====================================================================================================================
void zp::gpu::passes::skin_work::Instance::record_cmd_buff(VkCommandBuffer cmd_buff)
{
    // verts
    {
        vkCmdBindPipeline(cmd_buff, VK_PIPELINE_BIND_POINT_COMPUTE, state.pipeline_verts);
        vkCmdBindDescriptorSets(cmd_buff, VK_PIPELINE_BIND_POINT_COMPUTE, state.pipeline_layout_verts, 0, 1, &state.desc_set_verts, 0, 0);

        for (uint32_t i = 0; i < state.dispatch_skins_verts_buff.count; i++)
        {
            DispatchSkinVerts* p_dispatch_skins = reinterpret_cast<DispatchSkinVerts*>(state.dispatch_skins_verts_buff.p_mapped);

            auto idx_copy                       = i;
            vkCmdPushConstants(cmd_buff, state.pipeline_layout_verts, VK_SHADER_STAGE_COMPUTE_BIT, 0, sizeof(uint32_t), &idx_copy);
            const uint32_t local_size_x = 128;
            uint32_t vert_count         = p_dispatch_skins[i].vert_count;
            uint32_t nb_workgroups_x    = (vert_count + local_size_x - 1) / local_size_x;
            vkCmdDispatch(cmd_buff, nb_workgroups_x, 1, 1);
        }
    }

    // nrms
    {
        vkCmdBindPipeline(cmd_buff, VK_PIPELINE_BIND_POINT_COMPUTE, state.pipeline_nrms);
        vkCmdBindDescriptorSets(cmd_buff, VK_PIPELINE_BIND_POINT_COMPUTE, state.pipeline_layout_nrms, 0, 1, &state.desc_set_nrms, 0, 0);

        for (uint32_t i = 0; i < state.dispatch_skins_nrms_buff.count; i++)
        {
            DispatchSkinNrms* p_dispatch_skins = reinterpret_cast<DispatchSkinNrms*>(state.dispatch_skins_nrms_buff.p_mapped);

            auto idx_copy                      = i;
            vkCmdPushConstants(cmd_buff, state.pipeline_layout_nrms, VK_SHADER_STAGE_COMPUTE_BIT, 0, sizeof(uint32_t), &idx_copy);
            const uint32_t local_size_x = 128;
            uint32_t nb_workgroups_x    = (p_dispatch_skins[i].tri_count + local_size_x - 1) / local_size_x;
            vkCmdDispatch(cmd_buff, nb_workgroups_x, 1, 1);
        }
    }
}