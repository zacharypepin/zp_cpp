#include "zp_cpp/gpu.hpp"

// ============================================================================================================
// ============================================================================================================
// ============================================================================================================
// ============================================================================================================
void zp::gpu::CmdBuffPool::init(Instance* p_inst, VkDevice device, uint32_t queue_family_idx)
{
    this->p_inst = p_inst;
    this->device = device;

    // ====================================================================================
    // ====================================================================================
    // create command pool
    // ====================================================================================
    // ====================================================================================
    {
        VkCommandPoolCreateInfo info = {};
        info.sType                   = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
        info.queueFamilyIndex        = queue_family_idx;

        VK_CHECK(vkCreateCommandPool(device, &info, nullptr, &commandPool), "failed to create command pool");
    }

    // ====================================================================================
    // ====================================================================================
    // allocate
    // ====================================================================================
    // ====================================================================================
    std::array<VkCommandBuffer, MAX_COMMAND_BUFFERS> allocated;
    {
        VkCommandBufferAllocateInfo allocInfo = {};
        allocInfo.sType                       = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
        allocInfo.commandPool                 = commandPool;
        allocInfo.level                       = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
        allocInfo.commandBufferCount          = MAX_COMMAND_BUFFERS;

        VK_CHECK(vkAllocateCommandBuffers(device, &allocInfo, allocated.data()), "allocating command buffers");
    }

    // ====================================================================================
    // ====================================================================================
    // create wrappers
    // ====================================================================================
    // ====================================================================================
    {
        for (int i = 0; i < MAX_COMMAND_BUFFERS; i++)
        {
            buffs[i] = allocated[i];
        }
    }
}

// ============================================================================================================
// ============================================================================================================
// ============================================================================================================
// ============================================================================================================
void zp::gpu::CmdBuffPool::reset()
{
    VK_CHECK(vkResetCommandPool(device, commandPool, 0), "resetting command pool");
    curr_idx = 0;
}

// ============================================================================================================
// ============================================================================================================
// ============================================================================================================
// ============================================================================================================
void zp::gpu::CmdBuffPool::cleanup()
{
    vkFreeCommandBuffers(device, commandPool, buffs.size(), buffs.data());
    vkDestroyCommandPool(device, commandPool, nullptr);
}

// ============================================================================================================
// ============================================================================================================
// ============================================================================================================
// ============================================================================================================
VkCommandBuffer zp::gpu::CmdBuffPool::acquire()
{
    if (curr_idx >= buffs.size())
    {
        ERR("Out of command buffers, max was: " << MAX_COMMAND_BUFFERS);
    }

    // ====================================================================================
    // ====================================================================================
    // inc
    // ====================================================================================
    // ====================================================================================
    VkCommandBuffer buff;
    {
        buff = buffs[curr_idx];
        curr_idx++;
    }

    // ====================================================================================
    // ====================================================================================
    // begin
    // ====================================================================================
    // ====================================================================================
    {
        VkCommandBufferBeginInfo beginInfo = {};
        beginInfo.sType                    = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
        beginInfo.flags                    = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

        vkBeginCommandBuffer(buff, &beginInfo);
    }

    return buff;
}

// ============================================================================================================
// ============================================================================================================
// ============================================================================================================
// ============================================================================================================
void zp::gpu::CmdBuffPool::submit(
    VkCommandBuffer buff,
    VkQueue queue,
    std::vector<std::tuple<VkSemaphore, VkPipelineStageFlags2>> wait_pairs,
    std::vector<std::tuple<VkSemaphore, VkPipelineStageFlags2>> signal_pairs,
    VkFence fence,
    std::vector<std::tuple<VkSemaphore, VkPipelineStageFlags2, uint64_t>> timeline_waits,
    std::vector<std::tuple<VkSemaphore, VkPipelineStageFlags2, uint64_t>> timeline_signals
)
{
    vkEndCommandBuffer(buff);

    // ====================================================================================
    // ====================================================================================
    // ====================================================================================
    // ====================================================================================
    std::vector<VkSemaphoreSubmitInfo> wait_infos(wait_pairs.size() + timeline_waits.size());
    {
        for (size_t i = 0; i < wait_pairs.size(); i++)
        {
            VkSemaphoreSubmitInfo& info = wait_infos[i];
            info.sType                  = VK_STRUCTURE_TYPE_SEMAPHORE_SUBMIT_INFO;
            info.semaphore              = std::get<0>(wait_pairs[i]);
            info.stageMask              = std::get<1>(wait_pairs[i]);
        }
        for (size_t i = 0; i < timeline_waits.size(); i++)
        {
            VkSemaphoreSubmitInfo& info = wait_infos[wait_pairs.size() + i];
            info.sType                  = VK_STRUCTURE_TYPE_SEMAPHORE_SUBMIT_INFO;
            info.semaphore              = std::get<0>(timeline_waits[i]);
            info.stageMask              = std::get<1>(timeline_waits[i]);
            info.value                  = std::get<2>(timeline_waits[i]);
        }
    }

    // ====================================================================================
    // ====================================================================================
    // ====================================================================================
    // ====================================================================================
    std::vector<VkSemaphoreSubmitInfo> signal_infos(signal_pairs.size() + timeline_signals.size());
    {
        for (size_t i = 0; i < signal_pairs.size(); i++)
        {
            VkSemaphoreSubmitInfo& info = signal_infos[i];
            info.sType                  = VK_STRUCTURE_TYPE_SEMAPHORE_SUBMIT_INFO;
            info.semaphore              = std::get<0>(signal_pairs[i]);
            info.stageMask              = std::get<1>(signal_pairs[i]);
        }
        for (size_t i = 0; i < timeline_signals.size(); i++)
        {
            VkSemaphoreSubmitInfo& info = signal_infos[signal_pairs.size() + i];
            info.sType                  = VK_STRUCTURE_TYPE_SEMAPHORE_SUBMIT_INFO;
            info.semaphore              = std::get<0>(timeline_signals[i]);
            info.stageMask              = std::get<1>(timeline_signals[i]);
            info.value                  = std::get<2>(timeline_signals[i]);
        }
    }

    // ====================================================================================
    // ====================================================================================
    // ====================================================================================
    // ====================================================================================
    VkCommandBufferSubmitInfo cmd_buff_info = {};
    cmd_buff_info.sType                     = VK_STRUCTURE_TYPE_COMMAND_BUFFER_SUBMIT_INFO;
    cmd_buff_info.commandBuffer             = buff;
    cmd_buff_info.deviceMask                = 0;
    cmd_buff_info.pNext                     = nullptr;

    VkSubmitInfo2 info                      = {};
    info.sType                              = VK_STRUCTURE_TYPE_SUBMIT_INFO_2;
    info.flags                              = 0;
    info.waitSemaphoreInfoCount             = wait_infos.size();
    info.pWaitSemaphoreInfos                = wait_infos.data();
    info.signalSemaphoreInfoCount           = signal_infos.size();
    info.pSignalSemaphoreInfos              = signal_infos.data();
    info.commandBufferInfoCount             = 1;
    info.pCommandBufferInfos                = &cmd_buff_info;
    info.pNext                              = nullptr;

    if (p_inst->using_vk_1_2)
    {
        VK_CHECK(p_inst->func_ptrs.vkQueueSubmit2(queue, 1, &info, fence), "submitting command buffer");
    }
    else
    {
        VK_CHECK(vkQueueSubmit2(queue, 1, &info, fence), "submitting command buffer");
    }
}