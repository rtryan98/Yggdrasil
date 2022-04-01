// Copyright 2022 Robert Ryan. See Licence.md.

#include "ygg/vulkan/resource.h"

#include "ygg/vulkan/image_utils.h"

#include <cassert>
#include <vk_mem_alloc.h>
#include <volk.h>

namespace ygg::vk
{
    Allocated_buffer& select_allocated_buffer(Buffer& buffer, uint32_t current_frame_in_flight)
    {
        switch (buffer.info.domain)
        {
        default: ;
        case Buffer_domain::Host_write_combined: return buffer.allocated_buffers[0];
        case Buffer_domain::Device:              return buffer.allocated_buffers[0];
        case Buffer_domain::Device_host_visible: return buffer.allocated_buffers[current_frame_in_flight];
        }
    }

    uint32_t get_allocated_buffer_count(Buffer_domain domain, uint32_t max_frames_in_flight)
    {
        switch (domain)
        {
        default: ;
        case Buffer_domain::Host_write_combined: return 1;
        case Buffer_domain::Device:              return 1;
        case Buffer_domain::Device_host_visible: return max_frames_in_flight;
        }
    }

    VkAccessFlags2 buffer_usage_to_access_flags_first_scope(VkBufferUsage usage)
    {
        VkAccessFlags2 flags = 0;
        if (usage & VK_BUFFER_USAGE_TRANSFER_DST_BIT)
            flags |= VK_ACCESS_2_TRANSFER_WRITE_BIT;
        if (usage & VK_BUFFER_USAGE_STORAGE_TEXEL_BUFFER_BIT)
            flags |= VK_ACCESS_2_SHADER_STORAGE_WRITE_BIT;
        if (usage & VK_BUFFER_USAGE_STORAGE_BUFFER_BIT)
            flags |= VK_ACCESS_2_SHADER_STORAGE_WRITE_BIT;
        if (usage & VK_BUFFER_USAGE_TRANSFORM_FEEDBACK_BUFFER_BIT_EXT)
            flags |= VK_ACCESS_2_TRANSFORM_FEEDBACK_WRITE_BIT_EXT;
        if (usage & VK_BUFFER_USAGE_TRANSFORM_FEEDBACK_COUNTER_BUFFER_BIT_EXT)
            flags |= VK_ACCESS_2_TRANSFORM_FEEDBACK_COUNTER_WRITE_BIT_EXT;
        if (usage & VK_BUFFER_USAGE_ACCELERATION_STRUCTURE_STORAGE_BIT_KHR)
            flags |= VK_ACCESS_2_ACCELERATION_STRUCTURE_WRITE_BIT_KHR;
        return flags;
    }

