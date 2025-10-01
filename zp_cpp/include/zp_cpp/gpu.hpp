#pragma once

#include <array>
#include <functional>
#include <iostream>
#include <memory>
#include <optional>
#include <queue>
#include <stdexcept>
#include <sstream>
#include <string>
#include <thread>

#include <cstddef>
#include <cstdint>
#include <tuple>
#include <unordered_map>
#include <vector>
#include <cstring>

#include <vulkan/vulkan.h>
#include <vulkan/vulkan_beta.h>

#include "zp_cpp/math.hpp"
#include "zp_cpp/uuid.hpp"
#include "zp_cpp/dbg.hpp"

// ====================================================================================================================
// ====================================================================================================================
// ====================================================================================================================
// ====================================================================================================================
#define VK_CHECK(vkFunc, title) \
    { \
        VkResult result = vkFunc; \
        if (result != VK_SUCCESS) ERR("failed: " << title << ", error: " << result); \
    }

// ====================================================================================================================
// ====================================================================================================================
// ====================================================================================================================
// ====================================================================================================================
inline void hash_combine(std::size_t& seed, std::size_t hash)
{
    seed ^= hash + 0x9e3779b9 + (seed << 6) + (seed >> 2);
}

struct tuple_hash
{
    template <typename Tuple, std::size_t Index = 0> struct Hasher
    {
        static void apply(const Tuple& t, std::size_t& seed)
        {
            if constexpr (Index < std::tuple_size_v<Tuple>)
            {
                hash_combine(seed, std::hash<std::tuple_element_t<Index, Tuple>>{}(std::get<Index>(t)));
                Hasher<Tuple, Index + 1>::apply(t, seed);
            }
        }
    };

    template <typename... Args> std::size_t operator()(const std::tuple<Args...>& t) const
    {
        std::size_t seed = 0;
        Hasher<std::tuple<Args...>>::apply(t, seed);
        return seed;
    }
};

// ====================================================================================================================
// ====================================================================================================================
// ====================================================================================================================
// ====================================================================================================================
namespace zp::gpu
{
    struct RegionHandle
    {
        void* ptr;
        uint32_t start_idx;
        uint32_t count;
    };

    template <typename... Args> using RegionMap = std::unordered_map<std::tuple<Args...>, RegionHandle, tuple_hash>;

    struct Shader
    {
        void* p_data;
        size_t size;
    };

    using shader_group_uuid = zp::uuid::uuid;

    struct ShaderGroup
    {
        shader_group_uuid uuid;
        Shader vert;
        Shader frag;
        Shader comp;
        Shader rgen;
        Shader miss;
        Shader chit;
        Shader ahit;
        Shader intr;
    };
}

// ====================================================================================================================
// ====================================================================================================================
// consts
// ====================================================================================================================
// ====================================================================================================================
namespace zp::gpu
{
    static constexpr std::array<const char*, 1> VALIDATION_LAYERS = {{"VK_LAYER_KHRONOS_validation"}};

    static constexpr const char* RT_DEVICE_EXTENSIONS[]           = {
        VK_KHR_ACCELERATION_STRUCTURE_EXTENSION_NAME,
        VK_KHR_DEFERRED_HOST_OPERATIONS_EXTENSION_NAME,
        VK_KHR_RAY_TRACING_PIPELINE_EXTENSION_NAME,
        VK_KHR_RAY_TRACING_POSITION_FETCH_EXTENSION_NAME,
    };

    static constexpr const char* MEMORY_DEVICE_EXTENSIONS[] = {
        VK_EXT_MEMORY_PRIORITY_EXTENSION_NAME,
        VK_EXT_PAGEABLE_DEVICE_LOCAL_MEMORY_EXTENSION_NAME,
    };

    struct Instance;
}

// ====================================================================================================================
// ====================================================================================================================
// ====================================================================================================================
// ====================================================================================================================
namespace zp::gpu
{
    // ================================================================================================================
    // ================================================================================================================
    // ================================================================================================================
    // ================================================================================================================
    struct CmdBuffPool
    {
        private:
        static const int MAX_COMMAND_BUFFERS = 20;
        Instance* p_inst;
        VkDevice device;
        VkCommandPool commandPool;
        std::array<VkCommandBuffer, MAX_COMMAND_BUFFERS> buffs;
        int curr_idx = 0;

