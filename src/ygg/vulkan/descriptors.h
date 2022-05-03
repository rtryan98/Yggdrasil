// Copyright 2022 Robert Ryan. See Licence.md.

#pragma once

#include "ygg/vulkan/vk_forward_decl.h"

#include <span>
#include <vector>

namespace ygg::vk
{
    /**
     * @brief https://www.khronos.org/registry/vulkan/specs/1.3-extensions/man/html/VkDescriptorBufferInfo.html
    */
    struct Descriptor_buffer_info
    {
        VkBuffer buffer;
        VkDeviceSize offset;
        VkDeviceSize range;
    };

    /**
     * @brief https://www.khronos.org/registry/vulkan/specs/1.3-extensions/man/html/VkDescriptorImageInfo.html
    */
    struct Descriptor_image_info
    {
        VkSampler sampler;
        VkImageView view;
        VkImageLayout layout;
    };

    /**
     * @brief https://www.khronos.org/registry/vulkan/specs/1.3-extensions/man/html/VkBufferView.html
    */
    struct Descriptor_texel_buffer_view_info
    {
        VkBufferView buffer_view;
    };

    /**
     * @brief https://www.khronos.org/registry/vulkan/specs/1.3-extensions/man/html/VkWriteDescriptorSetAccelerationStructureKHR.html
    */
    struct Descriptor_acceleration_structure_info
    {
        VkAccelerationStructureKHR acceleration_structure;
    };

    /**
     * @brief https://www.khronos.org/registry/vulkan/specs/1.3-extensions/man/html/VkWriteDescriptorSet.html
    */
    struct Descriptor_set_write_info
    {
        VkDescriptorSet set;
        uint32_t binding;
        uint32_t array_index;
        VkDescriptorType type;
        union
        {
            std::span<Descriptor_buffer_info> buffer_infos;
            std::span<Descriptor_image_info> image_infos;
            std::span<Descriptor_texel_buffer_view_info> texel_buffer_view_infos;
            std::span<Descriptor_acceleration_structure_info> acceleration_structure_infos;
        };
    };

    /**
     * @brief https://www.khronos.org/registry/vulkan/specs/1.3-extensions/man/html/vkUpdateDescriptorSets.html
    */
    void update_descriptor_set(VkDevice device, const Descriptor_set_write_info& write_info);
    void update_descriptor_sets(VkDevice device, std::span<Descriptor_set_write_info> write_infos);

    /**
     * @brief Combination of VkDescriptorSetLayoutBinding and an inlined
     * VkDescriptorSetLayoutBindingFlagsCreateInfo
    */
    struct Descriptor_set_layout_binding
    {
        uint32_t binding;
        VkDescriptorType type;
        uint32_t count;
        VkShaderStageFlags stages;
        std::vector<VkSampler> immutable_samplers;
        VkDescriptorBindingFlags flags;
    };

    /**
     * @brief https://www.khronos.org/registry/vulkan/specs/1.3-extensions/man/html/VkDescriptorSetLayoutCreateInfo.html
    */
    struct Descriptor_set_layout_info
    {
        VkDescriptorSetLayoutCreateFlags flags;
        std::vector<Descriptor_set_layout_binding> bindings;
    };

    /**
     * @brief https://www.khronos.org/registry/vulkan/specs/1.3-extensions/man/html/vkCreateDescriptorSetLayout.html
    */
    VkDescriptorSetLayout create_descriptor_set_layout(VkDevice device, const Descriptor_set_layout_info& info);

    /**
     * @brief https://www.khronos.org/registry/vulkan/specs/1.3-extensions/man/html/VkDescriptorPoolSize.html
    */
    struct Descriptor_pool_size
    {
        VkDescriptorType type;
        uint32_t size;
    };

    /**
     * @brief Descriptor allocator used to allocate one-frame-use descriptor sets.
    */
    class Transient_descriptor_set_allocator
    {
    public:
        /**
         * @brief Creates a Transient_descriptor_set_allocator able to allocate descriptor sets
         * using a descriptor set layout that fits within the passed sizes.
        */
        Transient_descriptor_set_allocator(VkDevice device, std::span<Descriptor_pool_size> sizes,
            uint32_t sets_per_pool = 1024);
        ~Transient_descriptor_set_allocator();

        /**
         * @brief Acquires a new transient descriptor set using the given layout.
        */
        VkDescriptorSet get_set(VkDescriptorSetLayout layout);

        /**
         * @brief Resets this allocator, freeing and thus invalidating all
         * sets that were allocated from this instance.
        */
        void reset();

    private:
        VkDescriptorPool acquire_new_pool();

    private:
        VkDevice m_device;
        const uint32_t m_sets_per_pool;
        VkDescriptorPool m_active_pool = nullptr;
        std::vector<Descriptor_pool_size> m_sizes;
        std::vector<VkDescriptorPool> m_recycled_pools = {};
        std::vector<VkDescriptorPool> m_available_pools = {};
    };

    /**
     * @brief https://www.khronos.org/registry/vulkan/specs/1.3-extensions/man/html/VkPushConstantRange.html
    */
    struct Pipeline_layout_push_constant_range
    {
        uint32_t size;
        uint32_t offset;
        VkShaderStageFlags stages;
    };

    /**
     * @brief https://www.khronos.org/registry/vulkan/specs/1.3-extensions/man/html/VkPipelineLayoutCreateInfo.html
    */
    struct Pipeline_layout_info
    {
        std::span<VkDescriptorSetLayout> layouts;
        std::span<Pipeline_layout_push_constant_range> push_constant_ranges;
    };

    /**
     * @brief https://www.khronos.org/registry/vulkan/specs/1.3-extensions/man/html/vkCreatePipelineLayout.html
    */
    VkPipelineLayout create_pipeline_layout(VkDevice device, const Pipeline_layout_info& info);
}
