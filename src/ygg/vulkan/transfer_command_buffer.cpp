// Copyright 2022 Robert Ryan. See Licence.md.

#include "ygg/vulkan/transfer_command_buffer.h"

#include "ygg/vulkan/command_buffer_recycler.h"
#include "ygg/vulkan/image_utils.h"
#include "ygg/vulkan/linear_host_resource_allocator.h"
#include "ygg/vulkan/resource.h"

#include <cassert>
#include <volk.h>

namespace ygg::vk
{
    Transfer_command_buffer::Transfer_command_buffer(VkCommandBuffer cmdbuf, Linear_host_resource_allocator& allocator,
        uint32_t frame_in_flight, uint32_t queue_family_index)
        : m_allocator(allocator), m_cmdbuf(cmdbuf), m_pipeline_barrier_builder(m_cmdbuf), m_frame_in_flight(frame_in_flight),
        m_queue_family_index(queue_family_index)
    {}

    void cmd_upload_buffer(VkCommandBuffer cmdbuf, Linear_host_resource_allocator::Mapped_host_buffer& src,
        Buffer& dst, VkDeviceSize size, VkDeviceSize offset, uint32_t frame_in_flight)
    {
        VkBufferCopy2 copy_region = {
            .sType = VK_STRUCTURE_TYPE_BUFFER_COPY_2,
            .pNext = nullptr,
            .srcOffset = 0ull,
            .dstOffset = offset,
            .size = size
        };
        VkCopyBufferInfo2 copy_info = {
            .sType = VK_STRUCTURE_TYPE_COPY_BUFFER_INFO_2,
            .pNext = nullptr,
            .srcBuffer = src.buf,
            .dstBuffer = select_allocated_buffer(dst, frame_in_flight).handle,
            .regionCount = 1,
            .pRegions = &copy_region
        };
        vkCmdCopyBuffer2(cmdbuf, &copy_info);
    }

    void cmd_upload_color_image(VkCommandBuffer cmdbuf, Linear_host_resource_allocator::Mapped_host_buffer& src,
        Image& dst, uint32_t width, uint32_t height, uint32_t depth, uint32_t width_offset, uint32_t height_offset,
        uint32_t depth_offset, uint32_t layers, uint32_t base_layer, uint32_t mip_level)
    {
        VkBufferImageCopy2 copy_region = {
            .sType = VK_STRUCTURE_TYPE_BUFFER_IMAGE_COPY_2,
            .pNext = nullptr,
            .bufferOffset = 0,
            .bufferRowLength = 0,
            .bufferImageHeight = 0,
            .imageSubresource = {
                .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
                .mipLevel = mip_level,
                .baseArrayLayer = base_layer,
                .layerCount = layers,
            },
            .imageOffset = { width_offset, height_offset, depth_offset },
            .imageExtent = { width, height, depth }
        };
        VkCopyBufferToImageInfo2 copy_info = {
            .sType = VK_STRUCTURE_TYPE_COPY_BUFFER_TO_IMAGE_INFO_2,
            .pNext = nullptr,
            .srcBuffer = src.buf,
            .dstImage = dst.allocated_image.handle,
            .dstImageLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
            .regionCount = 1,
            .pRegions = &copy_region
        };
        vkCmdCopyBufferToImage2(cmdbuf, &copy_info);
    }

    void Transfer_command_buffer::upload_buffer_data(Buffer& buffer, void* data, VkDeviceSize size, VkDeviceSize offset)
    {
        switch (buffer.info.domain)
        {
        default: {
            auto& allocation = m_allocator.allocate_buffer(size, m_queue_family_index);
            memcpy(allocation.mapped_data, data, size);
            cmd_upload_buffer(m_cmdbuf, allocation, buffer, size, offset, m_frame_in_flight);
            break;
        }
        case Buffer_domain::Device_host_visible:
            ;
        case Buffer_domain::Host_write_combined: {
            auto& allocated_buffer = select_allocated_buffer(buffer, m_frame_in_flight);
            memcpy(&static_cast<uint8_t*>(allocated_buffer.mapped_data)[offset], data, size);
            break;
        }
        }
    }

    void Transfer_command_buffer::upload_image_data(Image& image, void* data, uint32_t width, uint32_t height,
        uint32_t depth, uint32_t width_offset, uint32_t height_offset, uint32_t depth_offset, uint32_t layers,
        uint32_t base_layer, uint32_t mip_level)
    {
        auto& allocated_buffer = m_allocator
            .allocate_buffer(width * height * depth * img_utils::format_size(image.info.format), m_queue_family_index);
        cmd_upload_color_image(m_cmdbuf, allocated_buffer, image, width, height, depth,
            width_offset, height_offset, depth_offset, layers, base_layer, mip_level);
    }

