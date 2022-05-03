// Copyright 2022 Robert Ryan. See Licence.md.

#include "ygg/vulkan/resource.h"

#include "ygg/vulkan/image_utils.h"

#include <cassert>
#include <vk_mem_alloc.h>
#include <volk.h>

namespace ygg::vk
{
    Allocated_buffer& select_allocated_buffer(Buffer& buffer, uint32_t current_frame_in_flight)
    {
        switch (buffer.info.domain)
        {
        default: ;
        case Buffer_domain::Host_write_combined: return buffer.allocated_buffers[0];
        case Buffer_domain::Device:              return buffer.allocated_buffers[0];
        case Buffer_domain::Device_host_visible: return buffer.allocated_buffers[current_frame_in_flight];
        }
    }

    const Allocated_buffer& select_allocated_buffer(const Buffer& buffer, uint32_t current_frame_in_flight)
    {
        switch (buffer.info.domain)
        {
        default: ;
        case Buffer_domain::Host_write_combined: return buffer.allocated_buffers[0];
        case Buffer_domain::Device:              return buffer.allocated_buffers[0];
        case Buffer_domain::Device_host_visible: return buffer.allocated_buffers[current_frame_in_flight];
        }
    }

    uint32_t get_allocated_buffer_count(Buffer_domain domain, uint32_t max_frames_in_flight)
    {
        switch (domain)
        {
        default: ;
        case Buffer_domain::Host_write_combined: return 1;
        case Buffer_domain::Device:              return 1;
        case Buffer_domain::Device_host_visible: return max_frames_in_flight;
        }
    }

    VkAccessFlags2 buffer_usage_to_access_flags_first_scope(VkBufferUsage usage)
    {
        VkAccessFlags2 flags = 0;
        if (usage & VK_BUFFER_USAGE_TRANSFER_DST_BIT)
            flags |= VK_ACCESS_2_TRANSFER_WRITE_BIT;
        if (usage & VK_BUFFER_USAGE_STORAGE_TEXEL_BUFFER_BIT)
            flags |= VK_ACCESS_2_SHADER_STORAGE_WRITE_BIT;
        if (usage & VK_BUFFER_USAGE_STORAGE_BUFFER_BIT)
            flags |= VK_ACCESS_2_SHADER_STORAGE_WRITE_BIT;
        if (usage & VK_BUFFER_USAGE_TRANSFORM_FEEDBACK_BUFFER_BIT_EXT)
            flags |= VK_ACCESS_2_TRANSFORM_FEEDBACK_WRITE_BIT_EXT;
        if (usage & VK_BUFFER_USAGE_TRANSFORM_FEEDBACK_COUNTER_BUFFER_BIT_EXT)
            flags |= VK_ACCESS_2_TRANSFORM_FEEDBACK_COUNTER_WRITE_BIT_EXT;
        if (usage & VK_BUFFER_USAGE_ACCELERATION_STRUCTURE_STORAGE_BIT_KHR)
            flags |= VK_ACCESS_2_ACCELERATION_STRUCTURE_WRITE_BIT_KHR;
        return flags;
    }

