// Copyright 2022 Robert Ryan. See Licence.md.

#pragma once

#include "ygg/vulkan/descriptors.h"
#include "ygg/vulkan/linear_host_resource_allocator.h"
#include "ygg/vulkan/command_buffer_recycler.h"
#include "ygg/vulkan/resource.h"
#include "ygg/vulkan/vk_forward_decl.h"

#include <memory>
#include <vector>

namespace ygg::vk
{
    class Window_system_integration;
    class Context;
    class Transfer_command_buffer;
    class Compute_command_buffer;
    class Graphics_command_buffer;

    /**
     * @brief Selection of Profiles that the engine can use.
    */
    enum class Profile {
        /**
        * Baseline Profile.
        * This tier contains most of the functionality provided by code Vulkan 1.0 to 1.3.
        * Some notable exception are sparse memory and multi-viewports.
        */
        Tier_1 = 0,

        /**
        * Extended Profile.
        * This tier builds up from Tier_1 and contains support for acceleration structures,
        * ray tracing (both ray query and ray tracing pipelines) and fragment shading rate.
        */
        Tier_2,

        /**
        * Extended Profile.
        * This tier builds up from Tier_2 and contains support for Nvidias Mesh-Shaders
        * and Nvidias barycentric extension.
        */
        Tier_3
    };

    /**
     * @brief Per-frame "sub"-Context.
    */
    class Frame_context
    {
    public:
        /**
         * @brief Constructs a Frame_context instance that is bound to the Context.
        */
        Frame_context(const Context& context);
        ~Frame_context();

        /**
         * @brief Begins this Frame, clearing any allocators, command buffers and zombies.
        */
        void start_frame();

        /**
         * @brief Acquires a Linear_host_resource_allocator that is bound to this Frame_context.
        */
        Linear_host_resource_allocator& acquire_linear_host_resource_allocator();

        /**
         * @brief Acquires a Compute_command_buffer that is bound to this Frame_context.
        */
        Compute_command_buffer acquire_async_compute_command_buffer();

        /**
         * @brief Acquires a Graphics_command_buffer that is bound to this Frame_context.
        */
        Graphics_command_buffer acquire_graphics_command_buffer();

        /**
         * @brief Use to allocate a descriptor set that is only used in the current frame.
        */
        VkDescriptorSet allocate_transient_descriptor_set(VkDescriptorSetLayout layout);

        /**
        * Zombify-methods are used to declare that a resource is a zombie. Any zombified resource
        * will be destroyed once this frame in flight is hit the next time. That means either either
        * two or three frames. This is useful for resources such as semaphores when acquiring an image
        * from an swapchain or when having any "transient" submit where a fence is required.
        * A zombified resource can still be used in the current frame but should not be used in any
        * further frame. Further use can lead to deleting a resource before it's consumed.
        */

        inline void zombify_semaphore(VkSemaphore semaphore) { m_zombie_semaphores.emplace_back(semaphore); };
        inline void zombify_fence(VkFence fence) { m_zombie_fences.emplace_back(fence); };

    private:
        void destroy_all_zombies();

    private:
        const Context& m_context;
        Linear_host_resource_allocator_provider m_linear_host_resource_allocator_provider;
        Command_buffer_recycler m_graphics_command_buffer_recycler;
        Command_buffer_recycler m_async_compute_command_buffer_recycler;
        std::unique_ptr<Transient_descriptor_set_allocator> m_transient_descriptor_set_allocator;

        std::vector<VkSemaphore> m_zombie_semaphores = {};
        std::vector<VkFence> m_zombie_fences = {};
    };

    struct Queue
    {
        VkQueue queue;
        uint32_t queue_family_index;
    };

    struct Semaphore_signal_info
    {
        VkSemaphore semaphore;
        uint64_t value;
        VkPipelineStageFlags2 stage_mask;
    };

    struct Submit
    {
        std::span<Semaphore_signal_info> await_semas;
        std::span<VkCommandBuffer> cmd_bufs;
        std::span<Semaphore_signal_info> signal_semas;
    };

    /**
     * @brief A simple Context for Vulkan applications.
     * @details Initializes Vulkan completely to be used.
     * Multi-Window Contexts are not possible with this.
    */
    class Context
    {
    public:
        /**
         * @brief Constructs a Context instance and binds the WSI to it.
        */
        explicit Context(const Window_system_integration& wsi);
        ~Context();