    void* Transfer_command_buffer::allocate_and_upload_buffer_data(Buffer& buffer, VkDeviceSize size, VkDeviceSize offset)
    {
        if (buffer.info.domain != Buffer_domain::Device) {
            assert(false, "The used Buffer_domain in the passed buffer must be Buffer_domain::Device to use this function.");
            return nullptr;
        }
        auto& allocation = m_allocator.allocate_buffer(size, m_queue_family_index);
        cmd_upload_buffer(m_cmdbuf, allocation, buffer, size, offset, m_frame_in_flight);
        return allocation.mapped_data;
    }

    void* Transfer_command_buffer::allocate_and_upload_image_data(Image& image, uint32_t width,
        uint32_t height, uint32_t depth, uint32_t width_offset, uint32_t height_offset,
        uint32_t depth_offset, uint32_t layers, uint32_t base_layer, uint32_t mip_level)
    {
        auto& allocated_buffer = m_allocator
            .allocate_buffer(width * height * depth * img_utils::format_size(image.info.format), m_queue_family_index);
        cmd_upload_color_image(m_cmdbuf, allocated_buffer, image, width, height, depth,
            width_offset, height_offset, depth_offset, layers, base_layer, mip_level);
        return allocated_buffer.mapped_data;
    }

    void Transfer_command_buffer::copy_buffer(const Buffer& src, const Buffer& dst,
        VkDeviceSize src_offset, VkDeviceSize dst_offset, VkDeviceSize size) const
    {
        VkBufferCopy2 region = {
            .sType = VK_STRUCTURE_TYPE_BUFFER_COPY_2,
            .pNext = nullptr,
            .srcOffset = src_offset,
            .dstOffset = dst_offset,
            .size = size
        };
        VkCopyBufferInfo2 copy_info = {
            .sType = VK_STRUCTURE_TYPE_COPY_BUFFER_INFO_2,
            .pNext = nullptr,
            .srcBuffer = select_allocated_buffer(src, m_frame_in_flight).handle,
            .dstBuffer = select_allocated_buffer(dst, m_frame_in_flight).handle,
            .regionCount = 1,
            .pRegions = &region
        };
        vkCmdCopyBuffer2(m_cmdbuf, &copy_info);
    }

    VkBufferImageCopy2 buffer_image_copy_from_info(const Buffer_image_copy_info& info, VkImageAspectFlags aspect)
    {
        return {
            .sType = VK_STRUCTURE_TYPE_BUFFER_IMAGE_COPY_2,
            .pNext = nullptr,
            .bufferOffset = info.buffer_offset,
            .bufferRowLength = info.buffer_row_length,
            .bufferImageHeight = info.buffer_image_height,
            .imageSubresource = {
                .aspectMask = aspect,
                .mipLevel = info.image_mip_level,
                .baseArrayLayer = info.image_base_array_layer,
                .layerCount = info.image_layer_count
            },
            .imageOffset = { info.image_offset_x, info.image_offset_y, info.image_offset_z },
            .imageExtent = { info.image_width, info.image_height, info.image_depth }
        };
    }

    VkCopyBufferToImageInfo2 copy_buffer_image_info(VkBufferImageCopy2* region, uint32_t frame_in_flight,
        const Buffer& buffer, const Image& image, VkImageLayout layout)
    {
        return {
            .sType = VK_STRUCTURE_TYPE_COPY_BUFFER_TO_IMAGE_INFO_2,
            .pNext = nullptr,
            .srcBuffer = select_allocated_buffer(buffer, frame_in_flight).handle,
            .dstImage = image.allocated_image.handle,
            .dstImageLayout = layout,
            .regionCount = 1,
            .pRegions = region
        };
    }

    void Transfer_command_buffer::copy_buffer_to_image(const Buffer& src, const Image& dst,
        VkImageLayout dst_layout, const Buffer_image_copy_info& info) const
    {
        VkBufferImageCopy2 region = buffer_image_copy_from_info(info, img_utils::get_default_aspect_mask(dst.info.format));
        VkCopyBufferToImageInfo2 copy_info = copy_buffer_image_info(&region, m_frame_in_flight, src, dst, dst_layout);
        vkCmdCopyBufferToImage2(m_cmdbuf, &copy_info);
    }

    void Transfer_command_buffer::copy_buffer_to_image(const Buffer& src, const Image& dst,
        VkImageLayout dst_layout, VkImageAspectFlags dst_aspect, const Buffer_image_copy_info& info) const
    {
        VkBufferImageCopy2 region = buffer_image_copy_from_info(info, dst_aspect);
        VkCopyBufferToImageInfo2 copy_info = copy_buffer_image_info(&region, m_frame_in_flight, src, dst, dst_layout);
        vkCmdCopyBufferToImage2(m_cmdbuf, &copy_info);
    }

