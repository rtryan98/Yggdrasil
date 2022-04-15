// Copyright 2022 Robert Ryan. See Licence.md.

#pragma once

#include "ygg/vulkan/vk_forward_decl.h"

#include <memory>
#include <vector>

namespace ygg::vk
{
    /**
     * @brief Fast Buffer-allocator for transient resource uploads.
    */
    class Linear_host_resource_allocator
    {
    public:
        /**
         * @brief Single-time allocated buffer residing in write-combined host memory.
        */
        struct Mapped_host_buffer
        {
            VkBuffer buf;
            VmaAllocation allocation;
            void* mapped_data;
        };

    public:
        /**
         * @brief Creates a `Linear_host_resource_allocator` instance using the given `VmaAllocator`.
         * @param allocator The `VmaAllocator` to use.
        */
        explicit Linear_host_resource_allocator(VmaAllocator allocator);
        ~Linear_host_resource_allocator();

        /**
         * @brief Allocates a buffer which is write-combined and mapped.
         * @details The returned buffer should not be read from. Any read from this buffer may be
         * extremely slow and cause heavy performance penalties. This also goes for indirect reads
         * such as using `+=` or similar on the elements inside.
         * @param size The size of the allocation.
         * @param queue_family_index The queue family index to which this allocated buffer belongs to.
         * @return Transient mapped allocation, to be freed on `reset()`.
        */
        Mapped_host_buffer& allocate_buffer(VkDeviceSize size, uint32_t queue_family_index);

        /**
         * @brief Frees all buffers allocated from this instance.
        */
        void reset();

    private:
        VmaAllocator m_allocator;
        VmaPool m_pool;
        std::vector<Mapped_host_buffer> m_allocated_buffers;
    };

    /**
     * @brief Factory to instantiate and manage 'Linear_host_resource_allocator' instances.
    */
    class Linear_host_resource_allocator_provider
    {
    public:
        /**
         * @brief Creates a `Linear_host_resource_allocator_provider` instance using the given `VmaAllocator`.
         * @param allocator The `VmaAllocator` to use for every created `Linear_host_resource_allocator` instance
         * created by this instance.
        */
        explicit Linear_host_resource_allocator_provider(VmaAllocator allocator);
        ~Linear_host_resource_allocator_provider();

        /**
         * @brief Creates a `Linear_host_resource_allocator` instance with the `VmaAllocator` of this instance.
         * @return The non-owned `Linear_host_resource_allocator` instance.
        */
        Linear_host_resource_allocator& create_allocator();

        /**
         * @brief Resets this provider, destroying all provided `Linear_host_resource_allocator` instances
         * created from this instance.
        */
        void reset();
    private:
        VmaAllocator m_allocator;
        std::vector<std::unique_ptr<Linear_host_resource_allocator>> m_allocators;
    };
}
