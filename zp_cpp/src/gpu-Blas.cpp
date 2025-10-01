#include "zp_cpp/gpu.hpp"

// ====================================================================================================================
// ====================================================================================================================
// not a full build, unlike setup for proc sphere blas
// just 1. calculates size 2. initialises resources and variables
// useful to do this early to get stable device addresses
// ====================================================================================================================
// ====================================================================================================================
void zp::gpu::Blas::init_tri_blas(zp::gpu::Instance* p_inst, VkDevice device, VkPhysicalDevice phys_dev, std::vector<uint32_t> submesh_tri_counts)
{
    this->p_inst = p_inst;
    this->device = device;

    // ========================================================================================
    // ========================================================================================
    // compose as geometries
    // ========================================================================================
    // ========================================================================================
    std::vector<VkAccelerationStructureGeometryKHR> as_geoms(submesh_tri_counts.size());
    {
        VkAccelerationStructureGeometryTrianglesDataKHR tris = {};
        tris.sType                                           = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_GEOMETRY_TRIANGLES_DATA_KHR;
        tris.vertexFormat                                    = VK_FORMAT_R32G32B32_SFLOAT;
        tris.vertexStride                                    = sizeof(zp::math::vec3);
        tris.indexType                                       = VK_INDEX_TYPE_UINT32;

        VkAccelerationStructureGeometryKHR geom              = {};
        geom.sType                                           = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_GEOMETRY_KHR;
        geom.flags                                           = VK_GEOMETRY_NO_DUPLICATE_ANY_HIT_INVOCATION_BIT_KHR;
        geom.geometryType                                    = VK_GEOMETRY_TYPE_TRIANGLES_KHR;
        geom.geometry.triangles                              = tris;

        for (auto&& elem : as_geoms)
        {
            elem = geom;
        }
    }

    // ========================================================================================
    // ========================================================================================
    // get build sizes info
    // ========================================================================================
    // ========================================================================================
    VkAccelerationStructureBuildSizesInfoKHR sizes_info = {};
    {
        sizes_info.sType                                       = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_BUILD_SIZES_INFO_KHR;

        VkAccelerationStructureBuildGeometryInfoKHR geom_info  = {};
        geom_info.sType                                        = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_BUILD_GEOMETRY_INFO_KHR;
        geom_info.type                                         = VK_ACCELERATION_STRUCTURE_TYPE_BOTTOM_LEVEL_KHR;
        geom_info.flags                                        = VK_BUILD_ACCELERATION_STRUCTURE_PREFER_FAST_TRACE_BIT_KHR;
        geom_info.flags                                       |= VK_BUILD_ACCELERATION_STRUCTURE_ALLOW_DATA_ACCESS_KHR;
        geom_info.geometryCount                                = as_geoms.size();
        geom_info.pGeometries                                  = as_geoms.data();

        p_inst->func_ptrs.vkGetAccelerationStructureBuildSizesKHR(device, VK_ACCELERATION_STRUCTURE_BUILD_TYPE_DEVICE_KHR, &geom_info, submesh_tri_counts.data(), &sizes_info);
    }

    // ========================================================================================
    // ========================================================================================
    // create blas buffer
    // ========================================================================================
    // ========================================================================================
    {
        util::create_buff(device, phys_dev, sizes_info.accelerationStructureSize, VK_BUFFER_USAGE_ACCELERATION_STRUCTURE_STORAGE_BIT_KHR | VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, buffer, memory);
    }

    // ========================================================================================
    // ========================================================================================
    // create acceleration structure
    // ========================================================================================
    // ========================================================================================
    {
        VkAccelerationStructureCreateInfoKHR info = {};
        info.sType                                = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_CREATE_INFO_KHR;
        info.buffer                               = buffer;
        info.size                                 = sizes_info.accelerationStructureSize;
        info.type                                 = VK_ACCELERATION_STRUCTURE_TYPE_BOTTOM_LEVEL_KHR;

        p_inst->func_ptrs.vkCreateAccelerationStructureKHR(device, &info, nullptr, &handle);
    }

    // ========================================================================================
    // ========================================================================================
    // create its scratch buffer
    // ========================================================================================
    // ========================================================================================
    {
        scratch_buff.init(device, phys_dev, sizes_info.buildScratchSize, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT);
    }

    // ========================================================================================
    // ========================================================================================
    // store blas device address so tlas can reference it
    // ========================================================================================
    // ========================================================================================
    {
        deviceAddress = util::get_as_dev_addr(p_inst, device, handle);
    }

    // ========================================================================================
    // ========================================================================================
    // store how many tris this blas was initialised for
    // ========================================================================================
    // ========================================================================================
    {
        int count = 0;
        for (auto&& i : submesh_tri_counts) count += i;
        this->num_tris_initialised = count;
    }
}

