#include "zp_cpp/gpu.hpp"

// ====================================================================================================================
// ====================================================================================================================
// ====================================================================================================================
// ====================================================================================================================
uint32_t zp::gpu::util::aligned_size(uint32_t value, uint32_t alignment)
{
    return (value + alignment - 1) & ~(alignment - 1);
}

// ====================================================================================================================
// ====================================================================================================================
// ====================================================================================================================
// ====================================================================================================================
size_t zp::gpu::util::aligned_size(size_t value, size_t alignment)
{
    return (value + alignment - 1) & ~(alignment - 1);
}

// ====================================================================================================================
// ====================================================================================================================
// ====================================================================================================================
// ====================================================================================================================
VkDeviceSize zp::gpu::util::aligned_vk_size(VkDeviceSize value, VkDeviceSize alignment)
{
    return (value + alignment - 1) & ~(alignment - 1);
}

// ====================================================================================================================
// ====================================================================================================================
// ====================================================================================================================
// ====================================================================================================================
VkDeviceAddress zp::gpu::util::get_as_dev_addr(zp::gpu::Instance* p_inst, VkDevice device, VkAccelerationStructureKHR as)
{
    VkAccelerationStructureDeviceAddressInfoKHR info = {};
    info.sType                                       = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_DEVICE_ADDRESS_INFO_KHR;
    info.accelerationStructure                       = as;
    return p_inst->func_ptrs.vkGetAccelerationStructureDeviceAddressKHR(device, &info);
}
// ====================================================================================================================
// ====================================================================================================================
// ====================================================================================================================
// ====================================================================================================================
VkDeviceAddress zp::gpu::util::get_buff_dev_addr(VkDevice device, VkBuffer buff)
{
    VkBufferDeviceAddressInfo info = {};
    info.sType                     = VK_STRUCTURE_TYPE_BUFFER_DEVICE_ADDRESS_INFO;
    info.buffer                    = buff;
    return vkGetBufferDeviceAddress(device, &info);
}

// ====================================================================================================================
// ====================================================================================================================
// ====================================================================================================================
// ====================================================================================================================
uint32_t zp::gpu::util::find_mem_type(VkPhysicalDevice phys_dev, uint32_t filter, VkMemoryPropertyFlags prop_flags)
{
    VkPhysicalDeviceMemoryProperties phys_dev_mem_props;
    vkGetPhysicalDeviceMemoryProperties(phys_dev, &phys_dev_mem_props);
    for (uint32_t i = 0; i < phys_dev_mem_props.memoryTypeCount; i++)
    {
        if ((filter & (1 << i)) && (phys_dev_mem_props.memoryTypes[i].propertyFlags & prop_flags) == prop_flags)
        {
            return i;
        }
    }
    ERR("failed to find suitable memory type");
    return 0;
}

// ====================================================================================================================
// ====================================================================================================================
// ====================================================================================================================
// ====================================================================================================================
void zp::gpu::util::create_buff(VkDevice device, VkPhysicalDevice phys_dev, VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags props, VkBuffer& buff, VkDeviceMemory& mem)
{
    // ============================================================================================
    // ============================================================================================
    // create buff
    // ============================================================================================
    // ============================================================================================
    {
        VkBufferCreateInfo info = {};
        info.sType              = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
        info.size               = size;
        info.usage              = usage;
        info.sharingMode        = VK_SHARING_MODE_EXCLUSIVE;

        VK_CHECK(vkCreateBuffer(device, &info, nullptr, &buff), "creating buffer");
    }

    // ============================================================================================
    // ============================================================================================
    // allocate memory
    // ============================================================================================
    // ============================================================================================
    {
        VkMemoryRequirements memRequirements;
        vkGetBufferMemoryRequirements(device, buff, &memRequirements);

        VkMemoryPriorityAllocateInfoEXT prio_info  = {};
        prio_info.sType                            = VK_STRUCTURE_TYPE_MEMORY_PRIORITY_ALLOCATE_INFO_EXT;
        prio_info.priority                         = 1.0f;

        VkMemoryAllocateInfo allocInfo             = {};
        allocInfo.sType                            = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
        allocInfo.allocationSize                   = memRequirements.size;
        allocInfo.memoryTypeIndex                  = find_mem_type(phys_dev, memRequirements.memoryTypeBits, props);
        allocInfo.pNext                            = &prio_info;

        // if VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT usage bit, set appropriate memory alloc flag
        VkMemoryAllocateFlagsInfo alloc_flags_info = {};
        {
            if (usage & VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT)
            {
                alloc_flags_info.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_FLAGS_INFO_KHR;
                alloc_flags_info.flags = VK_MEMORY_ALLOCATE_DEVICE_ADDRESS_BIT;
                prio_info.pNext        = &alloc_flags_info;
            }
        }

        VK_CHECK(vkAllocateMemory(device, &allocInfo, nullptr, &mem), "allocating buffer memory");
    }

    // ============================================================================================
    // ============================================================================================
    // bind memory
    // ============================================================================================
    // ============================================================================================
    {
        VK_CHECK(vkBindBufferMemory(device, buff, mem, 0), "binding buffer memory");
    }
}

