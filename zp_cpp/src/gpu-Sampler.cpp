#include "zp_cpp/gpu.hpp"

// ====================================================================================================================
// ====================================================================================================================
// ====================================================================================================================
// ====================================================================================================================
void zp::gpu::Sampler::init(VkDevice device)
{
    VkSamplerCreateInfo info = {};
    info.sType               = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
    info.minFilter           = VK_FILTER_LINEAR;
    info.magFilter           = VK_FILTER_LINEAR;
    info.mipmapMode          = VK_SAMPLER_MIPMAP_MODE_LINEAR;
    info.addressModeU        = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER;
    info.addressModeV        = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER;
    info.anisotropyEnable    = VK_TRUE;
    info.maxAnisotropy       = 16.0f;
    info.minLod              = 0.f;
    info.maxLod              = 15.f;
    info.mipLodBias          = 0.f;

    VK_CHECK(vkCreateSampler(device, &info, 0, &handle), "creating sampler");
}

// ====================================================================================================================
// ====================================================================================================================
// ====================================================================================================================
// ====================================================================================================================
VkDescriptorImageInfo zp::gpu::Sampler::desc_info()
{
    VkDescriptorImageInfo info = {};
    info.sampler               = handle;

    return info;
}