        public:
        void init(Instance* p_inst, VkDevice device, uint32_t queue_family_idx);
        void reset();
        void cleanup();
        VkCommandBuffer acquire();
        void submit(
            VkCommandBuffer buff,
            VkQueue queue,
            std::vector<std::tuple<VkSemaphore, VkPipelineStageFlags2>> wait_pairs                 = {},
            std::vector<std::tuple<VkSemaphore, VkPipelineStageFlags2>> signal_pairs               = {},
            VkFence fence                                                                          = VK_NULL_HANDLE,
            std::vector<std::tuple<VkSemaphore, VkPipelineStageFlags2, uint64_t>> timeline_waits   = {},
            std::vector<std::tuple<VkSemaphore, VkPipelineStageFlags2, uint64_t>> timeline_signals = {}
        );
    };

    // ================================================================================================================
    // ================================================================================================================
    // ================================================================================================================
    // ================================================================================================================
    struct DeviceBuff4
    {
        private:
        VkDeviceMemory memory;

        public:
        VkBuffer handle;
        VkDeviceAddress deviceAddress;
        std::byte* p_mapped;
        size_t stride;
        uint32_t max_count;
        uint32_t count = 0;

        void init(VkDevice device, VkPhysicalDevice phys_dev, size_t stride, uint32_t max_count, VkBufferUsageFlags usage);
        RegionHandle bump(uint32_t num);
        RegionHandle push(const void* src, uint32_t num);
        VkDeviceAddress get_offset_addr(uint32_t idx);
        void remove(RegionHandle region);
        size_t size();
        VkDescriptorBufferInfo desc_info();
        void reset();
        void cleanup(VkDevice device);
    };
    struct StagedDeviceBuff4
    {
        private:
        VkDeviceMemory memory;
        std::vector<std::byte> staging_buff;
        void* p_mapped_device;

        public:
        VkBuffer handle;
        VkDeviceAddress deviceAddress;
        std::byte* p_mapped;
        size_t stride;
        uint32_t max_count;
        uint32_t count = 0;

        void init(VkDevice device, VkPhysicalDevice phys_dev, size_t stride, uint32_t max_count, VkBufferUsageFlags usage);
        RegionHandle bump(uint32_t num);
        RegionHandle push(std::byte* src, uint32_t num);
        void remove(RegionHandle region);
        void push_device();
        size_t size();
        VkDescriptorBufferInfo desc_info();
        void reset();
        void cleanup(VkDevice device);
    };

    // ================================================================================================================
    // ================================================================================================================
    // ================================================================================================================
    // ================================================================================================================
    struct HostBuff2
    {
        private:
        VkDeviceMemory memory;

        public:
        VkBuffer handle;
        std::byte* p_mapped;
        size_t stride;
        uint32_t max_count;
        uint32_t count = 0;

        void init(VkDevice device, VkPhysicalDevice phys_dev, size_t stride, uint32_t max_count, VkBufferUsageFlags usage);
        RegionHandle bump(uint32_t num);
        RegionHandle push(std::byte* src, uint32_t num);
        void remove(RegionHandle region);
        size_t size();
        void reset();
        void cleanup(VkDevice device);
    };

    // ================================================================================================================
    // ================================================================================================================
    // ================================================================================================================
    // ================================================================================================================
    struct HiddenDeviceLocalBuff
    {
        private:
        VkDeviceMemory memory;

        public:
        VkBuffer handle;
        VkDeviceAddress device_addr;
        void init(VkDevice device, VkPhysicalDevice phys_dev, VkDeviceSize BUFF_SIZE, VkBufferUsageFlags usage);
        void cleanup(VkDevice device);
    };

    // ================================================================================================================
    // ================================================================================================================
    // ================================================================================================================
    // ================================================================================================================
    struct DeviceLocalImage
    {
        private:
        VkDevice device;
        VkDeviceMemory memory;