    VkAccessFlags2 buffer_usage_to_access_flags_second_scope(VkBufferUsage usage)
    {
        VkAccessFlags2 flags = 0;
        if (usage & VK_BUFFER_USAGE_TRANSFER_SRC_BIT)
            flags |= VK_ACCESS_2_TRANSFER_READ_BIT;
        if (usage & VK_BUFFER_USAGE_TRANSFER_DST_BIT)
            flags |= VK_ACCESS_2_TRANSFER_WRITE_BIT;
        if (usage & VK_BUFFER_USAGE_UNIFORM_TEXEL_BUFFER_BIT)
            flags |= VK_ACCESS_2_UNIFORM_READ_BIT;
        if (usage & VK_BUFFER_USAGE_STORAGE_TEXEL_BUFFER_BIT)
            flags |= VK_ACCESS_2_SHADER_STORAGE_READ_BIT | VK_ACCESS_2_SHADER_STORAGE_WRITE_BIT;
        if (usage & VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT)
            flags |= VK_ACCESS_2_UNIFORM_READ_BIT;
        if (usage & VK_BUFFER_USAGE_STORAGE_BUFFER_BIT)
            flags |= VK_ACCESS_2_SHADER_STORAGE_READ_BIT | VK_ACCESS_2_SHADER_STORAGE_WRITE_BIT;
        if (usage & VK_BUFFER_USAGE_INDEX_BUFFER_BIT)
            flags |= VK_ACCESS_2_INDEX_READ_BIT;
        if (usage & VK_BUFFER_USAGE_VERTEX_BUFFER_BIT)
            flags |= VK_ACCESS_2_VERTEX_ATTRIBUTE_READ_BIT;
        if (usage & VK_BUFFER_USAGE_INDIRECT_BUFFER_BIT)
            flags |= VK_ACCESS_2_INDIRECT_COMMAND_READ_BIT;
        if (usage & VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT)
            flags |= VK_ACCESS_2_SHADER_READ_BIT;
        if (usage & VK_BUFFER_USAGE_TRANSFORM_FEEDBACK_BUFFER_BIT_EXT)
            flags |= VK_ACCESS_2_TRANSFORM_FEEDBACK_WRITE_BIT_EXT;
        if (usage & VK_BUFFER_USAGE_TRANSFORM_FEEDBACK_COUNTER_BUFFER_BIT_EXT)
            flags |= VK_ACCESS_2_TRANSFORM_FEEDBACK_COUNTER_READ_BIT_EXT | VK_ACCESS_2_TRANSFORM_FEEDBACK_COUNTER_WRITE_BIT_EXT;
        if (usage & VK_BUFFER_USAGE_CONDITIONAL_RENDERING_BIT_EXT)
            flags |= VK_ACCESS_2_CONDITIONAL_RENDERING_READ_BIT_EXT;
        if (usage & VK_BUFFER_USAGE_ACCELERATION_STRUCTURE_BUILD_INPUT_READ_ONLY_BIT_KHR)
            flags |= VK_ACCESS_2_ACCELERATION_STRUCTURE_READ_BIT_KHR;
        if (usage & VK_BUFFER_USAGE_ACCELERATION_STRUCTURE_STORAGE_BIT_KHR)
            flags |= VK_ACCESS_2_ACCELERATION_STRUCTURE_READ_BIT_KHR | VK_ACCESS_2_ACCELERATION_STRUCTURE_WRITE_BIT_KHR;
        if (usage & VK_BUFFER_USAGE_SHADER_BINDING_TABLE_BIT_KHR)
            flags |= VK_ACCESS_2_SHADER_READ_BIT;
        return flags;
    }

    VkAccessFlags2 image_usage_to_access_flags_first_scope(VkImageUsage usage)
    {
        VkAccessFlags2 flags = 0;
        if (usage & VK_IMAGE_USAGE_TRANSFER_DST_BIT)
            flags |= VK_ACCESS_2_TRANSFER_WRITE_BIT;
        if (usage & VK_IMAGE_USAGE_STORAGE_BIT)
            flags |= VK_ACCESS_2_SHADER_STORAGE_WRITE_BIT;
        if (usage & VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT)
            flags |= VK_ACCESS_2_COLOR_ATTACHMENT_WRITE_BIT;
        if (usage & VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT)
            flags |= VK_ACCESS_2_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
        return flags;
    }

    VkAccessFlags2 image_usage_to_access_flags_second_scope(VkImageUsage usage)
    {
        VkAccessFlags2 flags = 0;
        if (usage & VK_IMAGE_USAGE_TRANSFER_SRC_BIT)
            flags |= VK_ACCESS_2_TRANSFER_READ_BIT;
        if (usage & VK_IMAGE_USAGE_TRANSFER_DST_BIT)
            flags |= VK_ACCESS_2_TRANSFER_WRITE_BIT;
        if (usage & VK_IMAGE_USAGE_SAMPLED_BIT)
            flags |= VK_ACCESS_2_SHADER_SAMPLED_READ_BIT;
        if (usage & VK_IMAGE_USAGE_STORAGE_BIT)
            flags |= VK_ACCESS_2_SHADER_STORAGE_READ_BIT | VK_ACCESS_2_SHADER_STORAGE_WRITE_BIT;
        if (usage & VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT)
            flags |= VK_ACCESS_2_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_2_COLOR_ATTACHMENT_WRITE_BIT;
        if (usage & VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT)
            flags |= VK_ACCESS_2_DEPTH_STENCIL_ATTACHMENT_READ_BIT | VK_ACCESS_2_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
        return flags;
    }

