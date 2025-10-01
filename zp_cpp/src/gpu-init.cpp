#include "zp_cpp/gpu.hpp"

// ================================================================================================================
// ================================================================================================================
// ================================================================================================================
// ================================================================================================================
bool zp::gpu::init::is_validation_supported()
{
    uint32_t layer_count;
    vkEnumerateInstanceLayerProperties(&layer_count, nullptr);
    std::vector<VkLayerProperties> avail_layers(layer_count);
    vkEnumerateInstanceLayerProperties(&layer_count, avail_layers.data());
    for (const char* layer_name : zp::gpu::VALIDATION_LAYERS)
    {
        bool found = false;
        for (const auto& props : avail_layers)
        {
            if (strcmp(layer_name, props.layerName) == 0)
            {
                found = true;
                break;
            }
        }
        if (!found) return false;
    }
    return true;
}

// ====================================================================================================================
// ====================================================================================================================
// ====================================================================================================================
// ====================================================================================================================
zp::gpu::init::SwapChainSupportDetails zp::gpu::init::query_swapchain_support(VkSurfaceKHR surface, VkPhysicalDevice candidate)
{
    SwapChainSupportDetails details;
    vkGetPhysicalDeviceSurfaceCapabilitiesKHR(candidate, surface, &details.capabilities);
    uint32_t formatCount;
    vkGetPhysicalDeviceSurfaceFormatsKHR(candidate, surface, &formatCount, nullptr);
    if (formatCount != 0)
    {
        details.formats.resize(formatCount);
        vkGetPhysicalDeviceSurfaceFormatsKHR(candidate, surface, &formatCount, details.formats.data());
    }
    uint32_t presentModeCount;
    vkGetPhysicalDeviceSurfacePresentModesKHR(candidate, surface, &presentModeCount, nullptr);
    if (presentModeCount != 0)
    {
        details.presentModes.resize(presentModeCount);
        vkGetPhysicalDeviceSurfacePresentModesKHR(candidate, surface, &presentModeCount, details.presentModes.data());
    }
    return details;
}

// ====================================================================================================================
// ====================================================================================================================
// ====================================================================================================================
// ====================================================================================================================
void zp::gpu::init::init_inst_func_ptrs(zp::gpu::Instance& inst, VkInstance& instance, bool enable_validation)
{
    if (enable_validation)
    {
        inst.func_ptrs.vkCreateDebugUtilsMessengerEXT = reinterpret_cast<PFN_vkCreateDebugUtilsMessengerEXT>(vkGetInstanceProcAddr(instance, "vkCreateDebugUtilsMessengerEXT"));
    }
}

