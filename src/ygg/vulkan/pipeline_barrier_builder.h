// Copyright 2022 Robert Ryan. See Licence.md.

#pragma once

#include "ygg/vulkan/vk_forward_decl.h"

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
        ~Pipeline_barrier_builder();

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
         * @brief Stores a buffer memory barrier specifying a queue ownership transfer, to be executed on `flush()`.
         * @details QFOT works by pushing an image barrier from the src queue family specifying the dst queue family,
         * submitting the barriers and on the importing target specifying this barrier. The size and offset in both
         * barriers *must* match. Waiting on pipeline stage flags or access flags is not required as that should be
         * done with the semaphore specifying the dependency between both submits.
         * @return This instance.
        */
        Pipeline_barrier_builder& push_buffer_qfot_import_memory_barrier(VkBuffer buffer, VkDeviceSize offset, VkDeviceSize size);

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
         * @brief Stores a image memory barrier specifying a queue ownership transfer, to be executed on `flush()`.
         * @details QFOT works by pushing an image barrier from the src queue family specifying the dst queue family,
         * submitting the barriers and on the importing target specifying this barrier. The layouts used in both barriers
         * *must* match. Waiting on pipeline stage flags or access flags is not required as that should be done with the
         * semaphore specifying the dependency between both submits.
         * @return This instance.
        */
        Pipeline_barrier_builder& push_image_qfot_import_memory_barrier(VkImageLayout old_layout, VkImageLayout new_layout,
            uint32_t src_queue_family_index, uint32_t dst_queue_family_index,
            VkImage image, const VkImageSubresourceRange& subresource_range);

        /**
         * @brief Flushes the barriers to be executed by the given command buffer.
         * @param dependency_flags The VkDepdendencyFlags to be set for this `vkCmdPipelineBarrier2`.
        */
        void flush(VkDependencyFlags dependency_flags);
    private:
        VkCommandBuffer m_cmdbuf;
        struct Barrier_store;
        Barrier_store* m_barrier_store;
    };
}
