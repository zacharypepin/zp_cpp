#include "zp_cpp/gpu.hpp"

using namespace zp::gpu;
using namespace zp::gpu::passes;
using namespace zp::gpu::passes::rt_pass;

// ========================================================================================================================================
// ========================================================================================================================================
// ========================================================================================================================================
// ========================================================================================================================================
struct rt_pass::Instance::Internal
{
    uint32_t HANDLE_SIZE_ALIGNED;

    uint32_t RGEN_SECTION_SIZE;
    uint32_t MISS_SECTION_SIZE;
    uint32_t HITS_SECTION_SIZE;

    DeviceBuff4 ubo;
    DeviceBuff4 sbt_buff;

    VkDescriptorSetLayout desc_set_layout;
    VkPipelineLayout pipeline_layout;
    VkPipeline pipeline;
    VkDescriptorSet desc_set;

    std::vector<uint8_t> group_handles_storage;
    std::unordered_map<zp::uuid::uuid, std::byte*> group_handles;

    std::byte* p_sbt_rgen_region;
    std::byte* p_sbt_miss_region;
    std::byte* p_sbt_hits_region;

    VkStridedDeviceAddressRegionKHR rgen_entry;
    VkStridedDeviceAddressRegionKHR rmiss_entry;
    VkStridedDeviceAddressRegionKHR chit_entry;
    VkStridedDeviceAddressRegionKHR callable_entry;

    bool is_pipelines_init;
    bool is_descriptors_init;
};

// ====================================================================================================================
// ====================================================================================================================
// ====================================================================================================================
// ====================================================================================================================
void zp::gpu::passes::rt_pass::Instance::init()
{
    // ============================================================================================
    // ============================================================================================
    // ============================================================================================
    // ============================================================================================
    p_i = new Internal();

    // ============================================================================================
    // ============================================================================================
    // get phys dev props 2
    // ============================================================================================
    // ============================================================================================
    {
        VkPhysicalDeviceRayTracingPipelinePropertiesKHR rt_props = {};
        {
            rt_props.sType                    = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_RAY_TRACING_PIPELINE_PROPERTIES_KHR;

            VkPhysicalDeviceProperties2 props = {};
            props.sType                       = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PROPERTIES_2;
            props.pNext                       = &rt_props;

            vkGetPhysicalDeviceProperties2(setup.phys_dev, &props);
        }

        p_i->HANDLE_SIZE_ALIGNED = util::aligned_size(rt_props.shaderGroupHandleSize, rt_props.shaderGroupHandleAlignment);

        p_i->RGEN_SECTION_SIZE   = util::aligned_size((p_i->HANDLE_SIZE_ALIGNED * MAX_RGEN_GROUPS), rt_props.shaderGroupBaseAlignment);
        p_i->MISS_SECTION_SIZE   = util::aligned_size((p_i->HANDLE_SIZE_ALIGNED * MAX_MISS_GROUPS), rt_props.shaderGroupBaseAlignment);
        p_i->HITS_SECTION_SIZE   = util::aligned_size((p_i->HANDLE_SIZE_ALIGNED * MAX_HIT_GROUPS), rt_props.shaderGroupBaseAlignment);
    }

    // ============================================================================================
    // ============================================================================================
    // ubo
    // ============================================================================================
    // ============================================================================================
    {
        p_i->ubo.init(setup.device, setup.phys_dev, sizeof(UboData), 1, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT);
    }

    // ============================================================================================
    // ============================================================================================
    // desc set layout
    // ============================================================================================
    // ============================================================================================
    {
        // clang-format off
        shortcuts::standard_desc_set_layout(
            setup.device,
            {
				{			 VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 						  1, VK_SHADER_STAGE_ANY_HIT_BIT_KHR | VK_SHADER_STAGE_INTERSECTION_BIT_KHR | VK_SHADER_STAGE_RAYGEN_BIT_KHR | VK_SHADER_STAGE_CLOSEST_HIT_BIT_KHR},
				{             VK_DESCRIPTOR_TYPE_STORAGE_IMAGE,                           1,                                               								 								    VK_SHADER_STAGE_RAYGEN_BIT_KHR},
				{VK_DESCRIPTOR_TYPE_ACCELERATION_STRUCTURE_KHR,                           1,                                             								 								    VK_SHADER_STAGE_RAYGEN_BIT_KHR},
				{                   VK_DESCRIPTOR_TYPE_SAMPLER,                           1,									    VK_SHADER_STAGE_RAYGEN_BIT_KHR | VK_SHADER_STAGE_CLOSEST_HIT_BIT_KHR | VK_SHADER_STAGE_ANY_HIT_BIT_KHR},
				{             VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, setup.p_textures->MAX_COUNT,									    VK_SHADER_STAGE_RAYGEN_BIT_KHR | VK_SHADER_STAGE_CLOSEST_HIT_BIT_KHR | VK_SHADER_STAGE_ANY_HIT_BIT_KHR},
            },
            &p_i->desc_set_layout
        );
        // clang-format on
    }

    // ========================================================================================
    // ========================================================================================
    // pipeline layout
    // ========================================================================================
    // ========================================================================================
    {
        VkPipelineLayoutCreateInfo info = {};
        info.sType                      = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
        info.setLayoutCount             = 1;
        info.pSetLayouts                = &p_i->desc_set_layout;

        VK_CHECK(vkCreatePipelineLayout(setup.device, &info, nullptr, &p_i->pipeline_layout), "creating pipeline layout");
    }

    // ============================================================================================
    // ============================================================================================
    // sbt
    // ============================================================================================
    // ============================================================================================
    {
        p_i->sbt_buff.init(setup.device, setup.phys_dev, sizeof(std::byte), p_i->RGEN_SECTION_SIZE + p_i->MISS_SECTION_SIZE + p_i->HITS_SECTION_SIZE, VK_BUFFER_USAGE_SHADER_BINDING_TABLE_BIT_KHR | VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT);
    }
}