// ====================================================================================================================
// ====================================================================================================================
// ====================================================================================================================
// ====================================================================================================================
void zp::gpu::init::init_dev_func_ptrs(zp::gpu::Instance& inst, VkDevice& device, bool using_vk_1_2, bool enable_rt)
{
    inst.using_vk_1_2 = using_vk_1_2;

    if (using_vk_1_2)
    {
        inst.func_ptrs.vkQueueSubmit2        = ((PFN_vkQueueSubmit2KHR)(vkGetDeviceProcAddr(device, "vkQueueSubmit2KHR")));
        inst.func_ptrs.vkCmdPipelineBarrier2 = ((PFN_vkCmdPipelineBarrier2KHR)(vkGetDeviceProcAddr(device, "vkCmdPipelineBarrier2KHR")));
        inst.func_ptrs.vkCmdBeginRendering   = ((PFN_vkCmdBeginRenderingKHR)(vkGetDeviceProcAddr(device, "vkCmdBeginRenderingKHR")));
        inst.func_ptrs.vkCmdEndRendering     = ((PFN_vkCmdEndRenderingKHR)(vkGetDeviceProcAddr(device, "vkCmdEndRenderingKHR")));
        inst.func_ptrs.vkCmdBlitImage2       = ((PFN_vkCmdBlitImage2KHR)(vkGetDeviceProcAddr(device, "vkCmdBlitImage2KHR")));
    }

    if (enable_rt)
    {
        inst.func_ptrs.vkCmdBuildAccelerationStructuresKHR        = reinterpret_cast<PFN_vkCmdBuildAccelerationStructuresKHR>(vkGetDeviceProcAddr(device, "vkCmdBuildAccelerationStructuresKHR"));
        inst.func_ptrs.vkCreateAccelerationStructureKHR           = reinterpret_cast<PFN_vkCreateAccelerationStructureKHR>(vkGetDeviceProcAddr(device, "vkCreateAccelerationStructureKHR"));
        inst.func_ptrs.vkDestroyAccelerationStructureKHR          = reinterpret_cast<PFN_vkDestroyAccelerationStructureKHR>(vkGetDeviceProcAddr(device, "vkDestroyAccelerationStructureKHR"));
        inst.func_ptrs.vkGetAccelerationStructureBuildSizesKHR    = reinterpret_cast<PFN_vkGetAccelerationStructureBuildSizesKHR>(vkGetDeviceProcAddr(device, "vkGetAccelerationStructureBuildSizesKHR"));
        inst.func_ptrs.vkGetAccelerationStructureDeviceAddressKHR = reinterpret_cast<PFN_vkGetAccelerationStructureDeviceAddressKHR>(vkGetDeviceProcAddr(device, "vkGetAccelerationStructureDeviceAddressKHR"));
        inst.func_ptrs.vkCmdTraceRaysKHR                          = reinterpret_cast<PFN_vkCmdTraceRaysKHR>(vkGetDeviceProcAddr(device, "vkCmdTraceRaysKHR"));
        inst.func_ptrs.vkGetRayTracingShaderGroupHandlesKHR       = reinterpret_cast<PFN_vkGetRayTracingShaderGroupHandlesKHR>(vkGetDeviceProcAddr(device, "vkGetRayTracingShaderGroupHandlesKHR"));
        inst.func_ptrs.vkCreateRayTracingPipelinesKHR             = reinterpret_cast<PFN_vkCreateRayTracingPipelinesKHR>(vkGetDeviceProcAddr(device, "vkCreateRayTracingPipelinesKHR"));
    }
}

// ====================================================================================================================
// ====================================================================================================================
// ====================================================================================================================
// ====================================================================================================================
VkBool32 debug_callback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity, VkDebugUtilsMessageTypeFlagsEXT messageType, const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData, void* pUserData)
{
    std::string input     = std::format("{}", pCallbackData->pMessage);
    std::string delimiter = "]";
    WARN(input);
    return VK_FALSE;
}

