#include "zp_cpp/gpu.hpp"

// ====================================================================================================================
// ====================================================================================================================
// ====================================================================================================================
// ====================================================================================================================
void zp::gpu::HiddenDeviceLocalBuff::init(VkDevice device, VkPhysicalDevice phys_dev, VkDeviceSize BUFF_SIZE, VkBufferUsageFlags usage)
{
    util::create_buff(device, phys_dev, BUFF_SIZE, usage, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT | VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, handle, memory);
    if (usage & VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT)
    {
        device_addr = util::get_buff_dev_addr(device, handle);
    }
}

// ====================================================================================================================
// ====================================================================================================================
// ====================================================================================================================
// ====================================================================================================================
void zp::gpu::HiddenDeviceLocalBuff::cleanup(VkDevice device)
{
    vkDestroyBuffer(device, handle, nullptr);
    vkFreeMemory(device, memory, nullptr);
}