#include "zp_cpp/gpu.hpp"

// ====================================================================================================================
// ====================================================================================================================
// ====================================================================================================================
// ====================================================================================================================
void zp::gpu::Tlas::init(zp::gpu::Instance* p_inst, VkDevice device, VkPhysicalDevice phys_dev, uint32_t MAX_INSTS)
{
    this->MAX_INSTS = MAX_INSTS;

    // ========================================================================================
    // ========================================================================================
    // init instance buffer used to rebuild tlases
    // ========================================================================================
    // ========================================================================================
    {
        insts_buff.init(device, phys_dev, sizeof(VkAccelerationStructureInstanceKHR), MAX_INSTS, VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT | VK_BUFFER_USAGE_ACCELERATION_STRUCTURE_BUILD_INPUT_READ_ONLY_BIT_KHR);
    }

    // ========================================================================================
    // ========================================================================================
    // get build size info
    // note
    // using max size for convenience, no future re-creates needed
    // ========================================================================================
    // ========================================================================================
    VkAccelerationStructureBuildSizesInfoKHR build_sizes_info = {};
    {
        build_sizes_info.sType                                      = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_BUILD_SIZES_INFO_KHR;

        VkAccelerationStructureGeometryInstancesDataKHR insts_data  = {};
        insts_data.sType                                            = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_GEOMETRY_INSTANCES_DATA_KHR;
        insts_data.arrayOfPointers                                  = VK_FALSE;
        insts_data.data.deviceAddress                               = insts_buff.deviceAddress;

        VkAccelerationStructureGeometryKHR as_geom                  = {};
        as_geom.sType                                               = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_GEOMETRY_KHR;
        as_geom.geometryType                                        = VK_GEOMETRY_TYPE_INSTANCES_KHR;
        as_geom.flags                                               = VK_GEOMETRY_OPAQUE_BIT_KHR;
        as_geom.geometry.instances                                  = insts_data;

        VkAccelerationStructureBuildGeometryInfoKHR build_geom_info = {};
        build_geom_info.sType                                       = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_BUILD_GEOMETRY_INFO_KHR;
        build_geom_info.type                                        = VK_ACCELERATION_STRUCTURE_TYPE_TOP_LEVEL_KHR;
        build_geom_info.flags                                       = VK_BUILD_ACCELERATION_STRUCTURE_PREFER_FAST_TRACE_BIT_KHR;
        build_geom_info.geometryCount                               = 1;
        build_geom_info.pGeometries                                 = &as_geom;

        uint32_t prim_count                                         = MAX_INSTS;

        p_inst->func_ptrs.vkGetAccelerationStructureBuildSizesKHR(device, VK_ACCELERATION_STRUCTURE_BUILD_TYPE_DEVICE_KHR, &build_geom_info, &prim_count, &build_sizes_info);
    }

    // ============================================================================
    // ============================================================================
    // create tlas buff and as
    // ============================================================================
    // ============================================================================
    {
        util::create_buff(device, phys_dev, build_sizes_info.accelerationStructureSize, VK_BUFFER_USAGE_ACCELERATION_STRUCTURE_STORAGE_BIT_KHR | VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, buffer, memory);

        VkAccelerationStructureCreateInfoKHR info = {};
        info.sType                                = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_CREATE_INFO_KHR;
        info.buffer                               = buffer;
        info.size                                 = build_sizes_info.accelerationStructureSize;
        info.type                                 = VK_ACCELERATION_STRUCTURE_TYPE_TOP_LEVEL_KHR;

        p_inst->func_ptrs.vkCreateAccelerationStructureKHR(device, &info, nullptr, &handle);
    }

    // ============================================================================
    // ============================================================================
    // init scratch buff
    // ============================================================================
    // ============================================================================
    scratch_buff.init(device, phys_dev, build_sizes_info.buildScratchSize, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT);
}

// ====================================================================================================================
// ====================================================================================================================
// ====================================================================================================================
// ====================================================================================================================
void zp::gpu::Tlas::record_build(zp::gpu::Instance* p_inst, VkCommandBuffer cmd_buff)
{
    VkAccelerationStructureGeometryInstancesDataKHR data                 = {};
    data.sType                                                           = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_GEOMETRY_INSTANCES_DATA_KHR;
    data.arrayOfPointers                                                 = VK_FALSE;
    data.data.deviceAddress                                              = insts_buff.deviceAddress;

    VkAccelerationStructureGeometryKHR geom                              = {};
    geom.sType                                                           = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_GEOMETRY_KHR;
    geom.geometryType                                                    = VK_GEOMETRY_TYPE_INSTANCES_KHR;
    geom.flags                                                           = VK_GEOMETRY_OPAQUE_BIT_KHR;
    geom.geometry.instances                                              = data;

    VkAccelerationStructureBuildGeometryInfoKHR build_info               = {};
    build_info.sType                                                     = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_BUILD_GEOMETRY_INFO_KHR;
    build_info.type                                                      = VK_ACCELERATION_STRUCTURE_TYPE_TOP_LEVEL_KHR;
    build_info.flags                                                     = VK_BUILD_ACCELERATION_STRUCTURE_PREFER_FAST_TRACE_BIT_KHR;
    build_info.mode                                                      = VK_BUILD_ACCELERATION_STRUCTURE_MODE_BUILD_KHR;
    build_info.dstAccelerationStructure                                  = handle;
    build_info.geometryCount                                             = 1;
    build_info.pGeometries                                               = &geom;
    build_info.scratchData.deviceAddress                                 = scratch_buff.device_addr;

    VkAccelerationStructureBuildRangeInfoKHR range_info                  = {};
    range_info.primitiveCount                                            = insts_buff.count;
    range_info.primitiveOffset                                           = 0;
    range_info.firstVertex                                               = 0;
    range_info.transformOffset                                           = 0;

    std::vector<VkAccelerationStructureBuildRangeInfoKHR*> p_range_infos = {&range_info};

    p_inst->func_ptrs.vkCmdBuildAccelerationStructuresKHR(cmd_buff, 1, &build_info, p_range_infos.data());
}

// ====================================================================================================================
// ====================================================================================================================
// ====================================================================================================================
// ====================================================================================================================
VkWriteDescriptorSetAccelerationStructureKHR zp::gpu::Tlas::desc_info()
{
    VkWriteDescriptorSetAccelerationStructureKHR info = {};
    info.sType                                        = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET_ACCELERATION_STRUCTURE_KHR;
    info.accelerationStructureCount                   = 1;
    info.pAccelerationStructures                      = &handle;

    return info;
}
