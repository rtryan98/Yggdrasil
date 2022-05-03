// Copyright 2022 Robert Ryan. See Licence.md.

#pragma once

#include "ygg/vulkan/vk_forward_decl.h"

#include <array>
#include <optional>
#include <span>
#include <vector>

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
    const Allocated_buffer& select_allocated_buffer(const Buffer& buffer, uint32_t current_frame_in_flight);

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

    /**
     * @brief Pipeline "primitive".
    */
    struct Pipeline
    {
        VkPipeline handle;
        VkPipelineBindPoint bind_point;
    };

    /**
     * @brief Shader module "primitive".
    */
    struct Shader_module
    {
        VkShaderModule handle;
        VkShaderStageFlagBits stage;
    };

    /**
     * @brief https://www.khronos.org/registry/vulkan/specs/1.3-extensions/man/html/vkCreateShaderModule.html
    */
    Shader_module create_shader_module(VkDevice device, std::span<uint32_t> spirv, VkShaderStageFlagBits stage);

    void destroy_shader_module(VkDevice device, Shader_module& shader);

    /**
     * @brief Collection of Shader_modules used inside a graphics pipeline.
    */
    struct Graphics_program
    {
        Shader_module vert;
        std::optional<Shader_module> tesc;
        std::optional<Shader_module> tese;
        std::optional<Shader_module> geom;
        Shader_module frag;
    };

    /**
     * @brief https://www.khronos.org/registry/vulkan/specs/1.3-extensions/man/html/VkPipelineColorBlendAttachmentState.html
    */
    struct Graphics_pipeline_color_attachment_info
    {
        bool blend_enable;
        VkBlendFactor src_color_blend_factor;
        VkBlendFactor dst_color_blend_factor;
        VkBlendOp color_blend_op;
        VkBlendFactor src_alpha_blend_factor;
        VkBlendFactor dst_alpha_blend_factor;
        VkBlendOp alpha_blend_op;
        VkColorComponentFlags color_write_mask =
            0x00000001 |    // VK_COLOR_COMPONENT_R_BIT
            0x00000002 |    // VK_COLOR_COMPONENT_G_BIT
            0x00000004 |    // VK_COLOR_COMPONENT_B_BIT
            0x00000008;     // VK_COLOR_COMPONENT_A_BIT
        VkFormat format;
    };

    /**
     * @brief https://www.khronos.org/registry/vulkan/specs/1.3-extensions/man/html/VkVertexInputBindingDescription.html
    */
    struct Graphics_pipeline_vertex_binding
    {
        uint32_t binding;
        uint32_t stride;
        VkVertexInputRate input_rate;
    };

    /**
     * @brief https://www.khronos.org/registry/vulkan/specs/1.3-extensions/man/html/VkVertexInputAttributeDescription.html
    */
    struct Graphics_pipeline_attribute_description
    {
        uint32_t location;
        uint32_t binding;
        VkFormat format;
        uint32_t offset;
    };

    /**
     * @brief https://www.khronos.org/registry/vulkan/specs/1.3-extensions/man/html/VkPipelineVertexInputStateCreateInfo.html
    */
    struct Graphics_pipeline_vertex_input_state
    {
        std::vector<Graphics_pipeline_vertex_binding> bindings;
        std::vector<Graphics_pipeline_attribute_description> attribute_descriptions;
    };

    /**
     * @brief https://www.khronos.org/registry/vulkan/specs/1.3-extensions/man/html/VkPipelineTessellationStateCreateInfo.html
    */
    struct Graphics_pipeline_tesselation_state
    {
        uint32_t patch_control_points;
        VkTessellationDomainOrigin domain_origin;
    };

    /**
     * @brief https://www.khronos.org/registry/vulkan/specs/1.3-extensions/man/html/VkPipelineRasterizationStateCreateInfo.html
    */
    struct Graphics_pipeline_rasterization_state
    {
        bool depth_clamp_enable;
        bool rasterizer_discard_enable;
        VkPolygonMode polygon_mode;
        VkCullModeFlags cull_mode;
        VkFrontFace front_face;
        bool depth_bias_enable;
        float_t depth_bias_constant_factor;
        float_t depth_bias_clamp;
        float_t depth_bias_slope_factor;
        float_t line_width;
    };

    /**
     * @brief https://www.khronos.org/registry/vulkan/specs/1.3-extensions/man/html/VkStencilOpState.html
    */
    struct Graphics_pipeline_stencil_op_state
    {
        VkStencilOp fail_op;
        VkStencilOp pass_op;
        VkStencilOp depth_fail_op;
        VkCompareOp compare_op;
        uint32_t compare_mask;
        uint32_t write_mask;
        uint32_t reference;
    };

    /**
     * @brief https://www.khronos.org/registry/vulkan/specs/1.3-extensions/man/html/VkPipelineDepthStencilStateCreateInfo.html
    */
    struct Graphics_pipeline_depth_stencil_state
    {
        bool depth_test_enable;
        bool depth_write_enable;
        VkCompareOp compare_op;
        bool depth_bounds_test_enable;
        bool stencil_test_enable;
        Graphics_pipeline_stencil_op_state front;
        Graphics_pipeline_stencil_op_state back;
        float_t min_depth_bounds;
        float_t max_depth_bounds;
        VkFormat depth_format;
        VkFormat stencil_format;
    };

    /**
     * @brief https://www.khronos.org/registry/vulkan/specs/1.3-extensions/man/html/VkPipelineInputAssemblyStateCreateInfo.html
    */
    struct Graphics_pipeline_input_assembly_state
    {
        VkPrimitiveTopology topology;
        bool primitive_restart_enable;
    };

    /**
     * @brief https://www.khronos.org/registry/vulkan/specs/1.3-extensions/man/html/VkPipelineColorBlendStateCreateInfo.html
    */
    struct Graphics_pipeline_color_blend_state
    {
        bool logic_op_enable;
        VkLogicOp logic_op;
        float_t blend_constants[4];
    };

    /**
     * @brief https://www.khronos.org/registry/vulkan/specs/1.3-extensions/man/html/VkGraphicsPipelineCreateInfo.html
    */
    struct Graphics_pipeline_info
    {
        VkPipelineCreateFlags flags;
        Graphics_program program;
        std::optional<Graphics_pipeline_vertex_input_state> vertex_input_state;
        Graphics_pipeline_input_assembly_state input_assembly_state;
        Graphics_pipeline_rasterization_state raster_state;
        std::optional<Graphics_pipeline_tesselation_state> tesselation_state;
        std::optional<Graphics_pipeline_depth_stencil_state> depth_stencil_state;
        std::optional<Graphics_pipeline_color_blend_state> color_blend_state;
        std::vector<VkDynamicState> dynamic_states; // VK_DYNAMIC_STATE_VIEWPORT and VK_DYNAMIC_STATE_SCISSOR are always set.
        std::vector<Graphics_pipeline_color_attachment_info> render_target_infos;
        VkPipelineLayout layout;
    };

    /**
     * @brief https://www.khronos.org/registry/vulkan/specs/1.3-extensions/man/html/VkComputePipelineCreateInfo.html
    */
    struct Compute_pipeline_info
    {
        VkPipelineCreateFlags flags;
        Shader_module shader;
        VkPipelineLayout layout;
    };

    /**
     * @brief https://www.khronos.org/registry/vulkan/specs/1.3-extensions/man/html/vkCreateGraphicsPipelines.html
    */
    VkPipeline create_graphics_pipeline(VkDevice device, const Graphics_pipeline_info& info, VkPipelineCache cache = nullptr);

    /**
     * @brief https://www.khronos.org/registry/vulkan/specs/1.3-extensions/man/html/vkCreateComputePipelines.html
    */
    VkPipeline create_compute_pipeline(VkDevice device, const Compute_pipeline_info& info, VkPipelineCache cache = nullptr);

    void destroy_pipeline(VkDevice device, Pipeline pipeline);
}