        /**
         * @brief Ends the current frame.
         * @details Let n be the current frame in flight.
         * This sets n to n + 1 % max_frames_in_flight.
        */
        void end_frame();

        /**
         * @brief Drains the GPU of any work.
         * @details This should only ever be used for cleaning up before exiting, resizing the swapchain
         * or if absolutely necessary.
        */
        void device_wait_idle() const;

        /**
         * @brief Starts the current frame in flight.
         * @details This will clear any zombified resource and reset all command buffers and allocators.
        */
        void begin_frame();

        /*
        * Descriptor updates
        */
        void update_descriptor_set(const Descriptor_set_write_info& info);
        void update_descriptor_sets(std::span<Descriptor_set_write_info> infos);

        /**
         * Resource creation and destruction methods.
        */

        Image create_image(const Image_info& info, uint32_t initial_queue_family_index) const;
        Buffer create_buffer(const Buffer_info& info, uint32_t initial_queue_family_index) const;
        VkDescriptorSetLayout create_descriptor_set_layout(const Descriptor_set_layout_info& info) const;
        VkPipelineLayout create_pipeline_layout(const Pipeline_layout_info& info) const;
        Pipeline create_graphics_pipeline(const Graphics_pipeline_info& info) const;
        Pipeline create_compute_pipeline(const Compute_pipeline_info& info) const;
        Shader_module create_shader_module(std::span<uint32_t> spirv, VkShaderStageFlagBits stage) const;
        VkSemaphore create_binary_semaphore() const;
        Allocated_buffer select_allocated_buffer(const Buffer& buf) const;

        void destroy_image(Image& image) const;
        void destroy_buffer(Buffer& buffer) const;
        void destroy_descriptor_set_layout(VkDescriptorSetLayout layout) const;
        void destroy_pipeline_layout(VkPipelineLayout layout) const;
        void destroy_pipeline(Pipeline& pipeline) const;
        void destroy_shader_module(Shader_module& shader) const;

        /**
         * @brief Use for quickly submitting a single command buffer to a queue.
         * @details The semaphores are optional and may be VK_NULL_HANDLE.
         * The fence is optional and may be VK_NULL_HANDLE.
         * The semaphores that can be passed to this function MUST be binary semaphores.
        */
        VkResult submit_simple(VkQueue queue, VkCommandBuffer cmdbuf, 
            VkSemaphore await_sema, VkSemaphore signal_sema, VkFence signal_fence);

        VkResult submit(VkQueue queue, const Submit& info, VkFence signal_fence);

        /**
         * @return The frame context for the current frame in flight.
        */
        inline Frame_context& frame_context() { return *m_frame_contexts[m_current_frame_in_flight].get(); }

        inline const Queue& graphics_queue() const { return m_graphics_queue; }
        inline const Queue& compute_queue() const { return m_compute_queue; }
        inline const Queue& transfer_queue() const { return m_transfer_queue; }
        inline VmaAllocator allocator() const { return m_allocator; }
        inline VkDevice device() const { return m_device; }
        inline VkPhysicalDevice physical_device() const { return m_physical_device; }
        inline VkInstance instance() const { return m_instance; }
        inline VkSurfaceKHR surface() const { return m_surface; }
        inline VkFence frame_fence() const { return m_frame_fences[m_current_frame_in_flight]; }
        inline Profile profile() const { return m_profile; }
        inline uint32_t current_frame_in_flight() const { return m_current_frame_in_flight; }

    private:
        const Window_system_integration& m_wsi;
        VkInstance m_instance = nullptr;
        VkSurfaceKHR m_surface = nullptr;
        VkPhysicalDevice m_physical_device = nullptr;
        VkDevice m_device = nullptr;
        Queue m_graphics_queue = {};
        Queue m_compute_queue = {};
        Queue m_transfer_queue = {};
        VmaAllocator m_allocator = nullptr;
        uint32_t m_current_frame_in_flight = 0;
        uint32_t m_max_frames_in_flight = 2;
        Profile m_profile;
        std::vector<std::unique_ptr<Frame_context>> m_frame_contexts = {};
        std::array<VkFence, YGG_MAX_FRAMES_IN_FLIGHT> m_frame_fences = {};
    };
}