// ====================================================================================================================
// ====================================================================================================================
// ====================================================================================================================
// ====================================================================================================================
std::vector<VkPhysicalDevice> zp::gpu::util::get_candidate_phys_devs(VkInstance instance)
{
    uint32_t count = 0;
    vkEnumeratePhysicalDevices(instance, &count, nullptr);

    if (count == 0)
    {
        ERR("failed to find GPUs with Vulkan support!");
    }

    std::vector<VkPhysicalDevice> candidates(count);
    vkEnumeratePhysicalDevices(instance, &count, candidates.data());

    return candidates;
}

// ====================================================================================================================
// ====================================================================================================================
// ====================================================================================================================
// ====================================================================================================================
zp::gpu::util::QueueFamiliesDetails zp::gpu::util::query_queue_families(VkPhysicalDevice physicalDevice, VkSurfaceKHR surface)
{
    QueueFamiliesDetails indices = {};
    uint32_t queueFamilyCount    = 0;
    vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &queueFamilyCount, nullptr);
    std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
    vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &queueFamilyCount, queueFamilies.data());
    int i = 0;
    for (const auto& queueFamily : queueFamilies)
    {
        if (queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT)
        {
            indices.graphicsFamily = i;
        }
        VkBool32 presentSupport = VK_FALSE;
        vkGetPhysicalDeviceSurfaceSupportKHR(physicalDevice, i, surface, &presentSupport);
        if (presentSupport)
        {
            indices.presentFamily = i;
        }
        if (indices.graphicsFamily.has_value() && indices.presentFamily.has_value())
        {
            break;
        }
        i++;
    }
    return indices;
}

// ====================================================================================================================
// ====================================================================================================================
// ====================================================================================================================
// ====================================================================================================================
void zp::gpu::util::record_trans_image_layout(Instance* p_inst, VkCommandBuffer cmd_buff, VkImage image, VkFormat format, VkImageLayout prev_layout, VkImageLayout next_layout)
{
    VkImageAspectFlags aspectMask            = next_layout == VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL ? VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT : VK_IMAGE_ASPECT_COLOR_BIT;

    VkImageSubresourceRange subresourceRange = {};
    subresourceRange.aspectMask              = aspectMask;
    subresourceRange.baseMipLevel            = 0;
    subresourceRange.levelCount              = VK_REMAINING_MIP_LEVELS;
    subresourceRange.baseArrayLayer          = 0;
    subresourceRange.layerCount              = VK_REMAINING_ARRAY_LAYERS;

    VkImageMemoryBarrier2 imageBarrier       = {};
    imageBarrier.sType                       = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER_2;
    imageBarrier.srcStageMask                = VK_PIPELINE_STAGE_2_ALL_COMMANDS_BIT;
    imageBarrier.srcAccessMask               = VK_ACCESS_2_MEMORY_WRITE_BIT;
    imageBarrier.dstStageMask                = VK_PIPELINE_STAGE_2_ALL_COMMANDS_BIT;
    imageBarrier.dstAccessMask               = VK_ACCESS_2_MEMORY_WRITE_BIT | VK_ACCESS_2_MEMORY_READ_BIT;
    imageBarrier.oldLayout                   = prev_layout;
    imageBarrier.newLayout                   = next_layout;
    imageBarrier.subresourceRange            = subresourceRange;
    imageBarrier.image                       = image;
    imageBarrier.pNext                       = nullptr;

    VkDependencyInfo depInfo                 = {};
    depInfo.sType                            = VK_STRUCTURE_TYPE_DEPENDENCY_INFO;
    depInfo.imageMemoryBarrierCount          = 1;
    depInfo.pImageMemoryBarriers             = &imageBarrier;
    depInfo.pNext                            = nullptr;

    if (p_inst->using_vk_1_2)
    {
        p_inst->func_ptrs.vkCmdPipelineBarrier2(cmd_buff, &depInfo);
    }
    else
    {
        vkCmdPipelineBarrier2(cmd_buff, &depInfo);
    }
}

