// Copyright 2022 Robert Ryan. See Licence.md.

#include "ygg/vulkan/graphics_command_buffer.h"

#include "ygg/vulkan/resource.h"

#include <array>
#include <volk.h>

namespace ygg::vk
{
    void Graphics_command_buffer::bind_index_buffer(const Buffer& buffer, VkDeviceSize offset, VkIndexType type) const
    {
        vkCmdBindIndexBuffer(m_cmdbuf, select_allocated_buffer(buffer, m_frame_in_flight).handle, offset, type);
    }

    void Graphics_command_buffer::begin_rendering(const Rendering_info& info) const
    {
        static_assert(sizeof(VkClearValue) == sizeof(Clear_value));

        std::array<VkRenderingAttachmentInfo, 32> render_attachments = {};
        VkRenderingAttachmentInfo depth_attachment;
        VkRenderingAttachmentInfo stencil_attachment;

        VkRenderingInfo ri = {
            .sType = VK_STRUCTURE_TYPE_RENDERING_INFO,
            .pNext = nullptr,
            .flags = 0,
            .renderArea = { { info.offset_x, info.offset_y }, { info.width, info.height } },
            .layerCount = 1,
            .viewMask = 0,
        };
        if (info.color_attachments.has_value()) {
            const auto& cas = info.color_attachments.value();
            for (uint32_t i = 0; i < cas.size(); i++) {
                const auto& attachment = cas[i];
                render_attachments[i] = {
                    .sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO,
                    .pNext = nullptr,
                    .imageView = attachment.view,
                    .imageLayout = attachment.layout,
                    .loadOp = attachment.load_op,
                    .storeOp = attachment.store_op,
                };
                memcpy(&render_attachments[i].clearValue, &attachment.clear_value, sizeof(VkClearValue));
            }
            ri.colorAttachmentCount = uint32_t(cas.size());
            ri.pColorAttachments = render_attachments.data();
        }
        if (info.depth_attachment.has_value()) {
            const auto& d = info.depth_attachment.value();
            depth_attachment = {
                .sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO,
                .pNext = nullptr,
                .imageView = d.view,
                .imageLayout = d.layout,
                .loadOp = d.load_op,
                .storeOp = d.store_op,
            };
            memcpy(&depth_attachment.clearValue, &d.clear_value, sizeof(VkClearValue));
            ri.pDepthAttachment = &depth_attachment;
        }
        if (info.stencil_attachment.has_value()) {
            const auto& s = info.stencil_attachment.value();
            stencil_attachment = {
                .sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO,
                .pNext = nullptr,
                .imageView = s.view,
                .imageLayout = s.layout,
                .loadOp = s.load_op,
                .storeOp = s.store_op,
            };
            memcpy(&stencil_attachment.clearValue, &s.clear_value, sizeof(VkClearValue));
            ri.pStencilAttachment = &stencil_attachment;
        }
        vkCmdBeginRendering(m_cmdbuf, &ri);
    }

    void Graphics_command_buffer::blit(const Image& src, VkImageLayout src_layout, const Image& dst,
        VkImageLayout dst_layout, const Image_blit_info& blit_info) const
    {
        VkImageBlit2 region = {
            .sType = VK_STRUCTURE_TYPE_IMAGE_BLIT_2,
            .pNext = nullptr,
            .srcSubresource = {
                .aspectMask = blit_info.src_aspect_mask,
                .mipLevel = blit_info.src_mip_level,
                .baseArrayLayer = blit_info.src_base_array_layer,
                .layerCount = blit_info.src_layer_count
            },
            .srcOffsets = {
                { blit_info.src_offsets[0].x, blit_info.src_offsets[0].y, blit_info.src_offsets[0].z },
                { blit_info.src_offsets[1].x, blit_info.src_offsets[1].y, blit_info.src_offsets[1].z }
            },
            .dstSubresource = {
                .aspectMask = blit_info.dst_aspect_mask,
                .mipLevel = blit_info.dst_mip_level,
                .baseArrayLayer = blit_info.dst_base_array_layer,
                .layerCount = blit_info.dst_layer_count
            },
            .dstOffsets = {
                { blit_info.dst_offsets[0].x, blit_info.dst_offsets[0].y, blit_info.dst_offsets[0].z },
                { blit_info.dst_offsets[1].x, blit_info.dst_offsets[1].y, blit_info.dst_offsets[1].z }
            }
        };
        VkBlitImageInfo2 b = {
            .sType = VK_STRUCTURE_TYPE_BLIT_IMAGE_INFO_2,
            .pNext = nullptr,
            .srcImage = src.allocated_image.handle,
            .srcImageLayout = src_layout,
            .dstImage = dst.allocated_image.handle,
            .dstImageLayout = dst_layout,
            .regionCount = 1,
            .pRegions = &region,
            .filter = blit_info.filter
        };
        vkCmdBlitImage2(m_cmdbuf, &b);
    }

