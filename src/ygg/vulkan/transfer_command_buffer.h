// Copyright 2022 Robert Ryan. See Licence.md.

#pragma once

#include "ygg/vulkan/pipeline_barrier_builder.h"
#include "ygg/vulkan/vk_forward_decl.h"

#include <span>

namespace ygg::vk
{
    class Command_buffer_recycler;
    class Linear_host_resource_allocator;
    struct Buffer;
    struct Image;

    /**
     * @brief Parameter list for buffer-image and image-buffer copies.
    */
    struct Buffer_image_copy_info
    {
        uint32_t buffer_offset = 0;
        uint32_t buffer_row_length = 0;
        uint32_t buffer_image_height = 0;
        uint32_t image_mip_level;
        uint32_t image_base_array_layer = 0;
        uint32_t image_layer_count = 1;
        int32_t image_offset_x = 0;
        int32_t image_offset_y = 0;
        int32_t image_offset_z = 0;
        uint32_t image_width;
        uint32_t image_height;
        uint32_t image_depth = 1;
    };

    /**
     * @brief Parameter list for image-image copies.
    */
    struct Image_copy_info
    {
        uint32_t src_mip_level;
        uint32_t src_base_array_layer = 0;
        uint32_t src_layer_count = 1;
        int32_t src_offset_x = 0;
        int32_t src_offset_y = 0;
        int32_t src_offset_z = 0;
        uint32_t dst_mip_level;
        uint32_t dst_base_array_layer = 0;
        uint32_t dst_layer_count = 1;
        int32_t dst_offset_x = 0;
        int32_t dst_offset_y = 0;
        int32_t dst_offset_z = 0;
        int32_t width;
        int32_t height;
        int32_t depth = 1;
    };

    /**
     * @brief A wrapper around a given `VkCommandBuffer` only capable of transfer commands.
    */
    class Transfer_command_buffer
    {
    public:
        /**
         * @brief Creates and binds the wrapper instance to the given `VkCommandBuffer`.
         * @param cmdbuf The `VkCommandBuffer` to bind this wrapper to.
         * @param allocator A non-owned allocator, used by the `upload` functions.
         * @param frame_in_flight The amount of frames in flight, used by the `upload` functions.
         * @param queue_family_index The queue family index used by the passed `Linear_host_resource_allocator`.
        */
        Transfer_command_buffer(VkCommandBuffer cmdbuf, Linear_host_resource_allocator& allocator,
            uint32_t frame_in_flight, uint32_t queue_family_index);

        /**
         * @brief Returns the bound `Pipeline_barrier_builder` instance.
         * @return The `Pipeline_barrier_builder` associated with this command buffer.
        */
        inline Pipeline_barrier_builder& pipeline_barrier_builder() { return m_pipeline_barrier_builder; }

        /**
         * @brief Uploads provided data to a buffer.
        */
        void upload_buffer_data(Buffer& buffer, void* data, VkDeviceSize size, VkDeviceSize offset);

        /**
         * @brief Uploads provided data to an image.
         * @details The caller is responsible for transitioning the image into `VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL`
         * prior to calling this function. uploading data to a depth-only image is not supported with
         * this command.
        */
        void upload_image_data(Image& image, void* data, uint32_t width, uint32_t height, uint32_t depth,
            uint32_t width_offset, uint32_t height_offset, uint32_t depth_offset, uint32_t layers,
            uint32_t base_layer, uint32_t mip_level);

        /**
         * @brief Allocates a staging region and copies the contents from the region to the buffer.
         * @details This command is a *NOOP* for buffers that are not residing in the `Device` domain.
         * @return The mapped pointer to the allocation to be filled with data for the upload.
        */
        void* allocate_and_upload_buffer_data(Buffer& buffer, VkDeviceSize size, VkDeviceSize offset);

        /**
         * @brief Allocates a staging region and copies the contents from the region to the image.
         * @details The caller is responsible for transitioning the image into `VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL`
         * prior to calling this function. uploading data to a depth-only image is not supported with
         * this command.
         * @return The mapped pointer to the allocation to be filled with data for the upload.
        */
        void* allocate_and_upload_image_data(Image& image, uint32_t width, uint32_t height, uint32_t depth,
            uint32_t width_offset, uint32_t height_offset, uint32_t depth_offset, uint32_t layers,
            uint32_t base_layer, uint32_t mip_level);

        /**
         * The further commands are wrapped Vulkan commands using the given command buffer.
         * See the Vulkan spec for their explanation, which is linked for each command.
        */

        /**
         * @brief https://www.khronos.org/registry/vulkan/specs/1.3-extensions/man/html/vkCmdCopyBuffer2KHR.html
         */
        void copy_buffer(const Buffer& src, const Buffer& dst,
            VkDeviceSize src_offset, VkDeviceSize dst_offset, VkDeviceSize size) const;

        /**
         * @brief https://www.khronos.org/registry/vulkan/specs/1.3-extensions/man/html/vkCmdCopyBufferToImage2.html
        */
        void copy_buffer_to_image(const Buffer& src, const Image& dst,
            VkImageLayout dst_layout, const Buffer_image_copy_info& info) const;

        void copy_buffer_to_image(const Buffer& src, const Image& dst,
            VkImageLayout dst_layout, VkImageAspectFlags dst_aspect, const Buffer_image_copy_info& info) const;

        /**
         * @brief https://www.khronos.org/registry/vulkan/specs/1.3-extensions/man/html/vkCmdCopyImageToBuffer2.html
        */
        void copy_image_to_buffer(const Image& src, const Buffer& dst,
            VkImageLayout src_layout, const Buffer_image_copy_info& info) const;

        void copy_image_to_buffer(const Image& src, const Buffer& dst,
            VkImageLayout src_layout, VkImageAspectFlags src_aspect, const Buffer_image_copy_info& info) const;

        /**
         * @brief https://www.khronos.org/registry/vulkan/specs/1.3-extensions/man/html/vkCmdCopyImage2.html
        */
        void copy_image(const Image& src, const Image& dst, VkImageLayout src_layout,
            VkImageLayout dst_layout, const Image_copy_info& info) const;

        void copy_image(const Image& src, const Image& dst, VkImageLayout src_layout,
            VkImageLayout dst_layout, const Image_copy_info& info,
            VkImageAspectFlags src_aspect, VkImageAspectFlags dst_aspect) const;

    private:
        Linear_host_resource_allocator& m_allocator;
        VkCommandBuffer m_cmdbuf;
        Pipeline_barrier_builder m_pipeline_barrier_builder;
        uint32_t m_frame_in_flight;
        uint32_t m_queue_family_index;
    };
}
