#include "zp_cpp/gpu.hpp"

using namespace zp::gpu;
using namespace zp::gpu::passes;
using namespace zp::gpu::passes::compute;

// ========================================================================================================================================
// ========================================================================================================================================
// ========================================================================================================================================
// ========================================================================================================================================
#define IMPL p_inst->compute_sys.p_i

// ========================================================================================================================================
// ========================================================================================================================================
// ========================================================================================================================================
// ========================================================================================================================================
namespace
{
}

// ========================================================================================================================================
// ========================================================================================================================================
// ========================================================================================================================================
// ========================================================================================================================================
struct System::Internal
{
    // todo
    // move these to shared part of zgpu inst
    PFN_vkCmdBindDescriptorBuffersEXT vkCmdBindDescriptorBuffersEXT;
    PFN_vkCmdSetDescriptorBufferOffsetsEXT vkCmdSetDescriptorBufferOffsetsEXT;

    std::unordered_map<shader_group_uuid, VkPipeline> pipelines;
};

// ========================================================================================================================================
// ========================================================================================================================================
// ========================================================================================================================================
// ========================================================================================================================================
void compute::init(Instance* p_inst)
{
    // ============================================================================================
    // ============================================================================================
    // ============================================================================================
    // ============================================================================================
    {
        IMPL = new System::Internal();
    }

    // ========================================================================================
    // ========================================================================================
    // ========================================================================================
    // ========================================================================================
    {
        IMPL->vkCmdBindDescriptorBuffersEXT      = (PFN_vkCmdBindDescriptorBuffersEXT)vkGetDeviceProcAddr(p_inst->vk_dev, "vkCmdBindDescriptorBuffersEXT");
        IMPL->vkCmdSetDescriptorBufferOffsetsEXT = (PFN_vkCmdSetDescriptorBufferOffsetsEXT)vkGetDeviceProcAddr(p_inst->vk_dev, "vkCmdSetDescriptorBufferOffsetsEXT");
    }
}

// ========================================================================================================================================
// ========================================================================================================================================
// ========================================================================================================================================
// ========================================================================================================================================
void compute::record_cmd_buff(Instance* p_inst, VkCommandBuffer cmd_buff, std::vector<DispatchReq>* p_dispatch_reqs)
{
    // ========================================================================================
    // ========================================================================================
    // ========================================================================================
    // ========================================================================================
    {
        for (const auto& dispatch_req : *p_dispatch_reqs)
        {
            bool is_cached = IMPL->pipelines.contains(dispatch_req.shader_group);

            if (!is_cached)
            {
                VkShaderModule sh_mod;
                zp::gpu::util::create_shader_module(p_inst->vk_dev, sh_mod, &p_inst->shader_groups.at(dispatch_req.shader_group).comp);

                VkPipelineShaderStageCreateInfo shader_info = {};
                shader_info.sType                           = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
                shader_info.stage                           = VK_SHADER_STAGE_COMPUTE_BIT;
                shader_info.module                          = sh_mod;
                shader_info.pName                           = "main";

                VkComputePipelineCreateInfo pipeline_info   = {};
                pipeline_info.sType                         = VK_STRUCTURE_TYPE_COMPUTE_PIPELINE_CREATE_INFO;
                pipeline_info.pNext                         = nullptr;
                pipeline_info.flags                         = 0;
                pipeline_info.stage                         = shader_info;
                pipeline_info.layout                        = p_inst->desc_sys.vk_pipeline_layout;
                pipeline_info.basePipelineHandle            = VK_NULL_HANDLE;
                pipeline_info.basePipelineIndex             = 0;
                pipeline_info.flags                         = VK_PIPELINE_CREATE_DESCRIPTOR_BUFFER_BIT_EXT;

                VK_CHECK(vkCreateComputePipelines(p_inst->vk_dev, VK_NULL_HANDLE, 1, &pipeline_info, nullptr, &IMPL->pipelines[dispatch_req.shader_group]), "vkCreateComputePipelines");

                vkDestroyShaderModule(p_inst->vk_dev, sh_mod, nullptr);
            }
        }
    }

    // ========================================================================================
    // ========================================================================================
    // dispatch
    // ========================================================================================
    // ========================================================================================
    {
        for (const auto& dispatch_req : *p_dispatch_reqs)
        {
            vkCmdBindPipeline(cmd_buff, VK_PIPELINE_BIND_POINT_COMPUTE, IMPL->pipelines.at(dispatch_req.shader_group));

            // desc buffer
            {
                VkDescriptorBufferBindingInfoEXT bindInfo{VK_STRUCTURE_TYPE_DESCRIPTOR_BUFFER_BINDING_INFO_EXT};
                bindInfo.address = p_inst->desc_sys.desc_buff.deviceAddress;
                bindInfo.usage   = VK_BUFFER_USAGE_SAMPLER_DESCRIPTOR_BUFFER_BIT_EXT | VK_BUFFER_USAGE_RESOURCE_DESCRIPTOR_BUFFER_BIT_EXT;
                IMPL->vkCmdBindDescriptorBuffersEXT(cmd_buff, 1, &bindInfo);

                uint32_t bufIndex      = 0;
                VkDeviceSize setOffset = 0;
                IMPL->vkCmdSetDescriptorBufferOffsetsEXT(cmd_buff, VK_PIPELINE_BIND_POINT_COMPUTE, p_inst->desc_sys.vk_pipeline_layout, 0, 1, &bufIndex, &setOffset);
            }

            vkCmdPushConstants(cmd_buff, p_inst->desc_sys.vk_pipeline_layout, VK_SHADER_STAGE_COMPUTE_BIT | VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(VkDeviceAddress), &dispatch_req.p_per_dispatch);
            vkCmdDispatch(cmd_buff, dispatch_req.num_groups_x, dispatch_req.num_groups_y, dispatch_req.num_groups_z);
        }
    }
}

// ========================================================================================================================================
// ========================================================================================================================================
// todo
// ========================================================================================================================================
// ========================================================================================================================================
void compute::cleanup(Instance* p_inst)
{
    TODO("cleanup");
}

// ========================================================================================================================================
// ========================================================================================================================================
// ========================================================================================================================================
// ========================================================================================================================================
#undef IMPL