    void Graphics_command_buffer::draw(uint32_t vertex_count, uint32_t instance_count, uint32_t first_vertex,
        uint32_t first_instance) const
    {
        vkCmdDraw(m_cmdbuf, vertex_count, instance_count, first_vertex, first_instance);
    }

    void Graphics_command_buffer::draw_indexed(uint32_t index_count, uint32_t instance_count, uint32_t first_index,
        uint32_t vertex_offset, uint32_t first_instance) const
    {
        vkCmdDrawIndexed(m_cmdbuf, index_count, instance_count, first_index, vertex_offset, first_instance);
    }

    void Graphics_command_buffer::draw_indexed_indirect(const Buffer& buffer, VkDeviceSize buffer_offset,
        uint32_t draw_count, uint32_t stride) const
    {
        vkCmdDrawIndexedIndirect(m_cmdbuf, select_allocated_buffer(buffer, m_frame_in_flight).handle, buffer_offset,
            draw_count, stride);
    }

    void Graphics_command_buffer::draw_indexed_indirect_count(const Buffer& buffer, VkDeviceSize buffer_offset,
        const Buffer& count_buffer, VkDeviceSize count_buffer_offset, uint32_t max_draw_count, uint32_t stride) const
    {
        vkCmdDrawIndexedIndirectCount(m_cmdbuf, select_allocated_buffer(buffer, m_frame_in_flight).handle, buffer_offset,
            select_allocated_buffer(count_buffer, m_frame_in_flight).handle, count_buffer_offset, max_draw_count, stride);
    }

    void Graphics_command_buffer::draw_indirect(const Buffer& buffer, VkDeviceSize buffer_offset,
        uint32_t draw_count, uint32_t stride) const
    {
        vkCmdDrawIndirect(m_cmdbuf, select_allocated_buffer(buffer, m_frame_in_flight).handle, buffer_offset,
            draw_count, stride);
    }

    void Graphics_command_buffer::draw_indirect_count(const Buffer& buffer, VkDeviceSize buffer_offset,
        const Buffer& count_buffer, VkDeviceSize count_buffer_offset, uint32_t max_draw_count, uint32_t stride) const
    {
        vkCmdDrawIndirectCount(m_cmdbuf, select_allocated_buffer(buffer, m_frame_in_flight).handle, buffer_offset,
            select_allocated_buffer(count_buffer, m_frame_in_flight).handle, count_buffer_offset, max_draw_count, stride);
    }

    void Graphics_command_buffer::end_rendering() const
    {
        vkCmdEndRendering(m_cmdbuf);
    }

    void Graphics_command_buffer::set_scissor(int32_t offset_x, int32_t offset_y, uint32_t width, uint32_t height) const
    {
        VkRect2D sc = {
            .offset = { offset_x, offset_y },
            .extent = { width, height }
        };
        vkCmdSetScissor(m_cmdbuf, 0, 1, &sc);
    }

    void Graphics_command_buffer::set_viewport(float_t x, float_t y, float_t width, float_t height,
        float_t min_depth, float_t max_depth) const
    {
        VkViewport vp = {
            .x = x,
            .y = y,
            .width = width,
            .height = height,
            .minDepth = min_depth,
            .maxDepth = max_depth
        };
        vkCmdSetViewport(m_cmdbuf, 0, 1, &vp);
    }
}