// ====================================================================================================================
// ====================================================================================================================
// ====================================================================================================================
// ====================================================================================================================
void zp::gpu::passes::rt_pass::Instance::update_pipelines()
{
    // ============================================================================================
    // ============================================================================================
    // ============================================================================================
    // ============================================================================================
    if (p_i->is_pipelines_init)
    {
        vkDestroyPipeline(setup.device, p_i->pipeline, nullptr);
    }

    // ========================================================================================
    // ========================================================================================
    // create shader modules
    // ========================================================================================
    // ========================================================================================
    std::vector<VkShaderModule> modules;
    std::unordered_map<zp::uuid::uuid, std::tuple<int, int, int, int, int>> shader_group_to_idcs;
    {
        for (auto&& [uuid, sg] : setup.p_inst->shader_groups)
        {
            bool is_rgen_group = sg.rgen.p_data != nullptr;
            bool is_miss_group = sg.miss.p_data != nullptr;
            bool has_chit      = sg.chit.p_data != nullptr;
            bool has_ahit      = sg.ahit.p_data != nullptr;
            bool has_intr      = sg.intr.p_data != nullptr;

            VkShaderModule sm;

            int rgen_idx = -1;
            int miss_idx = -1;
            int chit_idx = -1;
            int ahit_idx = -1;
            int intr_idx = -1;

            if (is_rgen_group)
            {
                zp::gpu::util::create_shader_module(setup.device, sm, &sg.rgen);
                rgen_idx = modules.size();
                modules.push_back(sm);
            }

            if (is_miss_group)
            {
                zp::gpu::util::create_shader_module(setup.device, sm, &sg.miss);
                miss_idx = modules.size();
                modules.push_back(sm);
            }

            if (has_chit)
            {
                zp::gpu::util::create_shader_module(setup.device, sm, &sg.chit);
                chit_idx = modules.size();
                modules.push_back(sm);
            }

            if (has_ahit)
            {
                zp::gpu::util::create_shader_module(setup.device, sm, &sg.ahit);
                ahit_idx = modules.size();
                modules.push_back(sm);
            }

            if (has_intr)
            {
                zp::gpu::util::create_shader_module(setup.device, sm, &sg.intr);
                intr_idx = modules.size();
                modules.push_back(sm);
            }

            shader_group_to_idcs.insert({
                sg.uuid,
                {rgen_idx, miss_idx, chit_idx, ahit_idx, intr_idx}
            });
        }
    }

    // ========================================================================================
    // ========================================================================================
    // create pipeline
    // ========================================================================================
    // ========================================================================================
    {
        std::vector<VkPipelineShaderStageCreateInfo> stage_infos = {};
        {
            for (auto&& [uuid, sg] : setup.p_inst->shader_groups)
            {
                auto [rgen_idx, miss_idx, chit_idx, ahit_idx, intr_idx] = shader_group_to_idcs.at(sg.uuid);

                if (rgen_idx != -1)
                {
                    VkPipelineShaderStageCreateInfo info = {};
                    info.sType                           = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
                    info.stage                           = VK_SHADER_STAGE_RAYGEN_BIT_KHR;
                    info.module                          = modules[rgen_idx];
                    info.pName                           = "main";

                    stage_infos.push_back(info);
                }

                if (miss_idx != -1)
                {
                    VkPipelineShaderStageCreateInfo info = {};
                    info.sType                           = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
                    info.stage                           = VK_SHADER_STAGE_MISS_BIT_KHR;
                    info.module                          = modules[miss_idx];
                    info.pName                           = "main";

                    stage_infos.push_back(info);
                }

                if (chit_idx != -1)
                {
                    VkPipelineShaderStageCreateInfo info = {};
                    info.sType                           = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
                    info.stage                           = VK_SHADER_STAGE_CLOSEST_HIT_BIT_KHR;
                    info.module                          = modules[chit_idx];
                    info.pName                           = "main";

                    stage_infos.push_back(info);
                }

                if (ahit_idx != -1)
                {
                    VkPipelineShaderStageCreateInfo info = {};
                    info.sType                           = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
                    info.stage                           = VK_SHADER_STAGE_ANY_HIT_BIT_KHR;
                    info.module                          = modules[ahit_idx];
                    info.pName                           = "main";

                    stage_infos.push_back(info);
                }

                if (intr_idx != -1)
                {
                    VkPipelineShaderStageCreateInfo info = {};
                    info.sType                           = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
                    info.stage                           = VK_SHADER_STAGE_INTERSECTION_BIT_KHR;
                    info.module                          = modules[intr_idx];
                    info.pName                           = "main";

                    stage_infos.push_back(info);
                }
            }
        }

        std::vector<VkRayTracingShaderGroupCreateInfoKHR> groups = {};
        {
            for (auto&& [uuid, sg] : setup.p_inst->shader_groups)
            {
                auto [rgen_idx, miss_idx, chit_idx, ahit_idx, intr_idx] = shader_group_to_idcs.at(sg.uuid);

                VkRayTracingShaderGroupTypeKHR type                     = (rgen_idx != -1 || miss_idx != -1) ? VK_RAY_TRACING_SHADER_GROUP_TYPE_GENERAL_KHR : intr_idx != -1 ? VK_RAY_TRACING_SHADER_GROUP_TYPE_PROCEDURAL_HIT_GROUP_KHR : VK_RAY_TRACING_SHADER_GROUP_TYPE_TRIANGLES_HIT_GROUP_KHR;

                const VkRayTracingShaderGroupCreateInfoKHR group        = {
                           .sType              = VK_STRUCTURE_TYPE_RAY_TRACING_SHADER_GROUP_CREATE_INFO_KHR,
                           .type               = type,
                           .generalShader      = rgen_idx != -1 ? rgen_idx
                                               : miss_idx != -1 ? miss_idx
                                                                : VK_SHADER_UNUSED_KHR,
                           .closestHitShader   = chit_idx != -1 ? chit_idx : VK_SHADER_UNUSED_KHR,
                           .anyHitShader       = ahit_idx != -1 ? ahit_idx : VK_SHADER_UNUSED_KHR,
                           .intersectionShader = intr_idx != -1 ? intr_idx : VK_SHADER_UNUSED_KHR,
                };

                groups.push_back(group);
            }
        }

        VkRayTracingPipelineCreateInfoKHR info = {};
        info.sType                             = VK_STRUCTURE_TYPE_RAY_TRACING_PIPELINE_CREATE_INFO_KHR;
        info.stageCount                        = stage_infos.size();
        info.pStages                           = stage_infos.data();
        info.groupCount                        = groups.size();
        info.pGroups                           = groups.data();
        info.maxPipelineRayRecursionDepth      = 1;
        info.layout                            = p_i->pipeline_layout;

        VK_CHECK(setup.p_inst->func_ptrs.vkCreateRayTracingPipelinesKHR(setup.device, VK_NULL_HANDLE, VK_NULL_HANDLE, 1, &info, nullptr, &p_i->pipeline), "creating raytracing pipeline");
    }

    // ============================================================================================
    // ============================================================================================
    // setup sbt
    // ============================================================================================
    // ============================================================================================
    {
        // ====================================================================
        // ====================================================================
        // get handles
        // ====================================================================
        // ====================================================================
        {
            p_i->group_handles_storage.resize(p_i->HANDLE_SIZE_ALIGNED * setup.p_inst->shader_groups.size());

            setup.p_inst->func_ptrs.vkGetRayTracingShaderGroupHandlesKHR(setup.device, p_i->pipeline, 0, setup.p_inst->shader_groups.size(), p_i->group_handles_storage.size(), p_i->group_handles_storage.data());
        }

        // ====================================================================
        // ====================================================================
        // get ptrs
        // ====================================================================
        // ====================================================================
        {
            p_i->group_handles.clear();

            for (int idx = 0; auto&& [uuid, sg] : setup.p_inst->shader_groups)
            {
                p_i->group_handles.insert({sg.uuid, (std::byte*)p_i->group_handles_storage.data() + idx * p_i->HANDLE_SIZE_ALIGNED});

                idx++;
            }
        }

        // ====================================================================
        // ====================================================================
        // get mapped regions
        // ====================================================================
        // ====================================================================
        {
            p_i->p_sbt_rgen_region = p_i->sbt_buff.p_mapped;
            p_i->p_sbt_miss_region = p_i->p_sbt_rgen_region + p_i->RGEN_SECTION_SIZE;
            p_i->p_sbt_hits_region = p_i->p_sbt_miss_region + p_i->MISS_SECTION_SIZE;
        }

        // ====================================================================
        // ====================================================================
        // get sbtaddr regions
        // ====================================================================
        // ====================================================================
        {
            auto rgen_addr                                 = p_i->sbt_buff.deviceAddress;
            auto rmiss_addr                                = rgen_addr + p_i->RGEN_SECTION_SIZE;
            auto chit_addr                                 = rmiss_addr + p_i->MISS_SECTION_SIZE;

            VkStridedDeviceAddressRegionKHR rgen_entry     = {};
            rgen_entry.deviceAddress                       = rgen_addr;
            rgen_entry.stride                              = p_i->RGEN_SECTION_SIZE;
            rgen_entry.size                                = p_i->RGEN_SECTION_SIZE;

            VkStridedDeviceAddressRegionKHR rmiss_entry    = {};
            rmiss_entry.deviceAddress                      = rmiss_addr;
            rmiss_entry.stride                             = p_i->HANDLE_SIZE_ALIGNED;
            rmiss_entry.size                               = p_i->MISS_SECTION_SIZE;

            VkStridedDeviceAddressRegionKHR chit_entry     = {};
            chit_entry.deviceAddress                       = chit_addr;
            chit_entry.stride                              = p_i->HANDLE_SIZE_ALIGNED;
            chit_entry.size                                = p_i->HITS_SECTION_SIZE;

            VkStridedDeviceAddressRegionKHR callable_entry = {};

            p_i->rgen_entry                                = rgen_entry;
            p_i->rmiss_entry                               = rmiss_entry;
            p_i->chit_entry                                = chit_entry;
            p_i->callable_entry                            = callable_entry;
        }
    }

    // ============================================================================================
    // ============================================================================================
    // destroy shader modules
    // ============================================================================================
    // ============================================================================================
    {
        for (auto&& i : modules) vkDestroyShaderModule(setup.device, i, nullptr);
    }

    // ============================================================================================
    // ============================================================================================
    // ============================================================================================
    // ============================================================================================
    p_i->is_pipelines_init = true;
}

