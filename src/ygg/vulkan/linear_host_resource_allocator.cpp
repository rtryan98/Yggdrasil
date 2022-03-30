// Copyright 2022 Robert Ryan. See Licence.md.

#include "ygg/vulkan/linear_host_resource_allocator.h"

#include <volk.h>
#include <vk_mem_alloc.h>

namespace ygg::vk
{
    Linear_host_resource_allocator::Linear_host_resource_allocator(VmaAllocator allocator)
        : m_allocator(allocator), m_pool(VK_NULL_HANDLE), m_allocated_buffers()
    {
        uint32_t memory_type_index;
        VmaAllocationCreateInfo allocation_create_info = {
            .usage = VMA_MEMORY_USAGE_CPU_ONLY
        };
        vmaFindMemoryTypeIndex(m_allocator, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
            &allocation_create_info, &memory_type_index);
        VmaPoolCreateInfo pool_info = {
            .memoryTypeIndex = memory_type_index,
            .flags = VMA_POOL_CREATE_LINEAR_ALGORITHM_BIT |
                VMA_POOL_CREATE_IGNORE_BUFFER_IMAGE_GRANULARITY_BIT
        };
        vmaCreatePool(m_allocator, &pool_info, &m_pool);
    }

    Linear_host_resource_allocator::~Linear_host_resource_allocator()
    {
        reset();
        vmaDestroyPool(m_allocator, m_pool);
    }

    Linear_host_resource_allocator::Mapped_host_buffer& Linear_host_resource_allocator::allocate_buffer(VkDeviceSize size,
        uint32_t queue_family_index)
    {
        auto& result = m_allocated_buffers.emplace_back();
        VkBufferCreateInfo buffer_info = {
            .sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
            .pNext = nullptr,
            .size = size,
            .usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
            .sharingMode = VK_SHARING_MODE_EXCLUSIVE,
            .queueFamilyIndexCount = 1,
            .pQueueFamilyIndices = &queue_family_index
        };
        VmaAllocationCreateInfo allocation_create_info = {
            .flags = VMA_ALLOCATION_CREATE_MAPPED_BIT,
            .pool = m_pool
        };
        VmaAllocationInfo allocation_info = {};
        vmaCreateBuffer(m_allocator, &buffer_info, &allocation_create_info, &result.buf,
            &result.allocation, &allocation_info);
        result.mapped_data = allocation_info.pMappedData;
        return result;
    }

    void Linear_host_resource_allocator::reset()
    {
        for (auto& buf : m_allocated_buffers)
            vmaDestroyBuffer(m_allocator, buf.buf, buf.allocation);
        m_allocated_buffers.clear();
    }

    Linear_host_resource_allocator_provider::Linear_host_resource_allocator_provider(VmaAllocator allocator)
        : m_allocator(allocator), m_allocators()
    {}

    Linear_host_resource_allocator_provider::~Linear_host_resource_allocator_provider()
    {
        reset();
    }

    Linear_host_resource_allocator& Linear_host_resource_allocator_provider::create_allocator()
    {
        return m_allocators.emplace_back(Linear_host_resource_allocator(m_allocator));
    }

    void Linear_host_resource_allocator_provider::reset()
    {
        m_allocators.clear();
    }
}
