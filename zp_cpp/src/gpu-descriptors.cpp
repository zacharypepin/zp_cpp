#include "zp_cpp/gpu.hpp"

using namespace zp::gpu;
using namespace zp::gpu::descriptors;

// ========================================================================================================================================
// ========================================================================================================================================
// ========================================================================================================================================
// ========================================================================================================================================
#define DESC_SYS  p_inst->desc_sys
#define DESC_IMPL DESC_SYS.p_i

// ========================================================================================================================================
// ========================================================================================================================================
// ========================================================================================================================================
// ========================================================================================================================================
namespace
{
    constexpr size_t MAX_PANELS   = 12;
    constexpr size_t MAX_TEXTURES = 1024;
}

// ========================================================================================================================================
// ========================================================================================================================================
// ========================================================================================================================================
// ========================================================================================================================================
struct System::Internal
{
    VkPhysicalDeviceDescriptorBufferPropertiesEXT desc_buff_props;

    PFN_vkGetDescriptorSetLayoutSizeEXT vkGetDescriptorSetLayoutSizeEXT;
    PFN_vkGetDescriptorSetLayoutBindingOffsetEXT vkGetDescriptorSetLayoutBindingOffsetEXT;
    PFN_vkGetDescriptorEXT vkGetDescriptorEXT;

    VkDescriptorSetLayout vk_desc_set_layout;

    VkDeviceSize dsl_size;
    VkDeviceSize offset_sampler;
    VkDeviceSize offset_panels;
    VkDeviceSize offset_rw_panels;
    VkDeviceSize offset_textures;
};

