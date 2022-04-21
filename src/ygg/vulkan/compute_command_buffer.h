// Copyright 2022 Robert Ryan. See Licence.md.

#pragma once

#include "ygg/vulkan/transfer_command_buffer.h"

#include <span>

namespace ygg::vk
{
    struct Pipeline;

    /**
     * @brief https://www.khronos.org/registry/vulkan/specs/1.3-extensions/man/html/VkClearValue.html
    */
    union Clear_value
    {
        union
        {
            float_t f32[4];
            int32_t i32[4];
            uint32_t u32[4];
        } color;
        struct
        {
            float_t depth;
            uint32_t stencil;
        } depth_stencil;
    };

    /**
     * @brief A wrapper around a given `VkCommandBuffer` capable of transfer and compute commands.
    */
    class Compute_command_buffer : public Transfer_command_buffer
    {
    public:
        using Transfer_command_buffer::Transfer_command_buffer;

        /**
         * @brief https://www.khronos.org/registry/vulkan/specs/1.3-extensions/man/html/vkCmdBindDescriptorSets.html
        */
        void bind_descriptor_set(VkPipelineBindPoint bind_point, VkPipelineLayout layout,
            uint32_t set_offset, VkDescriptorSet set) const;
        void bind_descriptor_sets(VkPipelineBindPoint bind_point, VkPipelineLayout layout,
            uint32_t first_set, const std::span<VkDescriptorSet>& sets) const;

        /**
         * @brief https://www.khronos.org/registry/vulkan/specs/1.3-extensions/man/html/vkCmdBindPipeline.html
        */
        void bind_pipeline(const Pipeline& pipeline) const;

        /**
         * @brief https://www.khronos.org/registry/vulkan/specs/1.3-extensions/man/html/vkCmdClearColorImage.html
         * https://www.khronos.org/registry/vulkan/specs/1.3-extensions/man/html/vkCmdClearDepthStencilImage.html
        */
        void clear_image(const Image& image, VkImageLayout layout, const Clear_value& clear_value, uint32_t base_mip_level,
            uint32_t level_count, uint32_t base_array_layer, uint32_t layer_count) const;

        /**
         * @brief https://www.khronos.org/registry/vulkan/specs/1.3-extensions/man/html/vkCmdDispatch.html
        */
        void dispatch(uint32_t x, uint32_t y, uint32_t z) const;

        /**
         * @brief https://www.khronos.org/registry/vulkan/specs/1.3-extensions/man/html/vkCmdDispatchBase.html
        */
        void dispatch_base(uint32_t x, uint32_t y, uint32_t z, uint32_t x_offset, uint32_t y_offset, uint32_t z_offset) const;

        /**
         * @brief https://www.khronos.org/registry/vulkan/specs/1.3-extensions/man/html/vkCmdDispatchIndirect.html
        */
        void dispatch_indirect(const Buffer& buffer, uint32_t buffer_offset) const;

        /**
         * @brief https://www.khronos.org/registry/vulkan/specs/1.3-extensions/man/html/vkCmdPushConstants.html
        */
        void push_constants(VkPipelineLayout layout, VkShaderStageFlags stages, const void* data,
            uint32_t size, uint32_t offset) const;
    };
}
