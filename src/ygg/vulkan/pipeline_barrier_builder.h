// Copyright 2022 Robert Ryan. See Licence.md.

#pragma once

#include <vector>
#include <volk.h>

namespace ygg::vk
{
    /**
     * @brief Lightweight builder to improve pipeline barrier usability.
    */
    class Pipeline_barrier_builder
    {
    public:
        /**
         * @brief Constructs a `Pipeline_barrier_builder` instance exclusively for the given command buffer.
         * @param cmdbuf The command buffer to which to bind this instance.
        */
        explicit Pipeline_barrier_builder(VkCommandBuffer cmdbuf);

        /**
         * @brief Stores a memory barrier, to be executed on `flush()`.
         * @return This instance.
        */
        Pipeline_barrier_builder& push_memory_barrier(VkPipelineStageFlags2 src_stage_mask,
            VkAccessFlags2 src_access_mask, VkPipelineStageFlags2 dst_stage_mask, VkAccessFlags2 dst_access_mask);

        /**
         * @brief Stores a buffer memory barrier, to be executed on `flush()`.
         * @return This instance.
        */
        Pipeline_barrier_builder& push_buffer_memory_barrier(VkPipelineStageFlags2 src_stage_mask, VkAccessFlags2 src_access_mask,
            VkPipelineStageFlags2 dst_stage_mask, VkAccessFlags2 dst_access_mask, uint32_t src_queue_family_index,
            uint32_t dst_queue_family_index, VkBuffer buffer, VkDeviceSize offset, VkDeviceSize size);

        /**
         * @brief Stores a buffer memory barrier, to be executed on `flush()`.
         * @return This instance.
        */
        Pipeline_barrier_builder& push_buffer_memory_barrier(VkPipelineStageFlags2 src_stage_mask, VkAccessFlags2 src_access_mask,
            VkPipelineStageFlags2 dst_stage_mask, VkAccessFlags2 dst_access_mask,
            VkBuffer buffer, VkDeviceSize offset, VkDeviceSize size);

        /**
         * @brief Stores a image memory barrier, to be executed on `flush()`.
         * @return This instance.
        */
        Pipeline_barrier_builder& push_image_memory_barrier(VkPipelineStageFlags2 src_stage_mask,
            VkAccessFlags2 src_access_mask, VkPipelineStageFlags2 dst_stage_mask, VkAccessFlags2 dst_access_mask,
            VkImageLayout old_layout, VkImageLayout new_layout, uint32_t src_queue_family_index, uint32_t dst_queue_family_index,
            VkImage image, const VkImageSubresourceRange& subresource_range);

        /**
         * @brief Stores a image memory barrier, to be executed on `flush()`.
         * @return This instance.
        */
        Pipeline_barrier_builder& push_image_memory_barrier(VkPipelineStageFlags2 src_stage_mask,
            VkAccessFlags2 src_access_mask, VkPipelineStageFlags2 dst_stage_mask, VkAccessFlags2 dst_access_mask,
            VkImageLayout old_layout, VkImageLayout new_layout, VkImage image, const VkImageSubresourceRange& subresource_range);

        /**
         * @brief Flushes the barriers to be executed by the given command buffer.
        */
        void flush(VkDependencyFlags dependency_flags = VK_DEPENDENCY_BY_REGION_BIT);
    private:
        VkCommandBuffer m_cmdbuf;
        std::vector<VkMemoryBarrier2> m_memory_barriers;
        std::vector<VkBufferMemoryBarrier2> m_buffer_memory_barriers;
        std::vector<VkImageMemoryBarrier2> m_image_memory_barriers;
    };
}