// ========================================================================================================================================
// ========================================================================================================================================
// ========================================================================================================================================
// ========================================================================================================================================
void descriptors::init(Instance* p_inst)
{
    // ========================================================================================
    // ========================================================================================
    // ========================================================================================
    // ========================================================================================
    {
        DESC_IMPL = new System::Internal();
    }

    // ============================================================================================
    // ============================================================================================
    // device props
    // ============================================================================================
    // ============================================================================================
    {
        DESC_IMPL->desc_buff_props        = {};
        DESC_IMPL->desc_buff_props.sType  = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DESCRIPTOR_BUFFER_PROPERTIES_EXT;

        VkPhysicalDeviceProperties2 props = {};
        props.sType                       = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PROPERTIES_2;
        props.pNext                       = &DESC_IMPL->desc_buff_props;

        vkGetPhysicalDeviceProperties2(p_inst->vk_phys_dev, &props);
    }

    // ========================================================================================
    // ========================================================================================
    // ========================================================================================
    // ========================================================================================
    {
        DESC_IMPL->vkGetDescriptorSetLayoutSizeEXT          = (PFN_vkGetDescriptorSetLayoutSizeEXT)vkGetDeviceProcAddr(p_inst->vk_dev, "vkGetDescriptorSetLayoutSizeEXT");
        DESC_IMPL->vkGetDescriptorSetLayoutBindingOffsetEXT = (PFN_vkGetDescriptorSetLayoutBindingOffsetEXT)vkGetDeviceProcAddr(p_inst->vk_dev, "vkGetDescriptorSetLayoutBindingOffsetEXT");
        DESC_IMPL->vkGetDescriptorEXT                       = (PFN_vkGetDescriptorEXT)vkGetDeviceProcAddr(p_inst->vk_dev, "vkGetDescriptorEXT");
    }

    // ========================================================================================
    // ========================================================================================
    // dsl
    // ========================================================================================
    // ========================================================================================
    {
        std::array<VkDescriptorSetLayoutBinding, 4> bindings;
        {
            auto& binding              = bindings[0];
            binding.binding            = 0;
            binding.descriptorType     = VK_DESCRIPTOR_TYPE_SAMPLER;
            binding.descriptorCount    = 1;
            binding.stageFlags         = VK_SHADER_STAGE_COMPUTE_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;
            binding.pImmutableSamplers = nullptr;
        }
        {
            auto& binding              = bindings[1];
            binding.binding            = 1;
            binding.descriptorType     = VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE;
            binding.descriptorCount    = MAX_PANELS;
            binding.stageFlags         = VK_SHADER_STAGE_COMPUTE_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;
            binding.pImmutableSamplers = nullptr;
        }
        {
            auto& binding              = bindings[2];
            binding.binding            = 2;
            binding.descriptorType     = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
            binding.descriptorCount    = MAX_PANELS;
            binding.stageFlags         = VK_SHADER_STAGE_COMPUTE_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;
            binding.pImmutableSamplers = nullptr;
        }
        {
            auto& binding              = bindings[3];
            binding.binding            = 3;
            binding.descriptorType     = VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE;
            binding.descriptorCount    = MAX_TEXTURES;
            binding.stageFlags         = VK_SHADER_STAGE_COMPUTE_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;
            binding.pImmutableSamplers = nullptr;
        }

        VkDescriptorSetLayoutCreateInfo info = {};
        info.sType                           = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
        info.bindingCount                    = (uint32_t)bindings.size();
        info.pBindings                       = bindings.data();
        info.flags                           = VK_DESCRIPTOR_SET_LAYOUT_CREATE_DESCRIPTOR_BUFFER_BIT_EXT;

        VK_CHECK(vkCreateDescriptorSetLayout(p_inst->vk_dev, &info, nullptr, &DESC_IMPL->vk_desc_set_layout), "vkCreateDescriptorSetLayout");
    }

    // ========================================================================================
    // ========================================================================================
    // pipeline layout
    // ========================================================================================
    // ========================================================================================
    {
        const VkPushConstantRange pc_range = {
            .stageFlags = VK_SHADER_STAGE_COMPUTE_BIT | VK_SHADER_STAGE_VERTEX_BIT,
            .offset     = 0,
            .size       = sizeof(VkDeviceAddress),
        };

        VkPipelineLayoutCreateInfo info = {};
        info.sType                      = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
        info.setLayoutCount             = 1;
        info.pSetLayouts                = &DESC_IMPL->vk_desc_set_layout;
        info.pushConstantRangeCount     = 1;
        info.pPushConstantRanges        = &pc_range;

        VK_CHECK(vkCreatePipelineLayout(p_inst->vk_dev, &info, nullptr, &DESC_SYS.vk_pipeline_layout), "vkCreatePipelineLayout");
    }

    // ========================================================================================
    // ========================================================================================
    // ========================================================================================
    // ========================================================================================
    {
        DESC_IMPL->vkGetDescriptorSetLayoutSizeEXT(p_inst->vk_dev, DESC_IMPL->vk_desc_set_layout, &DESC_IMPL->dsl_size);
        DESC_IMPL->vkGetDescriptorSetLayoutBindingOffsetEXT(p_inst->vk_dev, DESC_IMPL->vk_desc_set_layout, 0, &DESC_IMPL->offset_sampler);
        DESC_IMPL->vkGetDescriptorSetLayoutBindingOffsetEXT(p_inst->vk_dev, DESC_IMPL->vk_desc_set_layout, 1, &DESC_IMPL->offset_panels);
        DESC_IMPL->vkGetDescriptorSetLayoutBindingOffsetEXT(p_inst->vk_dev, DESC_IMPL->vk_desc_set_layout, 2, &DESC_IMPL->offset_rw_panels);
        DESC_IMPL->vkGetDescriptorSetLayoutBindingOffsetEXT(p_inst->vk_dev, DESC_IMPL->vk_desc_set_layout, 3, &DESC_IMPL->offset_textures);
    }

    // ========================================================================================
    // ========================================================================================
    // ========================================================================================
    // ========================================================================================
    {
        DESC_SYS.desc_buff.init(p_inst->vk_dev, p_inst->vk_phys_dev, 1, DESC_IMPL->dsl_size, VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT | VK_BUFFER_USAGE_SAMPLER_DESCRIPTOR_BUFFER_BIT_EXT | VK_BUFFER_USAGE_RESOURCE_DESCRIPTOR_BUFFER_BIT_EXT);
    }
}