    VkAccessFlags2 buffer_usage_to_access_flags_second_scope(VkBufferUsage usage)
    {
        VkAccessFlags2 flags = 0;
        if (usage & VK_BUFFER_USAGE_TRANSFER_SRC_BIT)
            flags |= VK_ACCESS_2_TRANSFER_READ_BIT;
        if (usage & VK_BUFFER_USAGE_TRANSFER_DST_BIT)
            flags |= VK_ACCESS_2_TRANSFER_WRITE_BIT;
        if (usage & VK_BUFFER_USAGE_UNIFORM_TEXEL_BUFFER_BIT)
            flags |= VK_ACCESS_2_UNIFORM_READ_BIT;
        if (usage & VK_BUFFER_USAGE_STORAGE_TEXEL_BUFFER_BIT)
            flags |= VK_ACCESS_2_SHADER_STORAGE_READ_BIT | VK_ACCESS_2_SHADER_STORAGE_WRITE_BIT;
        if (usage & VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT)
            flags |= VK_ACCESS_2_UNIFORM_READ_BIT;
        if (usage & VK_BUFFER_USAGE_STORAGE_BUFFER_BIT)
            flags |= VK_ACCESS_2_SHADER_STORAGE_READ_BIT | VK_ACCESS_2_SHADER_STORAGE_WRITE_BIT;
        if (usage & VK_BUFFER_USAGE_INDEX_BUFFER_BIT)
            flags |= VK_ACCESS_2_INDEX_READ_BIT;
        if (usage & VK_BUFFER_USAGE_VERTEX_BUFFER_BIT)
            flags |= VK_ACCESS_2_VERTEX_ATTRIBUTE_READ_BIT;
        if (usage & VK_BUFFER_USAGE_INDIRECT_BUFFER_BIT)
            flags |= VK_ACCESS_2_INDIRECT_COMMAND_READ_BIT;
        if (usage & VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT)
            flags |= VK_ACCESS_2_SHADER_READ_BIT;
        if (usage & VK_BUFFER_USAGE_TRANSFORM_FEEDBACK_BUFFER_BIT_EXT)
            flags |= VK_ACCESS_2_TRANSFORM_FEEDBACK_WRITE_BIT_EXT;
        if (usage & VK_BUFFER_USAGE_TRANSFORM_FEEDBACK_COUNTER_BUFFER_BIT_EXT)
            flags |= VK_ACCESS_2_TRANSFORM_FEEDBACK_COUNTER_READ_BIT_EXT | VK_ACCESS_2_TRANSFORM_FEEDBACK_COUNTER_WRITE_BIT_EXT;
        if (usage & VK_BUFFER_USAGE_CONDITIONAL_RENDERING_BIT_EXT)
            flags |= VK_ACCESS_2_CONDITIONAL_RENDERING_READ_BIT_EXT;
        if (usage & VK_BUFFER_USAGE_ACCELERATION_STRUCTURE_BUILD_INPUT_READ_ONLY_BIT_KHR)
            flags |= VK_ACCESS_2_ACCELERATION_STRUCTURE_READ_BIT_KHR;
        if (usage & VK_BUFFER_USAGE_ACCELERATION_STRUCTURE_STORAGE_BIT_KHR)
            flags |= VK_ACCESS_2_ACCELERATION_STRUCTURE_READ_BIT_KHR | VK_ACCESS_2_ACCELERATION_STRUCTURE_WRITE_BIT_KHR;
        if (usage & VK_BUFFER_USAGE_SHADER_BINDING_TABLE_BIT_KHR)
            flags |= VK_ACCESS_2_SHADER_READ_BIT;
        return flags;
    }

    VkAccessFlags2 image_usage_to_access_flags_first_scope(VkImageUsage usage)
    {
        VkAccessFlags2 flags = 0;
        if (usage & VK_IMAGE_USAGE_TRANSFER_DST_BIT)
            flags |= VK_ACCESS_2_TRANSFER_WRITE_BIT;
        if (usage & VK_IMAGE_USAGE_STORAGE_BIT)
            flags |= VK_ACCESS_2_SHADER_STORAGE_WRITE_BIT;
        if (usage & VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT)
            flags |= VK_ACCESS_2_COLOR_ATTACHMENT_WRITE_BIT;
        if (usage & VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT)
            flags |= VK_ACCESS_2_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
        return flags;
    }

    VkAccessFlags2 image_usage_to_access_flags_second_scope(VkImageUsage usage)
    {
        VkAccessFlags2 flags = 0;
        if (usage & VK_IMAGE_USAGE_TRANSFER_SRC_BIT)
            flags |= VK_ACCESS_2_TRANSFER_READ_BIT;
        if (usage & VK_IMAGE_USAGE_TRANSFER_DST_BIT)
            flags |= VK_ACCESS_2_TRANSFER_WRITE_BIT;
        if (usage & VK_IMAGE_USAGE_SAMPLED_BIT)
            flags |= VK_ACCESS_2_SHADER_SAMPLED_READ_BIT;
        if (usage & VK_IMAGE_USAGE_STORAGE_BIT)
            flags |= VK_ACCESS_2_SHADER_STORAGE_READ_BIT | VK_ACCESS_2_SHADER_STORAGE_WRITE_BIT;
        if (usage & VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT)
            flags |= VK_ACCESS_2_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_2_COLOR_ATTACHMENT_WRITE_BIT;
        if (usage & VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT)
            flags |= VK_ACCESS_2_DEPTH_STENCIL_ATTACHMENT_READ_BIT | VK_ACCESS_2_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
        return flags;
    }