// ====================================================================================================================
// ====================================================================================================================
// ====================================================================================================================
// ====================================================================================================================
void zp::gpu::Blas::record_build_tri_blas(
    zp::gpu::Instance* p_inst, VkDevice vk_dev, VkPhysicalDevice vk_phys_dev, VkCommandBuffer cmd_buff, VkDeviceAddress verts_buff_addr, VkDeviceAddress idcs_buff_addr, uint32_t* p_mapped_idcs, std::vector<RegionHandle> submesh_verts_regions, std::vector<RegionHandle> submesh_idcs_regions
)
{
    // ========================================================================================
    // ========================================================================================
    // compose as geometries
    // ========================================================================================
    // ========================================================================================
    std::vector<VkAccelerationStructureGeometryKHR> asGeometries = {};
    {
        for (auto&& region : submesh_idcs_regions)
        {
            VkAccelerationStructureGeometryTrianglesDataKHR tris = {};
            tris.sType                                           = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_GEOMETRY_TRIANGLES_DATA_KHR;
            tris.vertexFormat                                    = VK_FORMAT_R32G32B32_SFLOAT;
            tris.vertexData.deviceAddress                        = verts_buff_addr;
            tris.maxVertex                                       = p_mapped_idcs[region.start_idx + region.count - 1];
            tris.vertexStride                                    = sizeof(zp::math::vec3);
            tris.indexType                                       = VK_INDEX_TYPE_UINT32;
            tris.indexData.deviceAddress                         = idcs_buff_addr;
            tris.transformData.deviceAddress                     = 0;
            tris.transformData.hostAddress                       = nullptr;

            VkAccelerationStructureGeometryKHR geom              = {};
            geom.sType                                           = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_GEOMETRY_KHR;
            geom.flags                                           = VK_GEOMETRY_NO_DUPLICATE_ANY_HIT_INVOCATION_BIT_KHR;
            geom.geometryType                                    = VK_GEOMETRY_TYPE_TRIANGLES_KHR;
            geom.geometry.triangles                              = tris;

            asGeometries.push_back(geom);
        }
    }

    // ========================================================================================
    // ========================================================================================
    // record build
    // ========================================================================================
    // ========================================================================================
    {
        VkAccelerationStructureBuildGeometryInfoKHR geomInfo             = {};
        geomInfo.sType                                                   = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_BUILD_GEOMETRY_INFO_KHR;
        geomInfo.type                                                    = VK_ACCELERATION_STRUCTURE_TYPE_BOTTOM_LEVEL_KHR;
        geomInfo.flags                                                   = VK_BUILD_ACCELERATION_STRUCTURE_PREFER_FAST_TRACE_BIT_KHR | VK_BUILD_ACCELERATION_STRUCTURE_ALLOW_DATA_ACCESS_KHR;
        geomInfo.mode                                                    = VK_BUILD_ACCELERATION_STRUCTURE_MODE_BUILD_KHR;
        geomInfo.dstAccelerationStructure                                = handle;
        geomInfo.geometryCount                                           = asGeometries.size();
        geomInfo.pGeometries                                             = asGeometries.data();
        geomInfo.scratchData.deviceAddress                               = scratch_buff.device_addr;

        std::vector<VkAccelerationStructureBuildRangeInfoKHR> rangeInfos = {};

        for (size_t i = 0; i < submesh_verts_regions.size(); i++)
        {
            auto& verts_region                            = submesh_verts_regions[i];
            auto& idcs_region                             = submesh_idcs_regions[i];

            VkAccelerationStructureBuildRangeInfoKHR info = {};
            info.firstVertex                              = verts_region.start_idx;
            info.primitiveOffset                          = idcs_region.start_idx * sizeof(uint32_t);
            info.primitiveCount                           = idcs_region.count / 3;
            info.transformOffset                          = 0;

            rangeInfos.push_back(info);
        }

        std::vector<VkAccelerationStructureBuildRangeInfoKHR*> pRangeInfos(rangeInfos.size());
        for (size_t i = 0; i < rangeInfos.size(); i++) pRangeInfos[i] = &rangeInfos[i];

        p_inst->func_ptrs.vkCmdBuildAccelerationStructuresKHR(cmd_buff, 1, &geomInfo, pRangeInfos.data());
    }
}