// ====================================================================================================================
// ====================================================================================================================
// ====================================================================================================================
// ====================================================================================================================
void zp::gpu::util::create_shader_module(VkDevice device, VkShaderModule& shader_module, std::vector<uint8_t> shader_bytes)
{
    VkShaderModuleCreateInfo info = {};
    info.sType                    = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
    info.codeSize                 = shader_bytes.size();
    info.pCode                    = reinterpret_cast<const uint32_t*>(shader_bytes.data());

    VK_CHECK(vkCreateShaderModule(device, &info, nullptr, &shader_module), "creating shader module");
}

// ====================================================================================================================
// ====================================================================================================================
// ====================================================================================================================
// ====================================================================================================================
void zp::gpu::util::create_shader_module(VkDevice device, VkShaderModule& shader_module, void* p_code, size_t code_size)
{
    VkShaderModuleCreateInfo info = {};
    info.sType                    = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
    info.codeSize                 = code_size;
    info.pCode                    = reinterpret_cast<const uint32_t*>(p_code);

    VK_CHECK(vkCreateShaderModule(device, &info, nullptr, &shader_module), "creating shader module");
}

// ====================================================================================================================
// ====================================================================================================================
// ====================================================================================================================
// ====================================================================================================================
void zp::gpu::util::create_shader_module(VkDevice device, VkShaderModule& shader_module, Shader* p_shader)
{
    VkShaderModuleCreateInfo info = {};
    info.sType                    = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
    info.codeSize                 = p_shader->size;
    info.pCode                    = reinterpret_cast<const uint32_t*>(p_shader->p_data);

    VK_CHECK(vkCreateShaderModule(device, &info, nullptr, &shader_module), "creating shader module");
}

// ================================================================================================================
// ================================================================================================================
// ================================================================================================================
// ================================================================================================================
void zp::gpu::util::record_blit(Instance* p_inst, VkCommandBuffer cmd_buff, VkImage src, VkImage dst, VkExtent2D src_size, VkExtent2D dst_size)
{
    VkImageBlit2 blit_2                  = {};
    blit_2.sType                         = VK_STRUCTURE_TYPE_IMAGE_BLIT_2;
    blit_2.srcOffsets[1].x               = src_size.width;
    blit_2.srcOffsets[1].y               = src_size.height;
    blit_2.srcOffsets[1].z               = 1;
    blit_2.dstOffsets[1].x               = dst_size.width;
    blit_2.dstOffsets[1].y               = dst_size.height;
    blit_2.dstOffsets[1].z               = 1;
    blit_2.srcSubresource.aspectMask     = VK_IMAGE_ASPECT_COLOR_BIT;
    blit_2.srcSubresource.baseArrayLayer = 0;
    blit_2.srcSubresource.layerCount     = 1;
    blit_2.srcSubresource.mipLevel       = 0;
    blit_2.dstSubresource.aspectMask     = VK_IMAGE_ASPECT_COLOR_BIT;
    blit_2.dstSubresource.baseArrayLayer = 0;
    blit_2.dstSubresource.layerCount     = 1;
    blit_2.dstSubresource.mipLevel       = 0;
    blit_2.pNext                         = nullptr;

    VkBlitImageInfo2 info                = {};
    info.sType                           = VK_STRUCTURE_TYPE_BLIT_IMAGE_INFO_2;
    info.dstImage                        = dst;
    info.dstImageLayout                  = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
    info.srcImage                        = src;
    info.srcImageLayout                  = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
    info.filter                          = VK_FILTER_LINEAR;
    info.regionCount                     = 1;
    info.pRegions                        = &blit_2;
    info.pNext                           = nullptr;

    if (p_inst->using_vk_1_2)
    {
        p_inst->func_ptrs.vkCmdBlitImage2(cmd_buff, &info);
    }
    else
    {
        vkCmdBlitImage2(cmd_buff, &info);
    }
}

