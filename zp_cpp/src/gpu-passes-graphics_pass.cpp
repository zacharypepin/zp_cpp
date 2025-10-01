#include "zp_cpp/gpu.hpp"

using namespace zp::gpu;
using namespace zp::gpu::passes;
using namespace zp::gpu::passes::graphics;

// ========================================================================================================================================
// ========================================================================================================================================
// ========================================================================================================================================
// ========================================================================================================================================
#define IMPL p_inst->graphics_sys.p_i

// ========================================================================================================================================
// ========================================================================================================================================
// ========================================================================================================================================
// ========================================================================================================================================
namespace
{
    using colour_format = VkFormat;
    using depth_format  = VkFormat;
    using pipeline_key  = std::tuple<colour_format, depth_format, shader_group_uuid>;
}

// ========================================================================================================================================
// ========================================================================================================================================
// ========================================================================================================================================
// ========================================================================================================================================
struct System::Internal
{
    PFN_vkCmdSetPolygonModeEXT vkCmdSetPolygonModeEXT;
    PFN_vkCmdSetColorBlendEnableEXT vkCmdSetColorBlendEnableEXT;

    PFN_vkCmdBindDescriptorBuffersEXT vkCmdBindDescriptorBuffersEXT;
    PFN_vkCmdSetDescriptorBufferOffsetsEXT vkCmdSetDescriptorBufferOffsetsEXT;

    std::unordered_map<pipeline_key, VkPipeline, tuple_hash> pipelines;
};

// ========================================================================================================================================
// ========================================================================================================================================
// ========================================================================================================================================
// ========================================================================================================================================
void graphics::init(Instance* p_inst)
{
    // ============================================================================================
    // ============================================================================================
    // ============================================================================================
    // ============================================================================================
    {
        IMPL = new System::Internal();
    }

    // ========================================================================================
    // ========================================================================================
    // ========================================================================================
    // ========================================================================================
    {
        IMPL->vkCmdSetPolygonModeEXT             = (PFN_vkCmdSetPolygonModeEXT)vkGetDeviceProcAddr(p_inst->vk_dev, "vkCmdSetPolygonModeEXT");
        IMPL->vkCmdSetColorBlendEnableEXT        = (PFN_vkCmdSetColorBlendEnableEXT)vkGetDeviceProcAddr(p_inst->vk_dev, "vkCmdSetColorBlendEnableEXT");
        IMPL->vkCmdBindDescriptorBuffersEXT      = (PFN_vkCmdBindDescriptorBuffersEXT)vkGetDeviceProcAddr(p_inst->vk_dev, "vkCmdBindDescriptorBuffersEXT");
        IMPL->vkCmdSetDescriptorBufferOffsetsEXT = (PFN_vkCmdSetDescriptorBufferOffsetsEXT)vkGetDeviceProcAddr(p_inst->vk_dev, "vkCmdSetDescriptorBufferOffsetsEXT");
    }
}