    Buffer create_buffer(const Buffer_info& info, uint32_t initial_queue_family_index,
        VmaAllocator allocator, uint32_t max_frames_in_flight)
    {
        Buffer result = {
            .info = info
        };
        VkBufferCreateInfo buffer_create_info = {
            .sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
            .pNext = nullptr,
            .flags = 0,
            .size = result.info.size,
            .usage = result.info.usage,
            .sharingMode = VK_SHARING_MODE_EXCLUSIVE,
            .queueFamilyIndexCount = 1,
            .pQueueFamilyIndices = &initial_queue_family_index
        };
        VmaAllocationCreateInfo allocation_create_info = {};
        switch (result.info.domain)
        {
        default: assert(false && "Unreachable code.");
        case Buffer_domain::Host_write_combined:
            allocation_create_info.flags =
                VMA_ALLOCATION_CREATE_MAPPED_BIT |
                VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT |
                VMA_ALLOCATION_CREATE_HOST_ACCESS_ALLOW_TRANSFER_INSTEAD_BIT;
            allocation_create_info.usage = VMA_MEMORY_USAGE_AUTO_PREFER_HOST;
            break;
        case Buffer_domain::Device:
            allocation_create_info.usage = VMA_MEMORY_USAGE_AUTO_PREFER_DEVICE;
            break;
        case Buffer_domain::Device_host_visible:
            allocation_create_info.flags =
                VMA_ALLOCATION_CREATE_MAPPED_BIT |
                VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT |
                VMA_ALLOCATION_CREATE_HOST_ACCESS_ALLOW_TRANSFER_INSTEAD_BIT;
            allocation_create_info.usage = VMA_MEMORY_USAGE_AUTO_PREFER_DEVICE;
            allocation_create_info.requiredFlags =
                VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT |
                VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT;
            break;
        }
        for (uint32_t i = 0; i < get_allocated_buffer_count(result.info.domain, max_frames_in_flight); i++) {
            auto& allocated_buffer = select_allocated_buffer(result, i);
            VmaAllocationInfo allocation_info = {};
            // TODO: add VK_CHECK();
            vmaCreateBuffer(allocator, &buffer_create_info, &allocation_create_info, &allocated_buffer.handle,
                &allocated_buffer.allocation, &allocation_info);
            allocated_buffer.mapped_data = allocation_info.pMappedData;
        }

        return result;
    }

    Image create_image(const Image_info& info, uint32_t initial_queue_family_index, VmaAllocator allocator, VkDevice device)
    {
        Image result = {
            .info = info
        };
        VkImageCreateInfo image_create_info = {
            .sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO,
            .pNext = nullptr,
            .flags = 0,
            .imageType = result.info.type,
            .format = result.info.format,
            .extent = { result.info.width, result.info.height, result.info.depth },
            .mipLevels = result.info.mip_levels,
            .arrayLayers = result.info.array_layers,
            .samples = VK_SAMPLE_COUNT_1_BIT,
            .tiling = VK_IMAGE_TILING_OPTIMAL,
            .usage = result.info.usage,
            .sharingMode = VK_SHARING_MODE_EXCLUSIVE,
            .queueFamilyIndexCount = 1,
            .pQueueFamilyIndices = &initial_queue_family_index,
            .initialLayout = VK_IMAGE_LAYOUT_UNDEFINED
        };
        VmaAllocationCreateInfo allocation_create_info = {
            .usage = VMA_MEMORY_USAGE_AUTO_PREFER_DEVICE
        };
        // TODO: add VK_CHECK();
        vmaCreateImage(allocator, &image_create_info, &allocation_create_info,
            &result.allocated_image.handle, &result.allocated_image.allocation, nullptr);

        VkImageViewCreateInfo image_view_create_info = {
            .sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
            .pNext = nullptr,
            .flags = img_utils::get_default_image_create_flags(result.info),
            .image = result.allocated_image.handle,
            .viewType = img_utils::get_default_image_view_type(result.info),
            .format = result.info.format,
            .components = {
                .r = VK_COMPONENT_SWIZZLE_IDENTITY,
                .g = VK_COMPONENT_SWIZZLE_IDENTITY,
                .b = VK_COMPONENT_SWIZZLE_IDENTITY,
                .a = VK_COMPONENT_SWIZZLE_IDENTITY
            },
            .subresourceRange = img_utils::get_default_image_subresource_range(result.info)
        };
        // TODO: add VK_CHECK();
        vkCreateImageView(device, &image_view_create_info, nullptr, &result.allocated_image.default_view);
        return result;
    }

