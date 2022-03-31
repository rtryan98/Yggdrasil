// Copyright 2022 Robert Ryan. See Licence.md.

#include "ygg/vulkan/pipeline_barrier_builder.h"

namespace ygg::vk
{
    Pipeline_barrier_builder::Pipeline_barrier_builder(VkCommandBuffer cmdbuf)
        : m_cmdbuf(cmdbuf), m_memory_barriers(), m_buffer_memory_barriers(), m_image_memory_barriers()
    {}

    Pipeline_barrier_builder& Pipeline_barrier_builder::push_memory_barrier(VkPipelineStageFlags2 src_stage_mask,
        VkAccessFlags2 src_access_mask, VkPipelineStageFlags2 dst_stage_mask, VkAccessFlags2 dst_access_mask)
    {
        m_memory_barriers.push_back({ VK_STRUCTURE_TYPE_MEMORY_BARRIER_2, nullptr,
            src_stage_mask, src_access_mask, dst_stage_mask, dst_access_mask });
        return *this;
    }

    Pipeline_barrier_builder& Pipeline_barrier_builder::push_buffer_memory_barrier(VkPipelineStageFlags2 src_stage_mask,
        VkAccessFlags2 src_access_mask, VkPipelineStageFlags2 dst_stage_mask, VkAccessFlags2 dst_access_mask,
        uint32_t src_queue_family_index, uint32_t dst_queue_family_index, VkBuffer buffer, VkDeviceSize offset, VkDeviceSize size)
    {
        m_buffer_memory_barriers.push_back({ VK_STRUCTURE_TYPE_BUFFER_MEMORY_BARRIER_2, nullptr,
            src_stage_mask, src_access_mask, dst_stage_mask, dst_access_mask, src_queue_family_index, dst_queue_family_index,
            buffer, offset, size });
        return *this;
    }

    Pipeline_barrier_builder& Pipeline_barrier_builder::push_buffer_memory_barrier(VkPipelineStageFlags2 src_stage_mask,
        VkAccessFlags2 src_access_mask, VkPipelineStageFlags2 dst_stage_mask, VkAccessFlags2 dst_access_mask,
        VkBuffer buffer, VkDeviceSize offset, VkDeviceSize size)
    {
        return push_buffer_memory_barrier(src_stage_mask, src_access_mask, dst_stage_mask, dst_access_mask,
            VK_QUEUE_FAMILY_IGNORED, VK_QUEUE_FAMILY_IGNORED, buffer, offset, size);
    }

    Pipeline_barrier_builder& Pipeline_barrier_builder::push_image_memory_barrier(VkPipelineStageFlags2 src_stage_mask,
        VkAccessFlags2 src_access_mask, VkPipelineStageFlags2 dst_stage_mask, VkAccessFlags2 dst_access_mask, VkImageLayout old_layout,
        VkImageLayout new_layout, uint32_t src_queue_family_index, uint32_t dst_queue_family_index,
        VkImage image, const VkImageSubresourceRange& subresource_range)
    {
        m_image_memory_barriers.push_back({ VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER_2, nullptr,
            src_stage_mask, src_access_mask, dst_stage_mask, dst_access_mask, old_layout, new_layout,
            src_queue_family_index, dst_queue_family_index, image, subresource_range });
        return *this;
    }

    Pipeline_barrier_builder& Pipeline_barrier_builder::push_image_memory_barrier(VkPipelineStageFlags2 src_stage_mask,
        VkAccessFlags2 src_access_mask, VkPipelineStageFlags2 dst_stage_mask, VkAccessFlags2 dst_access_mask,
        VkImageLayout old_layout, VkImageLayout new_layout, VkImage image, const VkImageSubresourceRange& subresource_range)
    {
        return push_image_memory_barrier(src_stage_mask, src_access_mask, dst_stage_mask, dst_access_mask, old_layout, new_layout,
            VK_QUEUE_FAMILY_IGNORED, VK_QUEUE_FAMILY_IGNORED, image, subresource_range);
    }

    void Pipeline_barrier_builder::flush(VkDependencyFlags dependency_flags)
    {
        VkDependencyInfo dependency_info = {
            .sType = VK_STRUCTURE_TYPE_DEPENDENCY_INFO,
            .pNext = nullptr,
            .dependencyFlags = dependency_flags,
            .memoryBarrierCount = static_cast<uint32_t>(m_memory_barriers.size()),
            .pMemoryBarriers = m_memory_barriers.data(),
            .bufferMemoryBarrierCount = static_cast<uint32_t>(m_buffer_memory_barriers.size()),
            .pBufferMemoryBarriers = m_buffer_memory_barriers.data(),
            .imageMemoryBarrierCount = static_cast<uint32_t>(m_image_memory_barriers.size()),
            .pImageMemoryBarriers = m_image_memory_barriers.data()
        };
        if (m_memory_barriers.size() || m_buffer_memory_barriers.size() ||m_image_memory_barriers.size()) {
            vkCmdPipelineBarrier2(m_cmdbuf, &dependency_info);
        }
        m_memory_barriers.clear();
        m_buffer_memory_barriers.clear();
        m_image_memory_barriers.clear();
    }
}
