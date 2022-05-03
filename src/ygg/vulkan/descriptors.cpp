// Copyright 2022 Robert Ryan. See Licence.md.

#include "ygg/vulkan/descriptors.h"

#include <array>
#include <volk.h>

namespace ygg::vk
{
    void update_descriptor_set(VkDevice device, const Descriptor_set_write_info& write_info)
    {
        auto arr = std::to_array<Descriptor_set_write_info>({ write_info });
        update_descriptor_sets(device, { arr });
    }

    void update_descriptor_sets(VkDevice device, std::span<Descriptor_set_write_info> write_infos)
    {
        std::vector<std::vector<VkDescriptorBufferInfo>> buffer_infos;
        buffer_infos.reserve(write_infos.size());
        std::vector<std::vector<VkDescriptorImageInfo>> image_infos;
        image_infos.reserve(write_infos.size());
        std::vector<std::vector<VkBufferView>> buffer_views;
        buffer_views.reserve(write_infos.size());

        std::vector<VkWriteDescriptorSet> writes;
        writes.reserve(write_infos.size());
        for (const auto& wi : write_infos) {
            auto& write_buffer_infos = buffer_infos.emplace_back();
            auto& write_image_infos = image_infos.emplace_back();
            auto& write_buffer_views = buffer_views.emplace_back();
            VkWriteDescriptorSet write {
                    .sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
                    .pNext = nullptr,
                    .dstSet = wi.set,
                    .dstBinding = wi.binding,
                    .dstArrayElement = wi.array_index,
                    .descriptorType = wi.type
            };
            uint32_t count = 0;
            switch (wi.type)
            {
            default: break;
            case VK_DESCRIPTOR_TYPE_SAMPLER:
                ;
            case VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER:
                ;
            case VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE:
                ;
            case VK_DESCRIPTOR_TYPE_STORAGE_IMAGE:
                write_image_infos.reserve(wi.image_infos.size());
                for (const auto& img_info : wi.image_infos) {
                    write_image_infos.emplace_back( VkDescriptorImageInfo {
                            .sampler = img_info.sampler,
                            .imageView = img_info.view,
                            .imageLayout = img_info.layout
                        });
                }
                count = uint32_t(wi.image_infos.size());
                write.pImageInfo = write_image_infos.data();
                break;
            case VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER:
                ;
            case VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER:
                write_buffer_views.reserve(wi.texel_buffer_view_infos.size());
                for (const auto& buffer_view : wi.texel_buffer_view_infos) {
                    write_buffer_views.emplace_back( buffer_view.buffer_view );
                }
                count = uint32_t(wi.texel_buffer_view_infos.size());
                write.pTexelBufferView = write_buffer_views.data();
                break;
            case VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER:
                ;
            case VK_DESCRIPTOR_TYPE_STORAGE_BUFFER:
                ;
            case VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC:
                ;
            case VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC:
                write_buffer_infos.reserve(wi.buffer_infos.size());
                for (const auto& buffer_info : wi.buffer_infos) {
                    write_buffer_infos.emplace_back( VkDescriptorBufferInfo {
                            .buffer = buffer_info.buffer,
                            .offset = buffer_info.offset,
                            .range = buffer_info.range
                        });
                }
                count = uint32_t(wi.buffer_infos.size());
                write.pBufferInfo = write_buffer_infos.data();
                break;
            case VK_DESCRIPTOR_TYPE_ACCELERATION_STRUCTURE_KHR:
                // TODO: implement acceleration structure updating.
                break;
            }
            write.descriptorCount = count;
            writes.emplace_back( write );
        }
        vkUpdateDescriptorSets(device, uint32_t(writes.size()), writes.data(), 0, nullptr);
    }

    VkDescriptorSetLayout create_descriptor_set_layout(VkDevice device, const Descriptor_set_layout_info& info)
    {
        std::vector<VkDescriptorBindingFlags> binding_flags;
        binding_flags.reserve(info.bindings.size());

        std::vector<VkDescriptorSetLayoutBinding> bindings;
        bindings.reserve(info.bindings.size());

        for (const auto& b : info.bindings) {
            bindings.emplace_back( VkDescriptorSetLayoutBinding {
                    .binding = b.binding,
                    .descriptorType = b.type,
                    .descriptorCount = b.count,
                    .stageFlags = b.stages,
                    .pImmutableSamplers = b.immutable_samplers.data()
                });
            binding_flags.emplace_back(b.flags);
        }

        VkDescriptorSetLayoutBindingFlagsCreateInfo flags_info = {
            .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_BINDING_FLAGS_CREATE_INFO,
            .pNext = nullptr,
            .bindingCount = uint32_t(binding_flags.size()),
            .pBindingFlags = binding_flags.data()
        };

        VkDescriptorSetLayoutCreateInfo create_info = {
            .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO,
            .pNext = nullptr,
            .flags = info.flags,
            .bindingCount = uint32_t(bindings.size()),
            .pBindings = bindings.data()
        };

        VkDescriptorSetLayout result;
        // TODO: VK_CHECK
        vkCreateDescriptorSetLayout(device, &create_info, nullptr, &result);
        return result;
    }