    VkCopyImageToBufferInfo2 copy_image_buffer_info(VkBufferImageCopy2* region, uint32_t frame_in_flight,
        const Buffer& buffer, const Image& image, VkImageLayout layout)
    {
        return {
            .sType = VK_STRUCTURE_TYPE_COPY_IMAGE_TO_BUFFER_INFO_2,
            .pNext = nullptr,
            .srcImage = image.allocated_image.handle,
            .srcImageLayout = layout,
            .dstBuffer = select_allocated_buffer(buffer, frame_in_flight).handle,
            .regionCount = 1,
            .pRegions = region
        };
    }

    void Transfer_command_buffer::copy_image_to_buffer(const Image& src, const Buffer& dst,
        VkImageLayout src_layout, const Buffer_image_copy_info& info) const
    {
        VkBufferImageCopy2 region = buffer_image_copy_from_info(info, img_utils::get_default_aspect_mask(src.info.format));
        VkCopyImageToBufferInfo2 copy_info = copy_image_buffer_info(&region, m_frame_in_flight, dst, src, src_layout);
        vkCmdCopyImageToBuffer2(m_cmdbuf, &copy_info);
    }

    void Transfer_command_buffer::copy_image_to_buffer(const Image& src, const Buffer& dst,
        VkImageLayout src_layout, VkImageAspectFlags src_aspect, const Buffer_image_copy_info& info) const
    {
        VkBufferImageCopy2 region = buffer_image_copy_from_info(info, src_aspect);
        VkCopyImageToBufferInfo2 copy_info = copy_image_buffer_info(&region, m_frame_in_flight, dst, src, src_layout);
        vkCmdCopyImageToBuffer2(m_cmdbuf, &copy_info);
    }

    void Transfer_command_buffer::copy_image(const Image& src, const Image& dst,
        VkImageLayout src_layout, VkImageLayout dst_layout, const Image_copy_info& info) const
    {
        VkImageCopy2 region = {
            .sType = VK_STRUCTURE_TYPE_IMAGE_COPY_2,
            .pNext = nullptr,
            .srcSubresource = {
                .aspectMask = img_utils::get_default_aspect_mask(src.info.format),
                .mipLevel = info.src_mip_level,
                .baseArrayLayer = info.src_base_array_layer,
                .layerCount = info.src_layer_count
            },
            .srcOffset = { info.src_offset_x, info.src_offset_y, info.src_offset_z },
            .dstSubresource = {
                .aspectMask = img_utils::get_default_aspect_mask(dst.info.format),
                .mipLevel = info.dst_mip_level,
                .baseArrayLayer = info.dst_base_array_layer,
                .layerCount = info.dst_layer_count
            },
            .dstOffset = { info.dst_offset_x, info.dst_offset_y, info.dst_offset_z },
            .extent = { info.width, info.height, info.depth }
        };
        VkCopyImageInfo2 copy_info = {
            .sType = VK_STRUCTURE_TYPE_COPY_IMAGE_INFO_2,
            .pNext = nullptr,
            .srcImage = src.allocated_image.handle,
            .srcImageLayout = src_layout,
            .dstImage = dst.allocated_image.handle,
            .dstImageLayout = dst_layout,
            .regionCount = 1,
            .pRegions = &region
        };
        vkCmdCopyImage2(m_cmdbuf, &copy_info);
    }

    void Transfer_command_buffer::copy_image(const Image& src, const Image& dst,
        VkImageLayout src_layout, VkImageLayout dst_layout, const Image_copy_info& info,
        VkImageAspectFlags src_aspect, VkImageAspectFlags dst_aspect) const
    {
        VkImageCopy2 region = {
            .sType = VK_STRUCTURE_TYPE_IMAGE_COPY_2,
            .pNext = nullptr,
            .srcSubresource = {
                .aspectMask = src_aspect,
                .mipLevel = info.src_mip_level,
                .baseArrayLayer = info.src_base_array_layer,
                .layerCount = info.src_layer_count
            },
            .srcOffset = { info.src_offset_x, info.src_offset_y, info.src_offset_z },
            .dstSubresource = {
                .aspectMask = dst_aspect,
                .mipLevel = info.dst_mip_level,
                .baseArrayLayer = info.dst_base_array_layer,
                .layerCount = info.dst_layer_count
            },
            .dstOffset = { info.dst_offset_x, info.dst_offset_y, info.dst_offset_z },
            .extent = { info.width, info.height, info.depth }
        };
        VkCopyImageInfo2 copy_info = {
            .sType = VK_STRUCTURE_TYPE_COPY_IMAGE_INFO_2,
            .pNext = nullptr,
            .srcImage = src.allocated_image.handle,
            .srcImageLayout = src_layout,
            .dstImage = dst.allocated_image.handle,
            .dstImageLayout = dst_layout,
            .regionCount = 1,
            .pRegions = &region
        };
        vkCmdCopyImage2(m_cmdbuf, &copy_info);
    }
}