        public:
        bool is_init;
        VkImage handle;
        VkImageView view;
        uint32_t width;
        uint32_t height;
        uint32_t num_channels;
        size_t pixel_size;
        uint32_t mip_levels;
        VkFormat format;

        void init(VkDevice device, VkPhysicalDevice phys_dev, uint32_t width, uint32_t height, uint32_t num_channels, size_t pixel_size, uint32_t mip_levels, VkFormat format, VkImageUsageFlags usage, VkImageAspectFlags flags, std::vector<uint32_t> queue_families);
        VkDescriptorImageInfo desc_info(VkImageLayout img_layout);
        void cleanup();
    };

    // ================================================================================================================
    // ================================================================================================================
    // ================================================================================================================
    // ================================================================================================================
    struct HostVisibleImage
    {
        private:
        VkDevice device;
        VkDeviceMemory memory;

        public:
        VkImage handle;
        VkImageView view;
        uint32_t width;
        uint32_t height;
        uint32_t num_channels;
        size_t pixel_size;
        uint32_t mip_levels;
        VkFormat format;
        void* p_mapped;

        void init(VkDevice device, VkPhysicalDevice phys_dev, uint32_t width, uint32_t height, uint32_t num_channels, size_t pixel_size, uint32_t mip_levels, VkFormat format, VkImageUsageFlags usage, VkImageAspectFlags flags, std::vector<uint32_t> queue_families);
        VkDescriptorImageInfo desc_info(VkImageLayout img_layout);
        void cleanup();
    };

    // ================================================================================================================
    // ================================================================================================================
    // ================================================================================================================
    // ================================================================================================================
    struct DeviceLocalImageArray
    {
        private:
        std::vector<DeviceLocalImage> images;

        public:
        uint32_t MAX_COUNT;
        void init(VkDevice vk_dev, VkPhysicalDevice vk_phys_dev, uint32_t max_count);
        std::vector<VkDescriptorImageInfo> desc_infos(VkImageLayout img_layout);
        DeviceLocalImage* get(uint32_t idx);
    };

    // ================================================================================================================
    // ================================================================================================================
    // ================================================================================================================
    // ================================================================================================================
    struct Tlas
    {
        uint32_t MAX_INSTS;
        VkAccelerationStructureKHR handle;
        VkDeviceMemory memory;
        VkBuffer buffer;
        DeviceBuff4 insts_buff;
        HiddenDeviceLocalBuff scratch_buff;
        void init(Instance* p_inst, VkDevice device, VkPhysicalDevice phys_dev, uint32_t MAX_INSTS);
        void record_build(Instance* p_inst, VkCommandBuffer cmd_buff);
        VkWriteDescriptorSetAccelerationStructureKHR desc_info();
    };

    // ================================================================================================================
    // ================================================================================================================
    // ================================================================================================================
    // ================================================================================================================
    class Blas
    {
        private:
        Instance* p_inst;
        VkDevice device;

        public:
        VkAccelerationStructureKHR handle;
        VkDeviceMemory memory;
        VkBuffer buffer;
        uint64_t deviceAddress;
        HiddenDeviceLocalBuff scratch_buff;
        int num_tris_initialised = -1;

        void init_tri_blas(Instance* p_inst, VkDevice device, VkPhysicalDevice phys_dev, std::vector<uint32_t> submesh_tri_counts);

        void record_build_tri_blas(
            zp::gpu::Instance* p_inst, VkDevice vk_dev, VkPhysicalDevice vk_phys_dev, VkCommandBuffer cmd_buff, VkDeviceAddress verts_buff_addr, VkDeviceAddress idcs_buff_addr, uint32_t* p_mapped_idcs, std::vector<RegionHandle> submesh_verts_regions, std::vector<RegionHandle> submesh_idcs_regions
        );

        void record_setup_sphere_blas(Instance* p_inst, VkDevice device, VkPhysicalDevice phys_dev, VkCommandBuffer cmd_buff, VkDeviceAddress aabb_pos_device_addr);

        void cleanup();
    };

    // ================================================================================================================
    // ================================================================================================================
    // ================================================================================================================
    // ================================================================================================================
    class BlasStore
    {
        public:
        using BlasId = uint64_t;