// ====================================================================================================================
// ====================================================================================================================
// ====================================================================================================================
// ====================================================================================================================
void zp::gpu::passes::rt_pass::Instance::update_descriptors(DeviceLocalImage* p_target)
{
    // ============================================================================================
    // ============================================================================================
    // ============================================================================================
    // ============================================================================================
    if (p_i->is_descriptors_init)
    {
        vkFreeDescriptorSets(setup.device, setup.desc_pool, 1, &p_i->desc_set);
    }

    // ============================================================================================
    // ============================================================================================
    // ============================================================================================
    // ============================================================================================
    {
        shortcuts::standard_setup_desc_set(
            setup.device,
            p_i->desc_set_layout,
            setup.desc_pool,
            &p_i->desc_set,
            {
                {            VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, {p_i->ubo.desc_info()},                                                                     {},                          {}},
                {             VK_DESCRIPTOR_TYPE_STORAGE_IMAGE,                     {},                         {p_target->desc_info(VK_IMAGE_LAYOUT_GENERAL)},                          {}},
                {VK_DESCRIPTOR_TYPE_ACCELERATION_STRUCTURE_KHR,                     {},                                                                     {}, {setup.p_tlas->desc_info()}},
                {                   VK_DESCRIPTOR_TYPE_SAMPLER,                     {},                                         {setup.p_sampler->desc_info()},                          {}},
                {             VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE,                     {}, setup.p_textures->desc_infos(VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL),                          {}},
        }
        );
    }

    // ============================================================================================
    // ============================================================================================
    // ============================================================================================
    // ============================================================================================
    p_i->is_descriptors_init = true;
}