    Buffer create_buffer(const Buffer_info& info, uint32_t initial_queue_family_index,
        VmaAllocator allocator, uint32_t max_frames_in_flight)
    {
        Buffer result = {
            .info = info
        };
        VkBufferCreateInfo buffer_create_info = {
            .sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
            .pNext = nullptr,
            .flags = 0,
            .size = result.info.size,
            .usage = result.info.usage,
            .sharingMode = VK_SHARING_MODE_EXCLUSIVE,
            .queueFamilyIndexCount = 1,
            .pQueueFamilyIndices = &initial_queue_family_index
        };
        VmaAllocationCreateInfo allocation_create_info = {};
        switch (result.info.domain)
        {
        default: assert(false && "Unreachable code.");
        case Buffer_domain::Host_write_combined:
            allocation_create_info.flags =
                VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT |
                VMA_ALLOCATION_CREATE_HOST_ACCESS_ALLOW_TRANSFER_INSTEAD_BIT;
            allocation_create_info.usage = VMA_MEMORY_USAGE_AUTO_PREFER_HOST;
            break;
        case Buffer_domain::Device:
            allocation_create_info.usage = VMA_MEMORY_USAGE_AUTO_PREFER_DEVICE;
            break;
        case Buffer_domain::Device_host_visible:
            allocation_create_info.flags =
                VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT |
                VMA_ALLOCATION_CREATE_HOST_ACCESS_ALLOW_TRANSFER_INSTEAD_BIT;
            allocation_create_info.usage = VMA_MEMORY_USAGE_AUTO_PREFER_DEVICE;
            allocation_create_info.requiredFlags =
                VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT |
                VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT;
            break;
        }
        for (uint32_t i = 0; i < get_allocated_buffer_count(result.info.domain, max_frames_in_flight); i++) {
            auto& allocated_buffer = select_allocated_buffer(result, max_frames_in_flight);
            VmaAllocationInfo allocation_info = {};
            // TODO: add VK_CHECK();
            vmaCreateBuffer(allocator, &buffer_create_info, &allocation_create_info, &allocated_buffer.handle,
                &allocated_buffer.allocation, &allocation_info);
            allocated_buffer.mapped_data = allocation_info.pMappedData;
        }

        return result;
    }

    Image create_image(const Image_info& info, uint32_t initial_queue_family_index, VmaAllocator allocator, VkDevice device)
    {
        Image result = {
            .info = info
        };
        VkImageCreateInfo image_create_info = {
            .sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO,
            .pNext = nullptr,
            .flags = 0,
            .imageType = result.info.type,
            .format = result.info.format,
            .extent = { result.info.width, result.info.height, result.info.depth },
            .mipLevels = result.info.mip_levels,
            .arrayLayers = result.info.array_layers,
            .samples = VK_SAMPLE_COUNT_1_BIT,
            .tiling = VK_IMAGE_TILING_OPTIMAL,
            .usage = result.info.usage,
            .sharingMode = VK_SHARING_MODE_EXCLUSIVE,
            .queueFamilyIndexCount = 1,
            .pQueueFamilyIndices = &initial_queue_family_index,
            .initialLayout = VK_IMAGE_LAYOUT_UNDEFINED
        };
        VmaAllocationCreateInfo allocation_create_info = {
            .usage = VMA_MEMORY_USAGE_AUTO_PREFER_DEVICE
        };
        // TODO: add VK_CHECK();
        vmaCreateImage(allocator, &image_create_info, &allocation_create_info,
            &result.allocated_image.handle, &result.allocated_image.allocation, nullptr);

        VkImageViewCreateInfo image_view_create_info = {
            .sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
            .pNext = nullptr,
            .flags = img_utils::get_default_image_create_flags(result.info),
            .image = result.allocated_image.handle,
            .viewType = img_utils::get_default_image_view_type(result.info),
            .format = result.info.format,
            .components = {
                .r = VK_COMPONENT_SWIZZLE_IDENTITY,
                .g = VK_COMPONENT_SWIZZLE_IDENTITY,
                .b = VK_COMPONENT_SWIZZLE_IDENTITY,
                .a = VK_COMPONENT_SWIZZLE_IDENTITY
            },
            .subresourceRange = img_utils::get_default_image_subresource_range(result.info)
        };
        // TODO: add VK_CHECK();
        vkCreateImageView(device, &image_view_create_info, nullptr, &result.allocated_image.default_view);
        return result;
    }

    void destroy_buffer(Buffer& buffer, VmaAllocator allocator, uint32_t max_frames_in_flight)
    {
        for (uint32_t i = 0; i < get_allocated_buffer_count(buffer.info.domain, max_frames_in_flight); i++) {
            auto& allocated_buffer = buffer.allocated_buffers[i];
            vmaDestroyBuffer(allocator, allocated_buffer.handle, allocated_buffer.allocation);
        }
    }

    void destroy_image(Image& image, VmaAllocator allocator, VkDevice device)
    {
        vkDestroyImageView(device, image.allocated_image.default_view, nullptr);
        vmaDestroyImage(allocator, image.allocated_image.handle, image.allocated_image.allocation);
    }
}