        private:
        BlasId next_id    = 0;
        uint32_t tail_idx = 0;
        std::vector<Blas> blas_arr;
        std::unordered_map<BlasId, uint32_t> id_to_idx;

        public:
        void init(uint32_t max_num)
        {
            blas_arr.resize(max_num);
        }

        BlasId add(Blas** pp_blas)
        {
            if (tail_idx >= blas_arr.size())
            {
                ERR("max num exceeded");
            }
            BlasId id = next_id++;
            id_to_idx.insert({id, tail_idx});
            *pp_blas = &blas_arr[tail_idx++];

            return id;
        }

        Blas* fetch(BlasId id)
        {
            return &blas_arr[id_to_idx.at(id)];
        }

        void reset()
        {
            for (size_t i = 0; i < tail_idx; i++)
            {
                // todo
                // breaks nvidia nsight
                blas_arr[i].cleanup();
            }
            tail_idx = 0;
            id_to_idx.clear();
        }
    };

    // ================================================================================================================
    // ================================================================================================================
    // ================================================================================================================
    // ================================================================================================================
    struct Sampler
    {
        VkSampler handle;

        void init(VkDevice device);
        VkDescriptorImageInfo desc_info();
    };
}

// ====================================================================================================================
// ====================================================================================================================
// ====================================================================================================================
// ====================================================================================================================
namespace zp::gpu::descriptors
{
    struct System
    {
        // exposed
        VkPipelineLayout vk_pipeline_layout;
        DeviceBuff4 desc_buff;

        struct Internal;
        Internal* p_i;
    };

    void init(Instance* p_inst);
    void cleanup(Instance* p_inst);

    void set_samp(Instance* p_inst, Sampler* p_samp);
    void set_panel(Instance* p_inst, uint32_t idx, DeviceLocalImage* p_img);
    void set_tex(Instance* p_inst, uint32_t idx, VkDescriptorImageInfo img_info);
}

// ====================================================================================================================
// ====================================================================================================================
// ====================================================================================================================
// ====================================================================================================================
namespace zp::gpu::passes::compute
{
    struct DispatchReq
    {
        shader_group_uuid shader_group;
        VkDeviceAddress p_per_dispatch;
        uint32_t num_groups_x;
        uint32_t num_groups_y;
        uint32_t num_groups_z;
    };

    struct System
    {
        struct Internal;
        Internal* p_i;
    };

    void init(Instance* p_inst);
    void cleanup(Instance* p_inst);

    void record_cmd_buff(Instance* p_inst, VkCommandBuffer cmd_buff, std::vector<DispatchReq>* p_dispatch_reqs);
}

// ====================================================================================================================
// ====================================================================================================================
// ====================================================================================================================
// ====================================================================================================================
namespace zp::gpu::passes::graphics
{
    struct DrawReq
    {
        zp::math::rect scissor_nrm;
        shader_group_uuid shader_group;
        bool is_point_draw;
        bool is_line_draw;
        bool is_alpha_blend;
        bool should_depth_test;
        bool should_depth_write;
        uint32_t idx_count;
        uint32_t inst_count;
        VkDeviceAddress p_per_draw;
    };

    struct System
    {
        struct Internal;
        Internal* p_i;
    };

    void init(Instance* p_inst);
    void cleanup(Instance* p_inst);

    void record_cmd_buff(Instance* p_inst, VkCommandBuffer cmd_buff, DeviceLocalImage* p_colour, DeviceLocalImage* p_depth, bool should_clear, std::vector<DrawReq>* p_draw_reqs);
}

// ====================================================================================================================
// ====================================================================================================================
// resources
// ====================================================================================================================
// ====================================================================================================================
namespace zp::gpu
{
    // ================================================================================================================
    // ================================================================================================================
    // ================================================================================================================
    // ================================================================================================================
    struct Instance
    {
        struct FuncPtrs
        {
            PFN_vkQueueSubmit2 vkQueueSubmit2;
            PFN_vkCmdPipelineBarrier2 vkCmdPipelineBarrier2;
            PFN_vkCmdBeginRendering vkCmdBeginRendering;
            PFN_vkCmdEndRendering vkCmdEndRendering;
            PFN_vkCmdBlitImage2 vkCmdBlitImage2;

