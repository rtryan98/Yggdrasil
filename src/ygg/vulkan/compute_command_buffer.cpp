// Copyright 2022 Robert Ryan. See Licence.md.

#include "ygg/vulkan/compute_command_buffer.h"

#include "ygg/vulkan/image_utils.h"
#include "ygg/vulkan/resource.h"

#include <volk.h>

namespace ygg::vk
{
    void Compute_command_buffer::bind_descriptor_set(VkPipelineBindPoint bind_point,
        VkPipelineLayout layout, uint32_t set_offset, VkDescriptorSet set) const
    {
        vkCmdBindDescriptorSets(m_cmdbuf, bind_point, layout, set_offset, 1, &set, 0, nullptr);
    }

    void Compute_command_buffer::bind_descriptor_sets(VkPipelineBindPoint bind_point,
        VkPipelineLayout layout, uint32_t first_set, const std::span<VkDescriptorSet>& sets) const
    {
        vkCmdBindDescriptorSets(m_cmdbuf, bind_point, layout, first_set, uint32_t(sets.size()), sets.data(), 0, nullptr);
    }

    void Compute_command_buffer::bind_pipeline(const Pipeline& pipeline) const
    {
        vkCmdBindPipeline(m_cmdbuf, pipeline.bind_point, pipeline.handle);
    }

    void Compute_command_buffer::clear_image(const Image& image, VkImageLayout layout,
        const Clear_value& clear_value, uint32_t base_mip_level, uint32_t level_count,
        uint32_t base_array_layer, uint32_t layer_count) const
    {
        static_assert(sizeof(VkClearValue) == sizeof(Clear_value));

        VkImageSubresourceRange range = {
            .aspectMask = img_utils::get_default_aspect_mask(image.info.format),
            .baseMipLevel = base_mip_level,
            .levelCount = level_count,
            .baseArrayLayer = base_array_layer,
            .layerCount = layer_count
        };
        VkClearValue cv = {};
        memcpy(&cv, &clear_value, sizeof(VkClearValue));

        auto format_info = img_utils::get_format_info(image.info.format);
        if (format_info.depth || format_info.stencil) {
            vkCmdClearDepthStencilImage(m_cmdbuf, image.allocated_image.handle, layout,
                &cv.depthStencil, 1, &range);
        }
        else if (format_info.red || format_info.green || format_info.blue || format_info.alpha) {
            vkCmdClearColorImage(m_cmdbuf, image.allocated_image.handle, layout,
                &cv.color, 1, &range);
        }
    }

    void Compute_command_buffer::dispatch(uint32_t x, uint32_t y, uint32_t z) const
    {
        vkCmdDispatch(m_cmdbuf, x, y, z);
    }

    void Compute_command_buffer::dispatch_base(uint32_t x, uint32_t y, uint32_t z,
        uint32_t x_offset, uint32_t y_offset, uint32_t z_offset) const
    {
        vkCmdDispatchBase(m_cmdbuf, x_offset, y_offset, z_offset, x, y, z);
    }

    void Compute_command_buffer::dispatch_indirect(const Buffer& buffer, uint32_t buffer_offset) const
    {
        vkCmdDispatchIndirect(m_cmdbuf, select_allocated_buffer(buffer, m_frame_in_flight).handle, buffer_offset);
    }

    void Compute_command_buffer::push_constants(VkPipelineLayout layout,
        VkShaderStageFlags stages, const void* data, uint32_t size, uint32_t offset) const
    {
        vkCmdPushConstants(m_cmdbuf, layout, stages, offset, size, data);
    }
}