// ====================================================================================================================
// ====================================================================================================================
// ====================================================================================================================
// ====================================================================================================================
void zp::gpu::init::setup_debug_messenger(VkInstance& instance, VkDebugUtilsMessengerEXT& debug_msgr)
{
    auto func = (PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkCreateDebugUtilsMessengerEXT");

    if (!func) ERR("failed to get vkCreateDebugUtilsMessengerEXT, ext not present?");

    VkDebugUtilsMessengerCreateInfoEXT debug_info  = {};
    debug_info.sType                               = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
    debug_info.messageSeverity                     = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT;
    debug_info.messageSeverity                    |= VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT;
    debug_info.messageSeverity                    |= VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
    debug_info.messageType                         = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT;
    debug_info.messageType                        |= VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT;
    debug_info.messageType                        |= VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
    debug_info.pfnUserCallback                     = debug_callback;

    VK_CHECK(func(instance, &debug_info, nullptr, &debug_msgr), "setting up debug messenger");
}

// ====================================================================================================================
// ====================================================================================================================
// ====================================================================================================================
// ====================================================================================================================
void zp::gpu::init::create_swapchain(
    VkDevice vk_dev,
    VkSurfaceKHR vk_surface,
    VkPhysicalDevice vk_phys_dev,
    VkFormat swapchain_ideal_format,
    VkColorSpaceKHR swapchain_ideal_colour_space,
    VkPresentModeKHR swapchain_ideal_present_mode,
    VkPresentModeKHR swapchain_fallback_present_mode,
    uint32_t framebuffer_width,
    uint32_t framebuffer_height,
    VkSwapchainKHR* p_vk_swapchain,
    std::vector<VkImage>* p_out_swapchain_imgs,
    VkFormat* p_out_swapchain_format,
    VkExtent2D* p_out_swapchain_extent
)
{
    SwapChainSupportDetails swapchain_details = query_swapchain_support(vk_surface, vk_phys_dev);

    // ============================================================================================
    // ============================================================================================
    // swapchain format
    // ============================================================================================
    // ============================================================================================
    VkSurfaceFormatKHR surface_format;
    {
        surface_format = swapchain_details.formats[0];

        for (auto&& avail_format : swapchain_details.formats)
        {
            if (avail_format.format == swapchain_ideal_format && avail_format.colorSpace == swapchain_ideal_colour_space)
            {
                surface_format = avail_format;
                break;
            }
        }
    }

    // ============================================================================================
    // ============================================================================================
    // swapchain present mode
    // ============================================================================================
    // ============================================================================================
    VkPresentModeKHR present_mode;
    {
        present_mode = swapchain_fallback_present_mode;
        for (auto&& avail_mode : swapchain_details.presentModes)
        {
            if (avail_mode == swapchain_ideal_present_mode)
            {
                present_mode = swapchain_ideal_present_mode;
                break;
            }
        }
    }

    // ============================================================================================
    // ============================================================================================
    // swapchain extent
    // ============================================================================================
    // ============================================================================================
    VkExtent2D extent;
    {
        if (swapchain_details.capabilities.currentExtent.width != UINT32_MAX)
        {
            extent = swapchain_details.capabilities.currentExtent;
        }
        else
        {
            extent.width  = std::clamp(framebuffer_width, swapchain_details.capabilities.minImageExtent.width, swapchain_details.capabilities.maxImageExtent.width);
            extent.height = std::clamp(framebuffer_height, swapchain_details.capabilities.minImageExtent.height, swapchain_details.capabilities.maxImageExtent.height);
        }
    }

    // ============================================================================================
    // ============================================================================================
    // swapchain nb images
    // ============================================================================================
    // ============================================================================================
    uint32_t image_count;
    {
        image_count = swapchain_details.capabilities.minImageCount + 1;
        if (swapchain_details.capabilities.maxImageCount > 0 && image_count > swapchain_details.capabilities.maxImageCount)
        {
            image_count = swapchain_details.capabilities.maxImageCount;
        }
    }

    // ============================================================================================
    // ============================================================================================
    // queue families and sharing mode
    // ============================================================================================
    // ============================================================================================
    VkSharingMode sharing_mode;
    uint32_t queue_family_idx_count;
    uint32_t indices[2];
    {
        zp::gpu::util::QueueFamiliesDetails details = zp::gpu::util::query_queue_families(vk_phys_dev, vk_surface);

        indices[0]                                  = details.graphicsFamily.value();
        indices[1]                                  = details.presentFamily.value();

        bool separate_graphics_and_present_family   = details.graphicsFamily != details.presentFamily;
        if (separate_graphics_and_present_family)
        {
            sharing_mode           = VK_SHARING_MODE_CONCURRENT;
            queue_family_idx_count = 2;
        }
        else
        {
            sharing_mode           = VK_SHARING_MODE_EXCLUSIVE;
            queue_family_idx_count = 0;
        }
    }

    // ============================================================================================
    // ============================================================================================
    // create
    // ============================================================================================
    // ============================================================================================
    {
        VkSwapchainCreateInfoKHR info = {};
        info.sType                    = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
        info.surface                  = vk_surface;
        info.minImageCount            = image_count;
        info.imageFormat              = surface_format.format;
        info.imageColorSpace          = surface_format.colorSpace;
        info.imageExtent              = extent;
        info.imageArrayLayers         = 1;
        info.imageUsage               = VK_IMAGE_USAGE_TRANSFER_DST_BIT;
        info.preTransform             = swapchain_details.capabilities.currentTransform;
        info.compositeAlpha           = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
        info.presentMode              = present_mode;
        info.clipped                  = VK_TRUE;
        info.imageSharingMode         = sharing_mode;
        info.queueFamilyIndexCount    = queue_family_idx_count;
        info.pQueueFamilyIndices      = indices;

        VK_CHECK(vkCreateSwapchainKHR(vk_dev, &info, nullptr, p_vk_swapchain), "creating swapchain");
    }

    // ========================================================================================
    // ========================================================================================
    // get swapchain images
    // ========================================================================================
    // ========================================================================================
    {
        uint32_t image_count;
        vkGetSwapchainImagesKHR(vk_dev, *p_vk_swapchain, &image_count, nullptr);
        p_out_swapchain_imgs->resize(image_count);
        vkGetSwapchainImagesKHR(vk_dev, *p_vk_swapchain, &image_count, p_out_swapchain_imgs->data());
        *p_out_swapchain_format = surface_format.format;
        *p_out_swapchain_extent = extent;
    }
}