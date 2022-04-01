// Copyright 2022 Robert Ryan. See Licence.md.

#pragma once

#include "ygg/vulkan/vk_forward_decl.h"

#include <array>

namespace ygg::vk
{
    /**
     * @brief Memory domain in which a buffer resides in.
     * @details Buffers with the domain `Device_host_visible` will be buffered
     * by the amount of max frames in flight. All other domains are single-buffered.
    */
    enum class Buffer_domain
    {
        Host_write_combined, // Maps to HOST_VISIBLE | HOST_COHERENT
        Device,              // Maps to DEVICE_LOCAL
        Device_host_visible  // Maps to DEVICE_LOCAL | HOST_VISIBLE | HOST_COHERENT
    };

    /**
     * @brief A single allocation for a buffer.
    */
    struct Allocated_buffer
    {
        VkBuffer handle;
        VmaAllocation allocation;
        void* mapped_data;
    };

    /**
     * @brief Information about a buffer, used for queries and buffer creation.
    */
    struct Buffer_info
    {
        Buffer_domain domain;
        VkDeviceSize size;
        VkBufferUsage usage;
    };

    /**
     * @brief Buffer "primitive".
    */
    struct Buffer
    {
        std::array<Allocated_buffer, YGG_MAX_FRAMES_IN_FLIGHT> allocated_buffers;
        Buffer_info info;
    };

    /**
     * @brief Selects the buffer allocation based on the buffers `Buffer_domain` and current
     * frame in flight.
     * @return The selected `Allocated_buffer`, which can bei either the first one or the one
     * from the current frame in flight.
    */
    Allocated_buffer& select_allocated_buffer(Buffer& buffer, uint32_t current_frame_in_flight);

    /**
     * @brief Queries the count of `Allocated_buffer`s used in a given buffer domain.
     * @return `max_frames_in_flight` for `Buffer_domain::Device_host_visible`, `1` otherwise.
    */
    uint32_t get_allocated_buffer_count(Buffer_domain domain, uint32_t max_frames_in_flight);

    /**
     * @brief A single allocation for an Image.
    */
    struct Allocated_image
    {
        VkImage handle;
        VkImageView default_view;
        VmaAllocation allocation;
    };

    /**
     * @brief Information required to create an Image.
    */
    struct Image_info
    {
        uint32_t width;
        uint32_t height;
        uint32_t depth;
        uint32_t mip_levels;
        uint32_t array_layers;
        VkFormat format;
        VkImageUsage usage;
        VkImageType type;
    };

    /**
     * @brief Image "primitive".
    */
    struct Image
    {
        Allocated_image allocated_image;
        Image_info info;
    };

    /**
     * @brief Conservatively selects `VkAccessFlagBits2` based on the provided `VkBufferUsage`.
     * @return The combination of any `VkAccessFlagBits2` value that can be derived from the
     * `VkBufferUsage` that is usable inside the first synchronization scope of a `VkBufferMemoryBarrier2`.
    */
    VkAccessFlags2 buffer_usage_to_access_flags_first_scope(VkBufferUsage usage);

    /**
     * @brief Conservatively selects `VkAccessFlagBits2` based on the provided `VkImageUsage`.
     * @return The combination of any `VkAccessFlagBits2` value that can be derived from the
     * `VkImageUsage` that is usable inside the first synchronization scope of a `VkImageMemoryBarrier2`.
    */
    VkAccessFlags2 image_usage_to_access_flags_first_scope(VkImageUsage usage);

    /**
     * @brief Conservatively selects `VkAccessFlagBits2` based on the provided `VkBufferUsage`.
     * @return The combination of any `VkAccessFlagBits2` value that can be derived from the
     * `VkBufferUsage` that is usable inside the second synchronization scope of a `VkBufferMemoryBarrier2`.
    */
    VkAccessFlags2 buffer_usage_to_access_flags_second_scope(VkBufferUsage usage);

    /**
     * @brief Conservatively selects `VkAccessFlagBits2` based on the provided `VkImageUsage`.
     * @return The combination of any `VkAccessFlagBits2` value that can be derived from the
     * `VkImageUsage` that is usable inside the second synchronization scope of a `VkImageMemoryBarrier2`.
    */
    VkAccessFlags2 image_usage_to_access_flags_second_scope(VkImageUsage usage);

    /**
     * @brief Constructs an `Buffer` instance with the given information.
     * The returned instance will be owned by the passed `VmaAllocator` and reside in the
     * queue family index it was created in.
     * @return The constructed instance.
    */
    Buffer create_buffer(const Buffer_info& info, uint32_t initial_queue_family_index,
        VmaAllocator allocator, uint32_t max_frames_in_flight);

    /**
     * @brief Constructs an `Image` instance with the given information.
     * The returned instance will be owned by the passed `VmaAllocator` and `VkDevice`
     * and reside in the queue family index it was created in.
     * @return The constructed instance.
    */
    Image create_image(const Image_info& info, uint32_t initial_queue_family_index,
        VmaAllocator allocator, VkDevice device);

    /**
     * @brief Destroys a `Buffer` instance. If the buffer wasn't created, this call is UB.
     * @detail The passed `VmaAllocator` and `max_frames_in_flight` must be the same ones
     * that were used for creating the passed `Buffer`.
    */
    void destroy_buffer(Buffer& buffer, VmaAllocator allocator, uint32_t max_frames_in_flight);

    /**
     * @brief Destroys an `Image` instance. If the image wasn't created, this call is UB.
     * @detail The passed `VmaAllocator` and `VkDevice` must be the same ones that were
     * used for creating the passed `Image`.
     * @param image The `Image` instance to destroy.
     * @param allocator The allocator used to create the `Image` instance with.
     * @param device The device that owns the `Image` instance.
    */
    void destroy_image(Image& image, VmaAllocator allocator, VkDevice device);
}
