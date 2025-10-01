#include "zp_cpp/gpu.hpp"

// ================================================================================================================
// ================================================================================================================
// ================================================================================================================
// ================================================================================================================
void zp::gpu::shortcuts::standard_desc_set_layout(VkDevice device, std::vector<std::tuple<VkDescriptorType, uint32_t, VkShaderStageFlags>> bind_infos, VkDescriptorSetLayout* p_set_layout)
{
    std::vector<VkDescriptorSetLayoutBinding> bindings(bind_infos.size());
    for (size_t i = 0; i < bind_infos.size(); i++)
    {
        auto& binding           = bindings[i];
        binding.binding         = i;
        binding.descriptorType  = std::get<0>(bind_infos[i]);
        binding.descriptorCount = std::get<1>(bind_infos[i]);
        binding.stageFlags      = std::get<2>(bind_infos[i]);
    }

    VkDescriptorSetLayoutCreateInfo layoutInfo = {};
    layoutInfo.sType                           = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    layoutInfo.bindingCount                    = static_cast<uint32_t>(bindings.size());
    layoutInfo.pBindings                       = bindings.data();

    VK_CHECK(vkCreateDescriptorSetLayout(device, &layoutInfo, nullptr, p_set_layout), "create descriptor set layout");
}

// ================================================================================================================
// ================================================================================================================
// ================================================================================================================
// ================================================================================================================
void zp::gpu::shortcuts::standard_setup_desc_set(
    VkDevice device, VkDescriptorSetLayout desc_set_layout, VkDescriptorPool desc_pool, VkDescriptorSet* p_desc_set, std::vector<std::tuple<VkDescriptorType, std::vector<VkDescriptorBufferInfo>, std::vector<VkDescriptorImageInfo>, std::vector<VkWriteDescriptorSetAccelerationStructureKHR>>> infos
)
{
    // ================================================================================
    // ================================================================================
    // allocate
    // ================================================================================
    // ================================================================================
    {
        VkDescriptorSetAllocateInfo info = {};
        info.sType                       = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
        info.descriptorPool              = desc_pool;
        info.descriptorSetCount          = 1;
        info.pSetLayouts                 = &desc_set_layout;

        VK_CHECK(vkAllocateDescriptorSets(device, &info, p_desc_set), "allocating descriptor sets");
    }

    // ================================================================================
    // ================================================================================
    // xd
    // ================================================================================
    // ================================================================================
    std::vector<VkWriteDescriptorSet> writes;
    {
        writes.resize(infos.size());
        for (size_t i = 0; i < infos.size(); i++)
        {

            auto type             = std::get<0>(infos[i]);
            auto& buffer_infos    = std::get<1>(infos[i]);
            auto& image_infos     = std::get<2>(infos[i]);
            auto& as_infos        = std::get<3>(infos[i]);

            auto& write           = writes[i];
            write.sType           = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
            write.dstSet          = *p_desc_set;
            write.dstBinding      = i;
            write.dstArrayElement = 0;
            write.descriptorType  = type;

            if (!buffer_infos.empty())
            {
                write.descriptorCount = buffer_infos.size();
                write.pBufferInfo     = buffer_infos.data();
            }
            else if (!image_infos.empty())
            {
                write.descriptorCount = image_infos.size();
                write.pImageInfo      = image_infos.data();
            }
            else if (!as_infos.empty())
            {
                write.descriptorCount = as_infos.size();
                write.pNext           = as_infos.data();
            }
        }
    }

    {
        vkUpdateDescriptorSets(device, writes.size(), writes.data(), 0, nullptr);
    }
}

// ================================================================================================================
// ================================================================================================================
// todo
// i think if multiple push constant sizes, the offset needs setting from the previous one?
// ================================================================================================================
// ================================================================================================================
void zp::gpu::shortcuts::standard_compute_pipeline(VkDevice device, VkShaderModule shader_module, std::vector<size_t> push_constant_configs, VkDescriptorSetLayout* p_set_layout, VkPipelineLayout* p_pipeline_layout, VkPipeline* p_pipeline)
{
    VkPipelineShaderStageCreateInfo shader_info        = {};
    shader_info.sType                                  = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    shader_info.stage                                  = VK_SHADER_STAGE_COMPUTE_BIT;
    shader_info.module                                 = shader_module;
    shader_info.pName                                  = "main";

    std::vector<VkPushConstantRange> push_const_ranges = {};
    for (auto&& size : push_constant_configs)
    {
        VkPushConstantRange push_const_range = {};
        push_const_range.stageFlags          = VK_SHADER_STAGE_COMPUTE_BIT;
        push_const_range.offset              = 0; // todo
        push_const_range.size                = size;

        push_const_ranges.push_back(push_const_range);
    }

    VkPipelineLayoutCreateInfo layout_info = {};
    layout_info.sType                      = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    layout_info.setLayoutCount             = 1;
    layout_info.pSetLayouts                = p_set_layout;
    layout_info.pushConstantRangeCount     = push_const_ranges.size();
    layout_info.pPushConstantRanges        = push_const_ranges.data();

    VK_CHECK(vkCreatePipelineLayout(device, &layout_info, nullptr, p_pipeline_layout), "creating pre draw pipeline layout");

    VkComputePipelineCreateInfo pipeline_info = {};
    pipeline_info.sType                       = VK_STRUCTURE_TYPE_COMPUTE_PIPELINE_CREATE_INFO;
    pipeline_info.flags                       = 0;
    pipeline_info.stage                       = shader_info;
    pipeline_info.layout                      = *p_pipeline_layout;
    pipeline_info.basePipelineHandle          = VK_NULL_HANDLE;
    pipeline_info.basePipelineIndex           = 0;
    pipeline_info.pNext                       = nullptr;

    VK_CHECK(vkCreateComputePipelines(device, VK_NULL_HANDLE, 1, &pipeline_info, nullptr, p_pipeline), "creating pre draw pipeline");
}