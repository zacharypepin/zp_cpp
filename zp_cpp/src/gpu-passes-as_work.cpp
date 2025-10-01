#include "zp_cpp/gpu.hpp"

// ================================================================================================================
// ================================================================================================================
// ================================================================================================================
// ================================================================================================================
void zp::gpu::passes::as_work::Instance::record_init(VkCommandBuffer cmd_buff) {}

// ================================================================================================================
// ================================================================================================================
// ================================================================================================================
// ================================================================================================================
void zp::gpu::passes::as_work::Instance::record_cmd_buff(VkCommandBuffer cmd_buff)
{
    // ============================================================================================
    // ============================================================================================
    // rebuild requested blases
    // ============================================================================================
    // ============================================================================================
    for (auto&& info : shared.requested_rebuild_infos)
    {
        Blas* p_blas = setup.p_blas_store->fetch(info.blas_id);

        p_blas->record_build_tri_blas(setup.p_inst, setup.vk_dev, setup.vk_phys_dev, cmd_buff, info.verts_buff_addr, setup.p_idcs_buff->deviceAddress, (uint32_t*)setup.p_idcs_buff->p_mapped, info.verts_regions, info.idcs_regions);

        zp::gpu::util::record_buff_barrier(setup.p_inst, setup.vk_dev, cmd_buff, p_blas->buffer, 0, VK_WHOLE_SIZE);

        zp::gpu::util::record_buff_barrier(setup.p_inst, setup.vk_dev, cmd_buff, p_blas->scratch_buff.handle, 0, VK_WHOLE_SIZE);

        // todo
        // make util func
        {
            VkMemoryBarrier2 barrier   = {};
            barrier.sType              = VK_STRUCTURE_TYPE_MEMORY_BARRIER_2;
            barrier.srcStageMask       = VK_PIPELINE_STAGE_2_ALL_COMMANDS_BIT;
            barrier.srcAccessMask      = VK_ACCESS_2_ACCELERATION_STRUCTURE_WRITE_BIT_KHR | VK_ACCESS_2_ACCELERATION_STRUCTURE_READ_BIT_KHR;
            barrier.dstStageMask       = VK_PIPELINE_STAGE_2_ALL_COMMANDS_BIT;
            barrier.dstAccessMask      = VK_ACCESS_2_ACCELERATION_STRUCTURE_WRITE_BIT_KHR | VK_ACCESS_2_ACCELERATION_STRUCTURE_READ_BIT_KHR;
            barrier.pNext              = nullptr;

            VkDependencyInfo depInfo   = {};
            depInfo.sType              = VK_STRUCTURE_TYPE_DEPENDENCY_INFO;
            depInfo.memoryBarrierCount = 1;
            depInfo.pMemoryBarriers    = &barrier;
            depInfo.pNext              = nullptr;

            vkCmdPipelineBarrier2(cmd_buff, &depInfo);
        }
    }

    // ============================================================================================
    // ============================================================================================
    // rebuild tlas
    // ============================================================================================
    // ============================================================================================
    setup.p_tlas->record_build(setup.p_inst, cmd_buff);
}