// ================================================================================================================
// ================================================================================================================
// ================================================================================================================
// ================================================================================================================
void zp::gpu::util::record_buff_barrier(Instance* p_inst, VkDevice device, VkCommandBuffer cmd_buff, VkBuffer buffer, VkDeviceSize offset, VkDeviceSize size)
{
    VkBufferMemoryBarrier2 barrier   = {};
    barrier.sType                    = VK_STRUCTURE_TYPE_BUFFER_MEMORY_BARRIER_2;
    barrier.srcStageMask             = VK_PIPELINE_STAGE_2_ALL_COMMANDS_BIT;
    barrier.srcAccessMask            = VK_ACCESS_2_MEMORY_WRITE_BIT;
    barrier.dstStageMask             = VK_PIPELINE_STAGE_2_ALL_COMMANDS_BIT;
    barrier.dstAccessMask            = VK_ACCESS_2_MEMORY_WRITE_BIT | VK_ACCESS_2_MEMORY_READ_BIT;
    barrier.buffer                   = buffer;
    barrier.offset                   = offset;
    barrier.size                     = size;
    barrier.pNext                    = nullptr;

    VkDependencyInfo depInfo         = {};
    depInfo.sType                    = VK_STRUCTURE_TYPE_DEPENDENCY_INFO;
    depInfo.bufferMemoryBarrierCount = 1;
    depInfo.pBufferMemoryBarriers    = &barrier;
    depInfo.pNext                    = nullptr;

    if (p_inst->using_vk_1_2)
    {
        p_inst->func_ptrs.vkCmdPipelineBarrier2(cmd_buff, &depInfo);
    }
    else
    {
        vkCmdPipelineBarrier2(cmd_buff, &depInfo);
    }
}

// ================================================================================================================
// ================================================================================================================
// ================================================================================================================
// ================================================================================================================
VkTransformMatrixKHR zp::gpu::util::convert_mat4(const zp::math::mat4& mat)
{
    return VkTransformMatrixKHR{
        {
         mat[0][0],
         mat[0][1],
         mat[0][2],
         mat[0][3],
         mat[1][0],
         mat[1][1],
         mat[1][2],
         mat[1][3],
         mat[2][0],
         mat[2][1],
         mat[2][2],
         mat[2][3],
         }
    };
}

// ================================================================================================================
// ================================================================================================================
// ================================================================================================================
// ================================================================================================================
void zp::gpu::util::record_copy_buff_to_image(VkCommandBuffer cmd_buff, VkBuffer buff, VkDeviceSize offset, VkImage dst, uint32_t width, uint32_t height, uint32_t num_channels, size_t pixel_size, uint32_t mip_levels)
{
    uint32_t running_width           = width;
    uint32_t running_height          = height;
    VkDeviceSize running_mips_offset = 0;

    std::vector<VkBufferImageCopy> copies(mip_levels);

    for (size_t i = 0; i < mip_levels; i++)
    {
        VkBufferImageCopy& copy               = copies[i];
        copy.bufferOffset                     = offset + running_mips_offset;
        copy.bufferRowLength                  = 0;
        copy.bufferImageHeight                = 0;
        copy.imageSubresource.aspectMask      = VK_IMAGE_ASPECT_COLOR_BIT;
        copy.imageSubresource.mipLevel        = i;
        copy.imageSubresource.baseArrayLayer  = 0;
        copy.imageSubresource.layerCount      = 1;
        copy.imageOffset                      = {0, 0, 0};
        copy.imageExtent                      = {running_width, running_height, 1};

        VkDeviceSize per_mip_size             = running_width * running_height * num_channels * pixel_size;

        running_width                        /= 2;
        running_height                       /= 2;
        running_mips_offset                  += per_mip_size;
    }

    vkCmdCopyBufferToImage(cmd_buff, buff, dst, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, copies.size(), copies.data());
}