    void destroy_buffer(Buffer& buffer, VmaAllocator allocator, uint32_t max_frames_in_flight)
    {
        for (uint32_t i = 0; i < get_allocated_buffer_count(buffer.info.domain, max_frames_in_flight); i++) {
            auto& allocated_buffer = buffer.allocated_buffers[i];
            vmaDestroyBuffer(allocator, allocated_buffer.handle, allocated_buffer.allocation);
        }
    }

    void destroy_image(Image& image, VmaAllocator allocator, VkDevice device)
    {
        vkDestroyImageView(device, image.allocated_image.default_view, nullptr);
        vmaDestroyImage(allocator, image.allocated_image.handle, image.allocated_image.allocation);
    }

    Shader_module create_shader_module(VkDevice device, std::span<uint32_t> spirv, VkShaderStageFlagBits stage)
    {
        VkShaderModule sh_module = VK_NULL_HANDLE;
        VkShaderModuleCreateInfo info = {
            .sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO,
            .pNext = nullptr,
            .flags = 0,
            .codeSize = uint32_t(sizeof(uint32_t) * spirv.size()),
            .pCode = spirv.data()
        };
        // TODO: VK_CHECK
        vkCreateShaderModule(device, &info, nullptr, &sh_module);
        return {
            .handle = sh_module,
            .stage = stage
        };
    }

    void destroy_shader_module(VkDevice device, Shader_module& shader)
    {
        vkDestroyShaderModule(device, shader.handle, nullptr);
    }

    VkPipelineShaderStageCreateInfo shader_stage_create_info(VkShaderModule module, VkShaderStageFlagBits stage)
    {
        return {
                .sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
                .pNext = nullptr,
                .flags = 0,
                .stage = stage,
                .module = module,
                .pName = "main",
                .pSpecializationInfo = nullptr
        };
    }

