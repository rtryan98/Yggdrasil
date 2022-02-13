// Copyright 2022 Robert Ryan. See Licence.md.

#pragma once

#include "ygg/vulkan/vk_forward_decl.h"

#include <vector>

namespace ygg::vk
{
    /**
     * @brief VkCommandPool-owning utility class for creating recyclable VkCommandbuffer instances.
     * @details This recycler does not allow for creating secondary command buffers - prefer using
     * multiple submits instead and synchronize via timeline semaphore.
    */
    class Command_buffer_recycler
    {
    public:
        /**
         * @brief Constructs a `command_buffer_recycler` instance, bound to the queue family.
         * @param device The owning VkDevice.
         * @param queue_family_index The index of the queue family.
         * @param create_flags Additional flags as specified by VkCommandPoolCreateFlags.
        */
        Command_buffer_recycler(VkDevice device, uint32_t queue_family_index, uint32_t create_flags);

        ~Command_buffer_recycler();

        /**
         * @brief Retrieve a primary VkCommandBuffer.
         * @details This function must be externally synchronized.
         * @return The VkCommandBuffer.
        */
        [[nodiscard]] VkCommandBuffer get_or_allocate();

        /**
         * @brief Recycles a VkCommandBuffer back into this recycler.
         * @details A recycled VkCommandBuffer can still be used for submission.
         * The command buffer is made unusable as soon as `reset` is called.
         * It is undefined behavior if a command buffer that is not owned by this recycler is recycled.
         * As such, the caller must ensure that the passed command buffer was acquired from the same instance.
         * This function must be externally synchronized.
         * @param cmdbuf The command buffer to recycle.
        */
        void recycle(VkCommandBuffer cmdbuf) noexcept;

        /**
         * @brief Resets the underlying VkCommandPool and flushes pending recyclces.
         * @details This function must be externally synchronized.
         * @param flags Flags as specified by VkCommandPoolResetFlagBits.
        */
        void reset(uint32_t flags = 0);

    private:
        VkDevice m_device;
        VkCommandPool m_pool;

        std::vector<VkCommandBuffer> m_available_command_buffers = {};
        std::vector<VkCommandBuffer> m_recycled_command_buffers = {};
    };
}