            PFN_vkCmdBuildAccelerationStructuresKHR vkCmdBuildAccelerationStructuresKHR;
            PFN_vkCreateAccelerationStructureKHR vkCreateAccelerationStructureKHR;
            PFN_vkDestroyAccelerationStructureKHR vkDestroyAccelerationStructureKHR;
            PFN_vkGetAccelerationStructureBuildSizesKHR vkGetAccelerationStructureBuildSizesKHR;
            PFN_vkGetAccelerationStructureDeviceAddressKHR vkGetAccelerationStructureDeviceAddressKHR;
            PFN_vkCmdTraceRaysKHR vkCmdTraceRaysKHR;
            PFN_vkGetRayTracingShaderGroupHandlesKHR vkGetRayTracingShaderGroupHandlesKHR;
            PFN_vkCreateRayTracingPipelinesKHR vkCreateRayTracingPipelinesKHR;

            PFN_vkCreateDebugUtilsMessengerEXT vkCreateDebugUtilsMessengerEXT;
        };
        FuncPtrs func_ptrs;

        bool using_vk_1_2;

        VkPhysicalDevice vk_phys_dev;
        VkDevice vk_dev;

        std::unordered_map<shader_group_uuid, ShaderGroup> shader_groups;

        descriptors::System desc_sys;
        passes::compute::System compute_sys;
        passes::graphics::System graphics_sys;
    };

    void init_2(Instance* p_inst);
}

// ====================================================================================================================
// ====================================================================================================================
// init
// ====================================================================================================================
// ====================================================================================================================
namespace zp::gpu::init
{
    void init_inst_func_ptrs(Instance& inst, VkInstance& instance, bool enable_validation);
    void init_dev_func_ptrs(Instance& inst, VkDevice& device, bool using_vk_1_2, bool enable_rt);

    void setup_debug_messenger(VkInstance& instance, VkDebugUtilsMessengerEXT& debug_msgr);

    bool is_validation_supported();

    struct SwapChainSupportDetails
    {
        VkSurfaceCapabilitiesKHR capabilities;
        std::vector<VkSurfaceFormatKHR> formats;
        std::vector<VkPresentModeKHR> presentModes;
    };
    SwapChainSupportDetails query_swapchain_support(VkSurfaceKHR surface, VkPhysicalDevice candidate);

    void create_swapchain(
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
    );
}

// ====================================================================================================================
// ====================================================================================================================
// util
// ====================================================================================================================
// ====================================================================================================================
namespace zp::gpu::util
{
    uint32_t aligned_size(uint32_t value, uint32_t alignment);
    size_t aligned_size(size_t value, size_t alignment);
    VkDeviceSize aligned_vk_size(VkDeviceSize value, VkDeviceSize alignment);

    VkDeviceAddress get_as_dev_addr(zp::gpu::Instance* p_inst, VkDevice device, VkAccelerationStructureKHR as);
    VkDeviceAddress get_buff_dev_addr(VkDevice device, VkBuffer buff);

    uint32_t find_mem_type(VkPhysicalDevice phys_dev, uint32_t filter, VkMemoryPropertyFlags prop_flags);

    void create_buff(VkDevice device, VkPhysicalDevice phys_dev, VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags props, VkBuffer& buff, VkDeviceMemory& mem);

    std::vector<VkPhysicalDevice> get_candidate_phys_devs(VkInstance instance);

    struct QueueFamiliesDetails
    {
        std::optional<uint32_t> graphicsFamily;
        std::optional<uint32_t> presentFamily;
    };
    QueueFamiliesDetails query_queue_families(VkPhysicalDevice phys_dev, VkSurfaceKHR surface);

    void record_trans_image_layout(Instance* p_inst, VkCommandBuffer cmd_buff, VkImage image, VkFormat format, VkImageLayout prev_layout, VkImageLayout next_layout);

    void create_shader_module(VkDevice device, VkShaderModule& shader_module, std::vector<uint8_t> shader_bytes);
    void create_shader_module(VkDevice device, VkShaderModule& shader_module, void* p_code, size_t code_size);
    void create_shader_module(VkDevice device, VkShaderModule& shader_module, Shader* p_shader);

