// Copyright 2022 Robert Ryan. See Licence.md.

#pragma once

#include "ygg/vulkan/compute_command_buffer.h"

#include <optional>

namespace ygg::vk
{
    /**
     * @brief Base structure for draw_indirect commands.
    */
    struct Draw_indirect_command
    {
        uint32_t vertex_count;
        uint32_t instance_count;
        uint32_t first_vertex;
        uint32_t first_instance;
    };

    /**
     * @brief Base structure for draw_indexed_indirect commands.
    */
    struct Draw_indexed_indirect_command
    {
        uint32_t index_count;
        uint32_t instance_count;
        uint32_t first_index;
        int32_t vertex_offset;
        uint32_t first_instance;
    };

    /**
     * @brief https://www.khronos.org/registry/vulkan/specs/1.3-extensions/man/html/VkImageBlit2.html
    */
    struct Image_blit_info
    {
        struct Offset_3d
        {
            int32_t x;
            int32_t y;
            int32_t z;
        };
        uint32_t src_mip_level;
        uint32_t src_base_array_layer;
        uint32_t src_layer_count;
        Offset_3d src_offsets[2];
        uint32_t dst_mip_level;
        uint32_t dst_base_array_layer;
        uint32_t dst_layer_count;
        Offset_3d dst_offsets[2];
        VkFilter filter;
        VkImageAspectFlags aspect_mask;
    };

    /**
     * @brief https://www.khronos.org/registry/vulkan/specs/1.3-extensions/man/html/VkRenderingInfo.html
    */
    struct Rendering_info
    {
        struct Attachment_info
        {
            VkImageView view;
            VkImageLayout layout;
            VkAttachmentLoadOp load_op;
            VkAttachmentStoreOp store_op;
            Clear_value clear_value;
        };
        int32_t offset_x;
        int32_t offset_y;
        uint32_t width;
        uint32_t height;
        std::optional<std::span<Attachment_info>> color_attachments;
        std::optional<Attachment_info> depth_attachment;
        std::optional<Attachment_info> stencil_attachment;
    };

    /**
     * @brief A wrapper around a given `VkCommandBuffer` capable of transfer, compute and graphics commands.
    */
    class Graphics_command_buffer : public Compute_command_buffer
    {
    public:
        using Compute_command_buffer::Compute_command_buffer;

        /**
         * @brief https://www.khronos.org/registry/vulkan/specs/1.3-extensions/man/html/vkCmdBindIndexBuffer.html
        */
        void bind_index_buffer(const Buffer& buffer, VkDeviceSize offset, VkIndexType type) const;

        /**
         * @brief https://www.khronos.org/registry/vulkan/specs/1.3-extensions/man/html/vkCmdBeginRendering.html
        */
        void begin_rendering(const Rendering_info& info) const;

        /**
         * @brief https://www.khronos.org/registry/vulkan/specs/1.3-extensions/man/html/vkCmdBlitImage2.html
        */
        void blit(const Image& src, VkImageLayout src_layout, const Image& dst, VkImageLayout dst_layout,
            const Image_blit_info& blit_info) const;

        /**
         * @brief https://www.khronos.org/registry/vulkan/specs/1.3-extensions/man/html/vkCmdDraw.html
        */
        void draw(uint32_t vertex_count, uint32_t instance_count = 1,
            uint32_t first_vertex = 0, uint32_t first_instance = 0) const;

        /**
         * @brief https://www.khronos.org/registry/vulkan/specs/1.3-extensions/man/html/vkCmdDrawIndexed.html
        */
        void draw_indexed(uint32_t index_count, uint32_t instance_count = 1,
            uint32_t first_index = 0, uint32_t vertex_offset = 0, uint32_t first_instance = 0) const;

        /**
         * @brief https://www.khronos.org/registry/vulkan/specs/1.3-extensions/man/html/vkCmdDrawIndexedIndirect.html
        */
        void draw_indexed_indirect(const Buffer& buffer, VkDeviceSize buffer_offset,
            uint32_t draw_count, uint32_t stride) const;

        /**
         * @brief https://www.khronos.org/registry/vulkan/specs/1.3-extensions/man/html/vkCmdDrawIndexedIndirectCount.html
        */
        void draw_indexed_indirect_count(const Buffer& buffer, VkDeviceSize buffer_offset, const Buffer& count_buffer,
            VkDeviceSize count_buffer_offset, uint32_t max_draw_count, uint32_t stride) const;

        /**
         * @brief https://www.khronos.org/registry/vulkan/specs/1.3-extensions/man/html/vkCmdDrawIndirect.html
        */
        void draw_indirect(const Buffer& buffer, VkDeviceSize buffer_offset, uint32_t draw_count, uint32_t stride) const;

        /**
         * @brief https://www.khronos.org/registry/vulkan/specs/1.3-extensions/man/html/vkCmdDrawIndirectCount.html
        */
        void draw_indirect_count(const Buffer& buffer, VkDeviceSize buffer_offset, const Buffer& count_buffer,
            VkDeviceSize count_buffer_offset, uint32_t max_draw_count, uint32_t stride) const;

        /**
         * @brief https://www.khronos.org/registry/vulkan/specs/1.3-extensions/man/html/vkCmdEndRendering.html
        */
        void end_rendering() const;

        /**
         * @brief https://www.khronos.org/registry/vulkan/specs/1.3-extensions/man/html/vkCmdSetScissor.html
        */
        void set_scissor(int32_t offset_x, int32_t offset_y, uint32_t width, uint32_t height) const;

        /**
         * @brief https://www.khronos.org/registry/vulkan/specs/1.3-extensions/man/html/vkCmdSetViewport.html
        */
        void set_viewport(float_t x, float_t y, float_t width, float_t height,
            float_t min_depth = 0.0f, float_t max_depth = 1.0f) const;
    };
}