    VkPipeline create_graphics_pipeline(VkDevice device, const Graphics_pipeline_info& info, VkPipelineCache cache)
    {
        uint32_t stage_count = 0;
        std::array<VkPipelineShaderStageCreateInfo, 5> shader_stage_info = {};
        shader_stage_info[stage_count++] = shader_stage_create_info(info.program.vert.handle, info.program.vert.stage);
        if (info.program.tese.has_value()) {
            const auto& tese = info.program.tese.value();
            shader_stage_info[stage_count++] = shader_stage_create_info(tese.handle, tese.stage);
        }
        if (info.program.tesc.has_value()) {
            const auto& tesc = info.program.tesc.value();
            shader_stage_info[stage_count++] = shader_stage_create_info(tesc.handle, tesc.stage);
        }
        if (info.program.geom.has_value()) {
            const auto& geom = info.program.geom.value();
            shader_stage_info[stage_count++] = shader_stage_create_info(geom.handle, geom.stage);
        }
        shader_stage_info[stage_count++] = shader_stage_create_info(info.program.frag.handle, info.program.frag.stage);

        uint32_t vertex_binding_description_count = 0;
        std::array<VkVertexInputBindingDescription, 32> vertex_binding_descriptions = {};
        uint32_t vertex_attribute_description_count = 0;
        std::array<VkVertexInputAttributeDescription, 32> vertex_attribute_descriptions = {};
        if (info.vertex_input_state.has_value()) {
            const auto& vis = info.vertex_input_state.value();
            for (const auto& b : vis.bindings) {
                vertex_binding_descriptions[vertex_binding_description_count++] = {
                    .binding = b.binding,
                    .stride = b.stride,
                    .inputRate = b.input_rate
                };
            }
            for (const auto& a : vis.attribute_descriptions) {
                vertex_attribute_descriptions[vertex_attribute_description_count++] = {
                    .location = a.location,
                    .binding = a.binding,
                    .format = a.format,
                    .offset = a.offset
                };
            }
        }
        VkPipelineVertexInputStateCreateInfo vertex_input_state_info = {
            .sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO,
            .pNext = nullptr,
            .flags = 0,
            .vertexBindingDescriptionCount = vertex_binding_description_count,
            .pVertexBindingDescriptions = vertex_binding_descriptions.data(),
            .vertexAttributeDescriptionCount = vertex_attribute_description_count,
            .pVertexAttributeDescriptions = vertex_attribute_descriptions.data()
        };

        VkPipelineInputAssemblyStateCreateInfo input_assembly_info = {
            .sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO,
            .pNext = nullptr,
            .flags = 0,
            .topology = info.input_assembly_state.topology,
            .primitiveRestartEnable = info.input_assembly_state.primitive_restart_enable
        };

        VkPipelineTessellationDomainOriginStateCreateInfo tesselation_domain_info = {
            .sType = VK_STRUCTURE_TYPE_PIPELINE_TESSELLATION_DOMAIN_ORIGIN_STATE_CREATE_INFO,
            .pNext = nullptr,
            .domainOrigin = info.tesselation_state.has_value()
                ? info.tesselation_state.value().domain_origin
                : VK_TESSELLATION_DOMAIN_ORIGIN_UPPER_LEFT
        };
        VkPipelineTessellationStateCreateInfo tesselation_info = {
            .sType = VK_STRUCTURE_TYPE_PIPELINE_TESSELLATION_STATE_CREATE_INFO,
            .pNext = &tesselation_domain_info,
            .flags = 0,
            .patchControlPoints = info.tesselation_state.has_value()
                ? info.tesselation_state.value().patch_control_points
                :   0
        };

        VkViewport vp = {};
        VkRect2D sc = {};
        VkPipelineViewportStateCreateInfo viewport_state = {
            .sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO,
            .pNext = nullptr,
            .flags = 0,
            .viewportCount = 1,
            .pViewports = &vp,
            .scissorCount = 1,
            .pScissors = &sc
        };

        VkPipelineRasterizationStateCreateInfo raster_info = {
            .sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO,
            .pNext = nullptr,
            .flags = 0,
            .depthClampEnable = info.raster_state.depth_clamp_enable,
            .rasterizerDiscardEnable = info.raster_state.rasterizer_discard_enable,
            .polygonMode = info.raster_state.polygon_mode,
            .cullMode = info.raster_state.cull_mode,
            .frontFace = info.raster_state.front_face,
            .depthBiasEnable = info.raster_state.depth_bias_enable,
            .depthBiasConstantFactor = info.raster_state.depth_bias_constant_factor,
            .depthBiasClamp = info.raster_state.depth_bias_clamp,
            .depthBiasSlopeFactor = info.raster_state.depth_bias_slope_factor,
            .lineWidth = info.raster_state.line_width
        };

        VkPipelineMultisampleStateCreateInfo multi_sample_state = {
            .sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO,
            .pNext = nullptr,
            .flags = 0,
            .rasterizationSamples = VK_SAMPLE_COUNT_1_BIT,
            .sampleShadingEnable = VK_FALSE
        };

        auto ds_state = info.depth_stencil_state.value_or(Graphics_pipeline_depth_stencil_state());
        VkPipelineDepthStencilStateCreateInfo depth_stencil_info = {
            .sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO,
            .pNext = nullptr,
            .flags = 0,
            .depthTestEnable = ds_state.depth_test_enable,
            .depthWriteEnable = ds_state.depth_write_enable,
            .depthCompareOp = ds_state.compare_op,
            .depthBoundsTestEnable = ds_state.depth_bounds_test_enable,
            .stencilTestEnable = ds_state.stencil_test_enable,
            .front = {
                .failOp = ds_state.front.fail_op,
                .passOp = ds_state.front.pass_op,
                .depthFailOp = ds_state.front.depth_fail_op,
                .compareOp = ds_state.front.compare_op,
                .compareMask = ds_state.front.compare_mask,
                .writeMask = ds_state.front.write_mask,
                .reference = ds_state.front.reference
            },
            .back = {
                .failOp = ds_state.back.fail_op,
                .passOp = ds_state.back.pass_op,
                .depthFailOp = ds_state.back.depth_fail_op,
                .compareOp = ds_state.back.compare_op,
                .compareMask = ds_state.back.compare_mask,
                .writeMask = ds_state.back.write_mask,
                .reference = ds_state.back.reference
            },
            .minDepthBounds = ds_state.min_depth_bounds,
            .maxDepthBounds = ds_state.max_depth_bounds
        };

        uint32_t attachment_count = 0;
        std::array<VkPipelineColorBlendAttachmentState, 32> attachments = {};
        for (const auto& rt : info.render_target_infos) {
            attachments[attachment_count++] = {
                .blendEnable = rt.blend_enable,
                .srcColorBlendFactor = rt.src_color_blend_factor,
                .dstColorBlendFactor = rt.dst_color_blend_factor,
                .colorBlendOp = rt.color_blend_op,
                .srcAlphaBlendFactor = rt.src_alpha_blend_factor,
                .dstAlphaBlendFactor = rt.dst_alpha_blend_factor,
                .alphaBlendOp = rt.alpha_blend_op,
                .colorWriteMask = rt.color_write_mask
            };
        }
        const auto cbs = info.color_blend_state.value_or(Graphics_pipeline_color_blend_state());
        VkPipelineColorBlendStateCreateInfo color_blend_info = {
            .sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO,
            .pNext = nullptr,
            .flags = 0,
            .logicOpEnable = cbs.logic_op_enable,
            .logicOp = cbs.logic_op,
            .attachmentCount = attachment_count,
            .pAttachments = attachments.data(),
            .blendConstants = {
                cbs.blend_constants[0],
                cbs.blend_constants[1],
                cbs.blend_constants[2],
                cbs.blend_constants[3]
            }
        };

        uint32_t dynamic_state_count = 2u + uint32_t(info.dynamic_states.size());
        std::array<VkDynamicState, 64> dynamic_states = { VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR };
        for (uint32_t i = 0; i < info.dynamic_states.size(); i++) {
            dynamic_states[2 + i] = info.dynamic_states[i];
        }
        VkPipelineDynamicStateCreateInfo dynamic_state_create_info = {
            .sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO,
            .pNext = nullptr,
            .flags = 0,
            .dynamicStateCount = dynamic_state_count,
            .pDynamicStates = dynamic_states.data()
        };

        uint32_t color_format_count = 0;
        std::array<VkFormat, 32> color_formats = {};
        for (const auto& attachment : info.render_target_infos) {
            color_formats[color_format_count++] = attachment.format;
        }
        VkPipelineRenderingCreateInfo dynamic_rendering_info = {
            .sType = VK_STRUCTURE_TYPE_PIPELINE_RENDERING_CREATE_INFO,
            .pNext = nullptr,
            .viewMask = 0,
            .colorAttachmentCount = color_format_count,
            .pColorAttachmentFormats = color_formats.data(),
            .depthAttachmentFormat = info.depth_stencil_state.has_value()
                ? info.depth_stencil_state.value().depth_format
                : VK_FORMAT_UNDEFINED,
            .stencilAttachmentFormat = info.depth_stencil_state.has_value()
                ? info.depth_stencil_state.value().stencil_format
                : VK_FORMAT_UNDEFINED
        };

        VkGraphicsPipelineCreateInfo create_info = {
            .sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO,
            .pNext = (info.render_target_infos.size() > 0) ? &dynamic_rendering_info : nullptr,
            .flags = info.flags,
            .stageCount = stage_count,
            .pStages = shader_stage_info.data(),
            .pVertexInputState = &vertex_input_state_info,
            .pInputAssemblyState = &input_assembly_info,
            .pTessellationState = info.tesselation_state.has_value() ? &tesselation_info : nullptr,
            .pViewportState = &viewport_state,
            .pRasterizationState = &raster_info,
            .pMultisampleState = &multi_sample_state,
            .pDepthStencilState = info.depth_stencil_state.has_value() ? &depth_stencil_info : nullptr,
            .pColorBlendState = (info.render_target_infos.size() > 0) ? &color_blend_info : nullptr,
            .pDynamicState = &dynamic_state_create_info,
            .layout = info.layout,
            .renderPass = VK_NULL_HANDLE,
            .subpass = 0,
            .basePipelineHandle = VK_NULL_HANDLE,
            .basePipelineIndex = 0
        };
        VkPipeline result = VK_NULL_HANDLE;
        vkCreateGraphicsPipelines(device, cache, 1, &create_info, nullptr, &result);
        return result;
    }

    VkPipeline create_compute_pipeline(VkDevice device, const Compute_pipeline_info& info, VkPipelineCache cache)
    {
        VkComputePipelineCreateInfo create_info = {
            .sType = VK_STRUCTURE_TYPE_COMPUTE_PIPELINE_CREATE_INFO,
            .pNext = nullptr,
            .flags = info.flags,
            .stage = shader_stage_create_info(info.shader.handle, info.shader.stage),
            .layout = info.layout,
            .basePipelineHandle = VK_NULL_HANDLE,
            .basePipelineIndex = 0
        };

        VkPipeline result = VK_NULL_HANDLE;
        vkCreateComputePipelines(device, cache, 1, &create_info, nullptr, &result);
        return result;
    }

    void destroy_pipeline(VkDevice device, Pipeline pipeline)
    {
        vkDestroyPipeline(device, pipeline.handle, nullptr);
    }
}