// ====================================================================================================================
// ====================================================================================================================
// ====================================================================================================================
// ====================================================================================================================
void zp::gpu::Blas::record_setup_sphere_blas(zp::gpu::Instance* p_inst, VkDevice device, VkPhysicalDevice phys_dev, VkCommandBuffer cmd_buff, VkDeviceAddress aabb_pos_device_addr)
{
    // ========================================================================================
    // ========================================================================================
    // compose VkAccelerationStructureGeometryKHR
    // ========================================================================================
    // ========================================================================================
    VkAccelerationStructureGeometryKHR as_geom;
    {

        VkAccelerationStructureGeometryAabbsDataKHR aabbs = {};
        aabbs.sType                                       = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_GEOMETRY_AABBS_DATA_KHR;
        aabbs.stride                                      = sizeof(VkAabbPositionsKHR);
        aabbs.data.deviceAddress                          = aabb_pos_device_addr;

        VkAccelerationStructureGeometryKHR geom           = {};
        geom.sType                                        = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_GEOMETRY_KHR;
        geom.flags                                        = VK_GEOMETRY_OPAQUE_BIT_KHR;
        geom.geometryType                                 = VK_GEOMETRY_TYPE_AABBS_KHR;
        geom.geometry.aabbs                               = aabbs;

        as_geom                                           = geom;
    }

    // ========================================================================================
    // ========================================================================================
    // get build sizes info
    // ========================================================================================
    // ========================================================================================
    VkAccelerationStructureBuildSizesInfoKHR sizes_info = {};
    {
        sizes_info.sType                                       = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_BUILD_SIZES_INFO_KHR;

        VkAccelerationStructureBuildGeometryInfoKHR build_info = {};
        build_info.sType                                       = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_BUILD_GEOMETRY_INFO_KHR;
        build_info.type                                        = VK_ACCELERATION_STRUCTURE_TYPE_BOTTOM_LEVEL_KHR;
        build_info.flags                                       = VK_BUILD_ACCELERATION_STRUCTURE_PREFER_FAST_TRACE_BIT_KHR;
        build_info.geometryCount                               = 1;
        build_info.pGeometries                                 = &as_geom;

        std::vector<uint32_t> prim_counts                      = {1};

        p_inst->func_ptrs.vkGetAccelerationStructureBuildSizesKHR(device, VK_ACCELERATION_STRUCTURE_BUILD_TYPE_DEVICE_KHR, &build_info, prim_counts.data(), &sizes_info);
    }

    // ========================================================================================
    // ========================================================================================
    // create blas buffer
    // ========================================================================================
    // ========================================================================================
    util::create_buff(device, phys_dev, sizes_info.accelerationStructureSize, VK_BUFFER_USAGE_ACCELERATION_STRUCTURE_STORAGE_BIT_KHR | VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, buffer, memory);

    // ========================================================================================
    // ========================================================================================
    // create acceleration structure
    // ========================================================================================
    // ========================================================================================
    {
        VkAccelerationStructureCreateInfoKHR createInfo = {};
        createInfo.sType                                = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_CREATE_INFO_KHR;
        createInfo.buffer                               = buffer;
        createInfo.size                                 = sizes_info.accelerationStructureSize;
        createInfo.type                                 = VK_ACCELERATION_STRUCTURE_TYPE_BOTTOM_LEVEL_KHR;

        p_inst->func_ptrs.vkCreateAccelerationStructureKHR(device, &createInfo, nullptr, &handle);
    }

    // ========================================================================================
    // ========================================================================================
    // create its scratch buffer
    // ========================================================================================
    // ========================================================================================
    scratch_buff.init(device, phys_dev, sizes_info.buildScratchSize, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT);

    // ========================================================================================
    // ========================================================================================
    // record build
    // ========================================================================================
    // ========================================================================================
    {
        VkAccelerationStructureBuildGeometryInfoKHR build_info           = {};
        build_info.sType                                                 = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_BUILD_GEOMETRY_INFO_KHR;
        build_info.type                                                  = VK_ACCELERATION_STRUCTURE_TYPE_BOTTOM_LEVEL_KHR;
        build_info.flags                                                 = VK_BUILD_ACCELERATION_STRUCTURE_PREFER_FAST_TRACE_BIT_KHR;
        build_info.mode                                                  = VK_BUILD_ACCELERATION_STRUCTURE_MODE_BUILD_KHR;
        build_info.dstAccelerationStructure                              = handle;
        build_info.geometryCount                                         = 1;
        build_info.pGeometries                                           = &as_geom;
        build_info.scratchData.deviceAddress                             = scratch_buff.device_addr;

        std::vector<VkAccelerationStructureBuildRangeInfoKHR> rangeInfos = {};
        {
            VkAccelerationStructureBuildRangeInfoKHR info = {};
            info.firstVertex                              = 0;
            info.primitiveOffset                          = 0;
            info.primitiveCount                           = 1;
            info.transformOffset                          = 0;

            rangeInfos.push_back(info);
        }

        std::vector<VkAccelerationStructureBuildRangeInfoKHR*> pRangeInfos(rangeInfos.size());
        for (size_t i = 0; i < rangeInfos.size(); i++) pRangeInfos[i] = &rangeInfos[i];

        p_inst->func_ptrs.vkCmdBuildAccelerationStructuresKHR(cmd_buff, 1, &build_info, pRangeInfos.data());
    }

    // ========================================================================================
    // ========================================================================================
    // ========================================================================================
    // ========================================================================================
    deviceAddress = util::get_buff_dev_addr(device, buffer);
}

// ====================================================================================================================
// ====================================================================================================================
// ====================================================================================================================
// ====================================================================================================================
void zp::gpu::Blas::cleanup()
{
    scratch_buff.cleanup(device);

    p_inst->func_ptrs.vkDestroyAccelerationStructureKHR(device, handle, nullptr);
    vkFreeMemory(device, memory, nullptr);
    vkDestroyBuffer(device, buffer, nullptr);
}