    void record_blit(Instance* p_inst, VkCommandBuffer cmd_buff, VkImage src, VkImage dst, VkExtent2D src_size, VkExtent2D dst_size);

    void record_buff_barrier(Instance* p_inst, VkDevice device, VkCommandBuffer cmd_buff, VkBuffer buff, VkDeviceSize offset, VkDeviceSize size);

    VkTransformMatrixKHR convert_mat4(const zp::math::mat4& mat);

    void record_copy_buff_to_image(VkCommandBuffer cmd_buff, VkBuffer buff, VkDeviceSize offset, VkImage dst, uint32_t width, uint32_t height, uint32_t num_channels, size_t pixel_size, uint32_t mip_levels);
}

// ====================================================================================================================
// ====================================================================================================================
// ====================================================================================================================
// ====================================================================================================================
namespace zp::gpu::shortcuts
{
    void standard_desc_set_layout(VkDevice device, std::vector<std::tuple<VkDescriptorType, uint32_t, VkShaderStageFlags>> bind_infos, VkDescriptorSetLayout* p_set_layout);

    void standard_setup_desc_set(
        VkDevice device, VkDescriptorSetLayout desc_set_layout, VkDescriptorPool desc_pool, VkDescriptorSet* p_desc_set, std::vector<std::tuple<VkDescriptorType, std::vector<VkDescriptorBufferInfo>, std::vector<VkDescriptorImageInfo>, std::vector<VkWriteDescriptorSetAccelerationStructureKHR>>> infos
    );

    void standard_compute_pipeline(VkDevice device, VkShaderModule shader_module, std::vector<size_t> push_constant_configs, VkDescriptorSetLayout* p_set_layout, VkPipelineLayout* p_pipeline_layout, VkPipeline* p_pipeline);
}

// ====================================================================================================================
// ====================================================================================================================
// ====================================================================================================================
// ====================================================================================================================
namespace zp::gpu::passes::skin_work
{
    struct DispatchSkinVerts
    {
        zp::math::vec3 skel_space_mesh_origin;
        uint32_t start_rig_idx;
        uint32_t start_vert_idx;
        uint32_t start_skinned_vert_idx;
        uint32_t vert_count;
        uint32_t start_joint_mat_idx;
    };

    struct DispatchSkinNrms
    {
        uint32_t start_rig_idx;
        uint32_t start_per_tris_idx;
        uint32_t start_skinned_per_tris_idx;
        uint32_t tri_count;
        uint32_t start_joint_mat_idx;
        uint32_t start_idx_idx;
        float _pad1;
        float _pad2;
    };

    struct Instance
    {
        struct Setup
        {
            uint32_t max_dispatches;

            VkDevice device;
            VkPhysicalDevice phys_dev;
            VkDescriptorPool desc_pool;

            zp::gpu::DeviceBuff4* p_verts_buff;
            zp::gpu::DeviceBuff4* p_skinned_verts_buff;
            zp::gpu::DeviceBuff4* p_joint_mats_buff;
            zp::gpu::DeviceBuff4* p_rigs_buff;
            zp::gpu::DeviceBuff4* p_idcs_buff;
            zp::gpu::DeviceBuff4* p_per_tris_buff;
            zp::gpu::DeviceBuff4* p_skinned_per_tris_buff;
        };
        Setup setup;

        struct State
        {
            zp::gpu::DeviceBuff4 dispatch_skins_verts_buff;
            zp::gpu::DeviceBuff4 dispatch_skins_nrms_buff;

            VkDescriptorSetLayout desc_set_layout_verts;
            VkPipelineLayout pipeline_layout_verts;
            VkShaderModule shader_verts;
            VkPipeline pipeline_verts;
            VkDescriptorSet desc_set_verts;

            VkDescriptorSetLayout desc_set_layout_nrms;
            VkPipelineLayout pipeline_layout_nrms;
            VkShaderModule shader_nrms;
            VkPipeline pipeline_nrms;
            VkDescriptorSet desc_set_nrms;
        };
        State state;

        struct Shared
        {
            std::vector<DispatchSkinVerts> verts_dispatches;
            std::vector<DispatchSkinNrms> nrms_dispatches;
        };
        Shared shared;

