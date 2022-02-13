// Copyright 2022 Robert Ryan. See Licence.md.

#include "ygg/vulkan/command_buffer_recycler.h"

#include <volk.h>

namespace ygg::vk
{
    Command_buffer_recycler::Command_buffer_recycler(VkDevice device, uint32_t queue_family_index, uint32_t create_flags)
        : m_device(device), m_pool(VK_NULL_HANDLE)
    {
        VkCommandPoolCreateInfo info = {
            .sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
            .pNext = nullptr,
            .flags = create_flags,
            .queueFamilyIndex = queue_family_index,
        };
        // TODO: add VK_CHECK
        vkCreateCommandPool(m_device, &info, nullptr, &m_pool);
    }

    Command_buffer_recycler::~Command_buffer_recycler()
    {
        vkDestroyCommandPool(m_device, m_pool, nullptr);
    }

    VkCommandBuffer Command_buffer_recycler::get_or_allocate()
    {
        if (m_available_command_buffers.size()) {
            auto cmdbuf = m_available_command_buffers.back();
            m_available_command_buffers.pop_back();
            return cmdbuf;
        }
        VkCommandBufferAllocateInfo info = {
            .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
            .pNext = nullptr,
            .commandPool = m_pool,
            .level = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
            .commandBufferCount = 1
        };
        VkCommandBuffer cmdbuf = VK_NULL_HANDLE;
        // TODO: add VK_CHECK
        vkAllocateCommandBuffers(m_device, &info, &cmdbuf);
        return cmdbuf;
    }

    void Command_buffer_recycler::recycle(VkCommandBuffer cmdbuf) noexcept
    {
        m_recycled_command_buffers.push_back(cmdbuf);
    }

    void Command_buffer_recycler::reset(uint32_t flags)
    {
        // TODO: add VK_CHECK
        vkResetCommandPool(m_device, m_pool, flags);
        m_available_command_buffers.insert(
            m_recycled_command_buffers.begin(),
            m_recycled_command_buffers.end(),
            m_recycled_command_buffers.end()
        );
        m_recycled_command_buffers.clear();
    }
}