// ====================================================================================================================
// ====================================================================================================================
// ====================================================================================================================
// ====================================================================================================================
void zp::gpu::passes::rt_pass::Instance::update_buffs()
{
    // ============================================================================================
    // ============================================================================================
    // update ubo
    // ============================================================================================
    // ============================================================================================
    {
        memcpy(p_i->ubo.p_mapped, &shared.ubo_data, sizeof(UboData));
    }

    // ============================================================================================
    // ============================================================================================
    // update sbt
    // ============================================================================================
    // ============================================================================================
    {
        memcpy(p_i->p_sbt_rgen_region, p_i->group_handles.at(shared.rgen_group), p_i->HANDLE_SIZE_ALIGNED);
        memcpy(p_i->p_sbt_miss_region, p_i->group_handles.at(shared.miss_group), p_i->HANDLE_SIZE_ALIGNED);

        for (int idx = 0; auto&& uuid : shared.hit_groups)
        {
            memcpy(p_i->p_sbt_hits_region + (idx * p_i->HANDLE_SIZE_ALIGNED), p_i->group_handles.at(uuid), p_i->HANDLE_SIZE_ALIGNED);
            idx++;
        }
    }
}

// ====================================================================================================================
// ====================================================================================================================
// ====================================================================================================================
// ====================================================================================================================
void zp::gpu::passes::rt_pass::Instance::record_cmd_buff(VkCommandBuffer cmd_buff, DeviceLocalImage* p_target)
{
    update_descriptors(p_target);

    if (!p_i->is_pipelines_init || !p_i->is_descriptors_init)
    {
        CLOG(p_i->is_pipelines_init);
        CLOG(p_i->is_descriptors_init);
        ERR("not fully initialised!");
    }

    vkCmdBindPipeline(cmd_buff, VK_PIPELINE_BIND_POINT_RAY_TRACING_KHR, p_i->pipeline);
    vkCmdBindDescriptorSets(cmd_buff, VK_PIPELINE_BIND_POINT_RAY_TRACING_KHR, p_i->pipeline_layout, 0, 1, &p_i->desc_set, 0, 0);

    setup.p_inst->func_ptrs.vkCmdTraceRaysKHR(cmd_buff, &p_i->rgen_entry, &p_i->rmiss_entry, &p_i->chit_entry, &p_i->callable_entry, p_target->width, p_target->height, 1);
}

// ====================================================================================================================
// ====================================================================================================================
// ====================================================================================================================
// ====================================================================================================================
void zp::gpu::passes::rt_pass::Instance::cleanup()
{
    vkDestroyPipeline(setup.device, p_i->pipeline, nullptr);
    vkDestroyPipelineLayout(setup.device, p_i->pipeline_layout, nullptr);
    vkFreeDescriptorSets(setup.device, setup.desc_pool, 1, &p_i->desc_set);
    vkDestroyDescriptorSetLayout(setup.device, p_i->desc_set_layout, nullptr);
    p_i->sbt_buff.cleanup(setup.device);
    p_i->ubo.cleanup(setup.device);

    delete p_i;
}
