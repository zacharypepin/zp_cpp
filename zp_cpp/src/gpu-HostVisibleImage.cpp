#include "zp_cpp/gpu.hpp"

// ====================================================================================================================
// ====================================================================================================================
// ====================================================================================================================
// ====================================================================================================================
void zp::gpu::HostVisibleImage::init(VkDevice device, VkPhysicalDevice phys_dev, uint32_t width, uint32_t height, uint32_t num_channels, size_t pixel_size, uint32_t mip_levels, VkFormat format, VkImageUsageFlags usage, VkImageAspectFlags flags, std::vector<uint32_t> queue_families)
{
    this->device       = device;
    this->width        = width;
    this->height       = height;
    this->num_channels = num_channels;
    this->pixel_size   = pixel_size;
    this->mip_levels   = mip_levels;
    this->format       = format;

    // ====================================================================================
    // ====================================================================================
    // create image
    // ====================================================================================
    // ====================================================================================
    {
        VkSharingMode sharing_mode;
        {
            sharing_mode = queue_families.size() > 1 ? VK_SHARING_MODE_CONCURRENT : VK_SHARING_MODE_EXCLUSIVE;
        }

        VkImageCreateInfo info     = {};
        info.sType                 = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
        info.imageType             = VK_IMAGE_TYPE_2D;
        info.extent.width          = width;
        info.extent.height         = height;
        info.extent.depth          = 1;
        info.mipLevels             = mip_levels;
        info.arrayLayers           = 1;
        info.format                = format;
        info.tiling                = VK_IMAGE_TILING_LINEAR;
        info.initialLayout         = VK_IMAGE_LAYOUT_UNDEFINED;
        info.usage                 = usage;
        info.samples               = VK_SAMPLE_COUNT_1_BIT;
        info.sharingMode           = sharing_mode;
        info.queueFamilyIndexCount = static_cast<uint32_t>(queue_families.size());
        info.pQueueFamilyIndices   = queue_families.data();

        VK_CHECK(vkCreateImage(device, &info, nullptr, &handle), "creating image");
    }

    // ====================================================================================
    // ====================================================================================
    // allocate and bind memory
    // ====================================================================================
    // ====================================================================================
    {
        VkMemoryRequirements mem_require;
        vkGetImageMemoryRequirements(device, handle, &mem_require);

        VkMemoryPropertyFlags mem_props            = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT;
        mem_props                                 |= VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;

        uint32_t memory_type_idx                   = util::find_mem_type(phys_dev, mem_require.memoryTypeBits, mem_props);

        VkMemoryPriorityAllocateInfoEXT prio_info  = {};
        prio_info.sType                            = VK_STRUCTURE_TYPE_MEMORY_PRIORITY_ALLOCATE_INFO_EXT;
        prio_info.priority                         = 1.0f;

        VkMemoryAllocateInfo alloc_info            = {};
        alloc_info.sType                           = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
        alloc_info.allocationSize                  = mem_require.size;
        alloc_info.memoryTypeIndex                 = memory_type_idx;
        alloc_info.pNext                           = &prio_info;

        VK_CHECK(vkAllocateMemory(device, &alloc_info, nullptr, &memory), "allocating image memory");

        VK_CHECK(vkBindImageMemory(device, handle, memory, 0), "binding image memory");
    }

    // ====================================================================================
    // ====================================================================================
    // map memory, offset to correct image layout
    // ====================================================================================
    // ====================================================================================
    {
        VkSubresourceLayout subResourceLayout;
        {
            VkImageSubresource subResource = {};
            subResource.aspectMask         = VK_IMAGE_ASPECT_COLOR_BIT;
            vkGetImageSubresourceLayout(device, handle, &subResource, &subResourceLayout);
        }

        void* imageData;
        vkMapMemory(device, memory, 0, VK_WHOLE_SIZE, 0, &imageData);
        p_mapped = static_cast<unsigned char*>(imageData) + subResourceLayout.offset;
    }

    // ====================================================================================
    // ====================================================================================
    // create view
    // ====================================================================================
    // ====================================================================================
    {
        VkImageViewCreateInfo info           = {};
        info.sType                           = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
        info.image                           = handle;
        info.viewType                        = VK_IMAGE_VIEW_TYPE_2D;
        info.format                          = format;
        info.subresourceRange.aspectMask     = flags;
        info.subresourceRange.baseMipLevel   = 0;
        info.subresourceRange.levelCount     = mip_levels;
        info.subresourceRange.baseArrayLayer = 0;
        info.subresourceRange.layerCount     = 1;

        VK_CHECK(vkCreateImageView(device, &info, nullptr, &view), "creating image view");
    }
}

// ====================================================================================================================
// ====================================================================================================================
// ====================================================================================================================
// ====================================================================================================================
VkDescriptorImageInfo zp::gpu::HostVisibleImage::desc_info(VkImageLayout img_layout)
{
    VkDescriptorImageInfo info = {};
    info.imageView             = view;
    info.sampler               = VK_NULL_HANDLE;
    info.imageLayout           = img_layout;

    return info;
}

// ====================================================================================================================
// ====================================================================================================================
// ====================================================================================================================
// ====================================================================================================================
void zp::gpu::HostVisibleImage::cleanup()
{
    vkDestroyImageView(device, view, nullptr);
    vkDestroyImage(device, handle, nullptr);
    vkFreeMemory(device, memory, nullptr);
}