        void record_init(VkCommandBuffer cmd_buff);
        void update_buffs();
        void record_cmd_buff(VkCommandBuffer cmd_buff);
    };
}

// ====================================================================================================================
// ====================================================================================================================
// note
// staging buffer max image size is 16k x 16k R8G8B8A8
// ====================================================================================================================
// ====================================================================================================================
namespace zp::gpu::passes::tex_work
{
    constexpr uint32_t MAX_WIDTH  = 4096;
    constexpr uint32_t MAX_HEIGHT = 4096;
    constexpr uint32_t MAX_SIZE   = MAX_WIDTH * MAX_HEIGHT * 4;

    struct Instance
    {
        struct Setup
        {
            zp::gpu::Instance* p_inst;
            VkDevice device;
            VkPhysicalDevice phys_dev;
        };

        struct State
        {
            struct StagedUpload
            {
                RegionHandle region;
                DeviceLocalImage* p_img;
            };
            std::vector<StagedUpload> staged_uploads;
            HostBuff2 staging_buff;
        };

        struct Shared
        {
            struct RequestUpload
            {
                std::vector<std::byte>* p_bytes;
                DeviceLocalImage* p_img;
            };
            std::deque<RequestUpload> requests;
        };

        Setup setup;
        State state;
        Shared shared;

        void record_init(VkCommandBuffer cmd_buff);
        void update_buffs();
        void record_cmd_buff(VkCommandBuffer cmd_buff);
    };
}

// ====================================================================================================================
// ====================================================================================================================
// ====================================================================================================================
// ====================================================================================================================
namespace zp::gpu::passes::as_work
{
    struct RequestedRebuildInfo
    {
        zp::gpu::BlasStore::BlasId blas_id;
        VkDeviceAddress verts_buff_addr;
        std::vector<zp::gpu::RegionHandle> verts_regions;
        std::vector<zp::gpu::RegionHandle> idcs_regions;
    };

    struct Instance
    {
        struct Setup
        {
            zp::gpu::Instance* p_inst;
            VkDevice vk_dev;
            VkPhysicalDevice vk_phys_dev;
            BlasStore* p_blas_store;
            DeviceBuff4* p_idcs_buff;
            Tlas* p_tlas;
        };

        struct Shared
        {
            std::vector<RequestedRebuildInfo> requested_rebuild_infos;
        };

        Setup setup;
        Shared shared;

        void record_init(VkCommandBuffer cmd_buff);
        void record_cmd_buff(VkCommandBuffer cmd_buff);
    };
}

// ====================================================================================================================
// ====================================================================================================================
// ====================================================================================================================
// ====================================================================================================================
namespace zp::gpu::passes::rt_pass
{
    struct UboData
    {
        zp::math::mat4 model;
        VkDeviceAddress p_per_dsm_buff;
        float near_plane_width;
        float near_plane_height;
        float focus_dist;
        float _pad[3];
    };

    constexpr uint32_t MAX_INSTS       = 1000;
    constexpr uint32_t MAX_RGEN_GROUPS = 10;
    constexpr uint32_t MAX_MISS_GROUPS = 10;
    constexpr uint32_t MAX_HIT_GROUPS  = 1000;

    struct Instance
    {
        struct Setup
        {
            zp::gpu::Instance* p_inst;
            VkDevice device;
            VkPhysicalDevice phys_dev;
            VkDescriptorPool desc_pool;

            Tlas* p_tlas;
            DeviceLocalImageArray* p_textures;
            Sampler* p_sampler;
        };

        struct Shared
        {
            UboData ubo_data;

            zp::uuid::uuid rgen_group;
            zp::uuid::uuid miss_group;
            std::vector<zp::uuid::uuid> hit_groups;
        };

        struct Internal;

        Setup setup;
        Shared shared;
        Internal* p_i;

        void init();
        void update_pipelines();
        void update_descriptors(DeviceLocalImage* p_target);
        void update_buffs();
        void record_cmd_buff(VkCommandBuffer cmd_buff, DeviceLocalImage* p_target);
        void cleanup();
    };
}