// ========================================================================================================================================
// ========================================================================================================================================
// ========================================================================================================================================
// ========================================================================================================================================
void descriptors::set_samp(Instance* p_inst, Sampler* p_samp)
{
    VkDescriptorDataEXT data = {
        .pSampler = &p_samp->handle,
    };

    VkDescriptorGetInfoEXT info = {
        .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_GET_INFO_EXT,
        .pNext = nullptr,
        .type  = VK_DESCRIPTOR_TYPE_SAMPLER,
        .data  = data,
    };

    DESC_IMPL->vkGetDescriptorEXT(p_inst->vk_dev, &info, DESC_IMPL->desc_buff_props.samplerDescriptorSize, DESC_SYS.desc_buff.p_mapped + DESC_IMPL->offset_sampler);
}

// ========================================================================================================================================
// ========================================================================================================================================
// ========================================================================================================================================
// ========================================================================================================================================
void descriptors::set_panel(Instance* p_inst, uint32_t idx, DeviceLocalImage* p_img)
{
    if (idx >= MAX_PANELS)
    {
        ERR("out of bounds");
    }

    {
        VkDescriptorImageInfo img_info = p_img->desc_info(VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

        VkDescriptorDataEXT data       = {
                  .pSampledImage = &img_info,
        };

        VkDescriptorGetInfoEXT info = {
            .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_GET_INFO_EXT,
            .pNext = nullptr,
            .type  = VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE,
            .data  = data,
        };

        DESC_IMPL->vkGetDescriptorEXT(p_inst->vk_dev, &info, DESC_IMPL->desc_buff_props.sampledImageDescriptorSize, DESC_SYS.desc_buff.p_mapped + DESC_IMPL->offset_panels + idx * DESC_IMPL->desc_buff_props.sampledImageDescriptorSize);
    }
    {
        VkDescriptorImageInfo img_info = p_img->desc_info(VK_IMAGE_LAYOUT_GENERAL);

        VkDescriptorDataEXT data       = {
                  .pStorageImage = &img_info,
        };

        VkDescriptorGetInfoEXT info = {
            .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_GET_INFO_EXT,
            .pNext = nullptr,
            .type  = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE,
            .data  = data,
        };

        DESC_IMPL->vkGetDescriptorEXT(p_inst->vk_dev, &info, DESC_IMPL->desc_buff_props.storageImageDescriptorSize, DESC_SYS.desc_buff.p_mapped + DESC_IMPL->offset_rw_panels + idx * DESC_IMPL->desc_buff_props.storageImageDescriptorSize);
    }
}

// ========================================================================================================================================
// ========================================================================================================================================
// ========================================================================================================================================
// ========================================================================================================================================
void descriptors::set_tex(Instance* p_inst, uint32_t idx, VkDescriptorImageInfo img_info)
{
    VkDescriptorDataEXT data = {
        .pSampledImage = &img_info,
    };

    VkDescriptorGetInfoEXT info = {
        .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_GET_INFO_EXT,
        .pNext = nullptr,
        .type  = VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE,
        .data  = data,
    };

    DESC_IMPL->vkGetDescriptorEXT(p_inst->vk_dev, &info, DESC_IMPL->desc_buff_props.sampledImageDescriptorSize, DESC_SYS.desc_buff.p_mapped + DESC_IMPL->offset_textures + idx * DESC_IMPL->desc_buff_props.sampledImageDescriptorSize);
}

// ========================================================================================================================================
// ========================================================================================================================================
// ========================================================================================================================================
// ========================================================================================================================================
void descriptors::cleanup(Instance* p_inst)
{
    delete DESC_IMPL;
}

// ========================================================================================================================================
// ========================================================================================================================================
// ========================================================================================================================================
// ========================================================================================================================================
#undef DESC_SYS
#undef DESC_IMPL