    std::vector<VkDescriptorPoolSize> pool_sizes(const std::span<Descriptor_pool_size>& sizes)
    {
        std::vector<VkDescriptorPoolSize> result;
        result.reserve(sizes.size());
        for (const auto& size : sizes) {
            result.emplace_back( VkDescriptorPoolSize { size.type, size.size } );
        }
        return result;
    }

    Transient_descriptor_set_allocator::Transient_descriptor_set_allocator(VkDevice device,
        std::span<Descriptor_pool_size> sizes, uint32_t sets_per_pool)
        : m_device(device), m_sets_per_pool(sets_per_pool), m_sizes(sizes.begin(), sizes.end())
    {
        m_active_pool = acquire_new_pool();
    }

    Transient_descriptor_set_allocator::~Transient_descriptor_set_allocator()
    {
        reset();
        for (auto pool : m_available_pools) {
            vkDestroyDescriptorPool(m_device, pool, nullptr);
        }
    }

    VkDescriptorSet Transient_descriptor_set_allocator::get_set(VkDescriptorSetLayout layout)
    {
        while (true) {
            VkDescriptorSet set = VK_NULL_HANDLE;
            VkDescriptorSetAllocateInfo alloc_info = {
                .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO,
                .pNext = nullptr,
                .descriptorPool = m_active_pool,
                .descriptorSetCount = 1,
                .pSetLayouts = &layout
            };
            auto alloc_result = vkAllocateDescriptorSets(m_device, &alloc_info, &set);
            if (alloc_result == VK_SUCCESS) {
                return set;
            }
            else if (alloc_result == VK_ERROR_FRAGMENTED_POOL
                || alloc_result == VK_ERROR_OUT_OF_POOL_MEMORY) {
                m_active_pool = acquire_new_pool();
            }
            else {
                // TODO: VK_CHECK
                return VK_NULL_HANDLE;
            }
        }
    }

    void Transient_descriptor_set_allocator::reset()
    {
        for (auto pool : m_recycled_pools) {
            m_available_pools.emplace_back(pool);
        }
        m_recycled_pools.clear();
        for (auto pool : m_available_pools) {
            vkResetDescriptorPool(m_device, pool, 0);
        }
    }

    VkDescriptorPool Transient_descriptor_set_allocator::acquire_new_pool()
    {
        VkDescriptorPool result = VK_NULL_HANDLE;
        if (m_available_pools.size()) {
            result = m_available_pools.back();
            m_available_pools.pop_back();
        }

        constexpr static uint64_t MAX_POOL_SIZES = 32;
        std::array<VkDescriptorPoolSize, MAX_POOL_SIZES> sizes = {};
        uint32_t size_count = 0;
        for (const auto& s : m_sizes) {
            sizes[size_count++] = {
                .type = s.type,
                .descriptorCount = s.size
            };
        }
        VkDescriptorPoolCreateInfo info = {
            .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO,
            .pNext = nullptr,
            .flags = 0,
            .maxSets = m_sets_per_pool,
            .poolSizeCount = size_count,
            .pPoolSizes = sizes.data()
        };
        // TODO: VK_CHECK
        vkCreateDescriptorPool(m_device, &info, nullptr, &result);
        m_available_pools.emplace_back(result);
        return result;
    }

    VkPipelineLayout create_pipeline_layout(VkDevice device, const Pipeline_layout_info& info)
    {
        std::vector<VkPushConstantRange> pc_ranges = {};
        pc_ranges.reserve(info.push_constant_ranges.size());
        for (const auto& range : info.push_constant_ranges) {
            pc_ranges.emplace_back( VkPushConstantRange{ range.stages, range.offset, range.size });
        }
        VkPipelineLayout result = VK_NULL_HANDLE;
        VkPipelineLayoutCreateInfo create_info = {
            .sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO,
            .pNext = nullptr,
            .flags = 0,
            .setLayoutCount = uint32_t(info.layouts.size()),
            .pSetLayouts = info.layouts.data(),
            .pushConstantRangeCount = uint32_t(pc_ranges.size()),
            .pPushConstantRanges = pc_ranges.data()
        };
        vkCreatePipelineLayout(device, &create_info, nullptr, &result);
        return result;
    }
}