// ========================================================================================================================================
// ========================================================================================================================================
// ========================================================================================================================================
// ========================================================================================================================================
void graphics::record_cmd_buff(Instance* p_inst, VkCommandBuffer cmd_buff, DeviceLocalImage* p_colour, DeviceLocalImage* p_depth, bool should_clear, std::vector<DrawReq>* p_draw_reqs)
{
    // ========================================================================================
    // ========================================================================================
    // ========================================================================================
    // ========================================================================================
    {
        for (const auto& draw_req : *p_draw_reqs)
        {
            pipeline_key key = {p_colour->format, p_depth->format, draw_req.shader_group};

            bool is_cached   = IMPL->pipelines.contains(key);

            if (!is_cached)
            {
                VkShaderModule v;
                VkShaderModule f;

                zp::gpu::util::create_shader_module(p_inst->vk_dev, v, &p_inst->shader_groups.at(draw_req.shader_group).vert);
                zp::gpu::util::create_shader_module(p_inst->vk_dev, f, &p_inst->shader_groups.at(draw_req.shader_group).frag);

                VkPipelineShaderStageCreateInfo v_info                        = {};
                v_info.sType                                                  = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
                v_info.stage                                                  = VK_SHADER_STAGE_VERTEX_BIT;
                v_info.module                                                 = v;
                v_info.pName                                                  = "main";

                VkPipelineShaderStageCreateInfo f_info                        = {};
                f_info.sType                                                  = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
                f_info.stage                                                  = VK_SHADER_STAGE_FRAGMENT_BIT;
                f_info.module                                                 = f;
                f_info.pName                                                  = "main";

                std::array<VkPipelineShaderStageCreateInfo, 2> shader_stages  = {v_info, f_info};

                VkPipelineVertexInputStateCreateInfo vert_state_info          = {};
                vert_state_info.sType                                         = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
                vert_state_info.vertexBindingDescriptionCount                 = 0;
                vert_state_info.vertexAttributeDescriptionCount               = 0;
                vert_state_info.pVertexBindingDescriptions                    = nullptr;
                vert_state_info.pVertexAttributeDescriptions                  = nullptr;

                VkPipelineInputAssemblyStateCreateInfo input_info             = {};
                input_info.sType                                              = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
                input_info.topology                                           = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
                input_info.primitiveRestartEnable                             = VK_FALSE;

                VkPipelineViewportStateCreateInfo viewport_state              = {};
                viewport_state.sType                                          = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
                viewport_state.viewportCount                                  = 1;
                viewport_state.scissorCount                                   = 1;

                VkPipelineRasterizationStateCreateInfo rasterizer             = {};
                rasterizer.sType                                              = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
                rasterizer.depthClampEnable                                   = VK_FALSE;
                rasterizer.rasterizerDiscardEnable                            = VK_FALSE;
                rasterizer.polygonMode                                        = VK_POLYGON_MODE_FILL;
                rasterizer.lineWidth                                          = 1.0f;
                rasterizer.cullMode                                           = VK_CULL_MODE_NONE;
                rasterizer.frontFace                                          = VK_FRONT_FACE_COUNTER_CLOCKWISE;
                rasterizer.depthBiasEnable                                    = VK_FALSE;

                VkPipelineMultisampleStateCreateInfo multisamp                = {};
                multisamp.sType                                               = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
                multisamp.sampleShadingEnable                                 = VK_FALSE;
                multisamp.rasterizationSamples                                = VK_SAMPLE_COUNT_1_BIT;

                VkPipelineColorBlendAttachmentState attach_info               = {};
                attach_info.colorWriteMask                                    = VK_COLOR_COMPONENT_R_BIT;
                attach_info.colorWriteMask                                   |= VK_COLOR_COMPONENT_G_BIT;
                attach_info.colorWriteMask                                   |= VK_COLOR_COMPONENT_B_BIT;
                attach_info.colorWriteMask                                   |= VK_COLOR_COMPONENT_A_BIT;
                attach_info.blendEnable                                       = VK_TRUE;
                attach_info.srcColorBlendFactor                               = VK_BLEND_FACTOR_ONE;
                attach_info.dstColorBlendFactor                               = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
                attach_info.colorBlendOp                                      = VK_BLEND_OP_ADD;
                attach_info.srcAlphaBlendFactor                               = VK_BLEND_FACTOR_ONE;
                attach_info.dstAlphaBlendFactor                               = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
                attach_info.alphaBlendOp                                      = VK_BLEND_OP_ADD;

                VkPipelineColorBlendStateCreateInfo colour_blend              = {};
                colour_blend.sType                                            = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
                colour_blend.logicOpEnable                                    = VK_FALSE;
                colour_blend.attachmentCount                                  = 1;
                colour_blend.pAttachments                                     = &attach_info;

                std::vector<VkDynamicState> dynamic_states                    = {
                    VK_DYNAMIC_STATE_VIEWPORT,
                    VK_DYNAMIC_STATE_SCISSOR,
                    VK_DYNAMIC_STATE_POLYGON_MODE_EXT,
                    VK_DYNAMIC_STATE_LINE_WIDTH,
                    VK_DYNAMIC_STATE_CULL_MODE,
                    VK_DYNAMIC_STATE_FRONT_FACE,
                    VK_DYNAMIC_STATE_DEPTH_TEST_ENABLE,
                    VK_DYNAMIC_STATE_DEPTH_WRITE_ENABLE,
                    VK_DYNAMIC_STATE_DEPTH_COMPARE_OP,
                    VK_DYNAMIC_STATE_COLOR_BLEND_ENABLE_EXT,
                };
                VkPipelineDynamicStateCreateInfo dynamic_state            = {};
                dynamic_state.sType                                       = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
                dynamic_state.dynamicStateCount                           = (uint32_t)dynamic_states.size();
                dynamic_state.pDynamicStates                              = dynamic_states.data();

                VkPipelineRenderingCreateInfo rendering_info              = {};
                rendering_info.sType                                      = VK_STRUCTURE_TYPE_PIPELINE_RENDERING_CREATE_INFO;
                rendering_info.viewMask                                   = 0;
                rendering_info.colorAttachmentCount                       = 1;
                rendering_info.pColorAttachmentFormats                    = &p_colour->format;
                rendering_info.depthAttachmentFormat                      = p_depth->format;
                rendering_info.stencilAttachmentFormat                    = VK_FORMAT_UNDEFINED;
                rendering_info.pNext                                      = nullptr;

                VkPipelineDepthStencilStateCreateInfo depth_stencil_state = {};
                depth_stencil_state.sType                                 = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
                depth_stencil_state.depthTestEnable                       = VK_TRUE;
                depth_stencil_state.depthWriteEnable                      = VK_TRUE;
                depth_stencil_state.depthCompareOp                        = VK_COMPARE_OP_LESS;
                depth_stencil_state.depthBoundsTestEnable                 = VK_FALSE;
                depth_stencil_state.stencilTestEnable                     = VK_FALSE;

                VkGraphicsPipelineCreateInfo pipeline_info                = {};
                pipeline_info.sType                                       = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
                pipeline_info.stageCount                                  = shader_stages.size();
                pipeline_info.pStages                                     = shader_stages.data();
                pipeline_info.pVertexInputState                           = &vert_state_info;
                pipeline_info.pInputAssemblyState                         = &input_info;
                pipeline_info.pViewportState                              = &viewport_state;
                pipeline_info.pRasterizationState                         = &rasterizer;
                pipeline_info.pMultisampleState                           = &multisamp;
                pipeline_info.pColorBlendState                            = &colour_blend;
                pipeline_info.pDynamicState                               = &dynamic_state;
                pipeline_info.layout                                      = p_inst->desc_sys.vk_pipeline_layout;
                pipeline_info.renderPass                                  = nullptr;
                pipeline_info.pDepthStencilState                          = &depth_stencil_state;
                pipeline_info.subpass                                     = 0;
                pipeline_info.basePipelineHandle                          = VK_NULL_HANDLE;
                pipeline_info.pNext                                       = &rendering_info;
                pipeline_info.flags                                       = VK_PIPELINE_CREATE_DESCRIPTOR_BUFFER_BIT_EXT;

                VK_CHECK(vkCreateGraphicsPipelines(p_inst->vk_dev, VK_NULL_HANDLE, 1, &pipeline_info, nullptr, &IMPL->pipelines[key]), "vkCreateGraphicsPipelines");

                vkDestroyShaderModule(p_inst->vk_dev, v, nullptr);
                vkDestroyShaderModule(p_inst->vk_dev, f, nullptr);
            }
        }
    }

    // ========================================================================================
    // ========================================================================================
    // begin rendering
    // ========================================================================================
    // ========================================================================================
    {
        const VkRenderingAttachmentInfo colour = {
            .sType       = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO,
            .pNext       = nullptr,
            .imageView   = p_colour->view,
            .imageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
            .loadOp      = should_clear ? VK_ATTACHMENT_LOAD_OP_CLEAR : VK_ATTACHMENT_LOAD_OP_LOAD,
            .storeOp     = VK_ATTACHMENT_STORE_OP_STORE,
            .clearValue  = VkClearValue{.color = {0.0f, 0.0f, 0.0f, 0.0f}},
        };

        const VkRenderingAttachmentInfo depth = {
            .sType       = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO,
            .pNext       = nullptr,
            .imageView   = p_depth->view,
            .imageLayout = VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL,
            .loadOp      = VK_ATTACHMENT_LOAD_OP_CLEAR,
            .storeOp     = VK_ATTACHMENT_STORE_OP_DONT_CARE,
            .clearValue  = VkClearValue{.depthStencil = {1.0f, 0}},
        };

        const VkRenderingInfo info = {
            .sType                = VK_STRUCTURE_TYPE_RENDERING_INFO,
            .pNext                = nullptr,
            .renderArea           = {.offset = VkOffset2D{0, 0}, .extent = {p_colour->width, p_colour->height}},
            .layerCount           = 1,
            .colorAttachmentCount = 1,
            .pColorAttachments    = &colour,
            .pDepthAttachment     = &depth,
            .pStencilAttachment   = nullptr,
        };
        vkCmdBeginRendering(cmd_buff, &info);
    }

    // ========================================================================================
    // ========================================================================================
    // dyn state
    // ========================================================================================
    // ========================================================================================
    {
        const VkViewport viewport = {
            .x        = 0.0f,
            .y        = 0.0f,
            .width    = (float)p_colour->width,
            .height   = (float)p_colour->height,
            .minDepth = 0.0f,
            .maxDepth = 1.0f,
        };
        vkCmdSetViewport(cmd_buff, 0, 1, &viewport);
    }

    // ========================================================================================
    // ========================================================================================
    // per draw req
    // ========================================================================================
    // ========================================================================================
    {
        for (auto&& draw_req : *p_draw_reqs)
        {
            const VkRect2D scissor = {
                .offset = {   int32_t(draw_req.scissor_nrm.xy.x * p_colour->width),  (int32_t)(draw_req.scissor_nrm.xy.y * p_colour->height)},
                .extent = {(uint32_t)(draw_req.scissor_nrm.wh.x * p_colour->width), (uint32_t)(draw_req.scissor_nrm.wh.y * p_colour->height)},
            };
            vkCmdSetScissor(cmd_buff, 0, 1, &scissor);

            if (draw_req.is_point_draw)
            {
                IMPL->vkCmdSetPolygonModeEXT(cmd_buff, VK_POLYGON_MODE_POINT);
                vkCmdSetLineWidth(cmd_buff, 1.0f);
            }
            else if (draw_req.is_line_draw)
            {
                IMPL->vkCmdSetPolygonModeEXT(cmd_buff, VK_POLYGON_MODE_LINE);
                vkCmdSetLineWidth(cmd_buff, 5.0f);
            }
            else
            {
                IMPL->vkCmdSetPolygonModeEXT(cmd_buff, VK_POLYGON_MODE_FILL);
                vkCmdSetLineWidth(cmd_buff, 1.0f);
            }

            vkCmdSetCullMode(cmd_buff, VK_CULL_MODE_NONE);
            vkCmdSetFrontFace(cmd_buff, VK_FRONT_FACE_COUNTER_CLOCKWISE);
            vkCmdSetDepthTestEnable(cmd_buff, draw_req.should_depth_test ? VK_TRUE : VK_FALSE);
            vkCmdSetDepthWriteEnable(cmd_buff, draw_req.should_depth_write ? VK_TRUE : VK_FALSE);
            vkCmdSetDepthCompareOp(cmd_buff, VK_COMPARE_OP_LESS);

            VkBool32 enableBlend = draw_req.is_alpha_blend ? VK_TRUE : VK_FALSE;
            IMPL->vkCmdSetColorBlendEnableEXT(cmd_buff, 0, 1, &enableBlend);

            vkCmdBindPipeline(cmd_buff, VK_PIPELINE_BIND_POINT_GRAPHICS, IMPL->pipelines.at({p_colour->format, p_depth->format, draw_req.shader_group}));

            // desc buffer
            {
                VkDescriptorBufferBindingInfoEXT bindInfo{VK_STRUCTURE_TYPE_DESCRIPTOR_BUFFER_BINDING_INFO_EXT};
                bindInfo.address = p_inst->desc_sys.desc_buff.deviceAddress;
                bindInfo.usage   = VK_BUFFER_USAGE_SAMPLER_DESCRIPTOR_BUFFER_BIT_EXT | VK_BUFFER_USAGE_RESOURCE_DESCRIPTOR_BUFFER_BIT_EXT;
                IMPL->vkCmdBindDescriptorBuffersEXT(cmd_buff, 1, &bindInfo);

                uint32_t bufIndex      = 0;
                VkDeviceSize setOffset = 0;
                IMPL->vkCmdSetDescriptorBufferOffsetsEXT(cmd_buff, VK_PIPELINE_BIND_POINT_GRAPHICS, p_inst->desc_sys.vk_pipeline_layout, 0, 1, &bufIndex, &setOffset);
            }

            vkCmdPushConstants(cmd_buff, p_inst->desc_sys.vk_pipeline_layout, VK_SHADER_STAGE_COMPUTE_BIT | VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(VkDeviceAddress), &draw_req.p_per_draw);
            vkCmdDraw(cmd_buff, draw_req.idx_count, draw_req.inst_count, 0, 0);
        }
    }

    // ========================================================================================
    // ========================================================================================
    // finish
    // ========================================================================================
    // ========================================================================================
    {
        vkCmdEndRendering(cmd_buff);
    }
}

// ========================================================================================================================================
// ========================================================================================================================================
// todo
// ========================================================================================================================================
// ========================================================================================================================================
void graphics::cleanup(Instance* p_inst)
{
    TODO("cleanup");
}

// ========================================================================================================================================
// ========================================================================================================================================
// ========================================================================================================================================
// ========================================================================================================================================
#undef IMPL