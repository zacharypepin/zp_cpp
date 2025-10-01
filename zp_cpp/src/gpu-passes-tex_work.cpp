#include "zp_cpp/gpu.hpp"

// ====================================================================================================================
// ====================================================================================================================
// ====================================================================================================================
// ====================================================================================================================
void zp::gpu::passes::tex_work::Instance::record_init(VkCommandBuffer cmd_buff)
{
    // ========================================================================================
    // ========================================================================================
    // setup staging buff
    // ========================================================================================
    // ========================================================================================
    {
        state.staging_buff.init(setup.device, setup.phys_dev, sizeof(std::byte), MAX_SIZE, VK_BUFFER_USAGE_TRANSFER_SRC_BIT);
    }
}

// ====================================================================================================================
// ====================================================================================================================
// ====================================================================================================================
// ====================================================================================================================
void zp::gpu::passes::tex_work::Instance::update_buffs()
{
    state.staging_buff.reset();
    state.staged_uploads.clear();

    while (!shared.requests.empty())
    {
        auto& request       = shared.requests.front();
        size_t request_size = request.p_bytes->size();

        if (state.staging_buff.max_count < state.staging_buff.count + request_size)
        {
            break;
        }

        RegionHandle region               = state.staging_buff.push(request.p_bytes->data(), request.p_bytes->size());

        State::StagedUpload staged_upload = {};
        staged_upload.region              = region;
        staged_upload.p_img               = request.p_img;

        state.staged_uploads.push_back(staged_upload);

        shared.requests.pop_front();
    }
}

// ====================================================================================================================
// ====================================================================================================================
// ====================================================================================================================
// ====================================================================================================================
void zp::gpu::passes::tex_work::Instance::record_cmd_buff(VkCommandBuffer cmd_buff)
{
    for (auto&& staged_upload : state.staged_uploads)
    {
        util::record_trans_image_layout(setup.p_inst, cmd_buff, staged_upload.p_img->handle, staged_upload.p_img->format, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);

        util::record_copy_buff_to_image(cmd_buff, state.staging_buff.handle, staged_upload.region.start_idx, staged_upload.p_img->handle, staged_upload.p_img->width, staged_upload.p_img->height, staged_upload.p_img->num_channels, staged_upload.p_img->pixel_size, staged_upload.p_img->mip_levels);

        util::record_trans_image_layout(setup.p_inst, cmd_buff, staged_upload.p_img->handle, staged_upload.p_img->format, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
    }
}