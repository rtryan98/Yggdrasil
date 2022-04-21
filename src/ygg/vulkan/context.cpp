// Copyright 2022 Robert Ryan. See Licence.md.

#include "ygg/vulkan/context.h"

#include "ygg/vulkan/graphics_command_buffer.h"
#include "ygg/vulkan/window_system_integration.h"

#define VOLK_IMPLEMENTATION
#include <volk.h>

#pragma warning(push, 0)
#include "ygg/vulkan/generated/vulkan_profiles.hpp"
#define VMA_IMPLEMENTATION
#include <vk_mem_alloc.h>
#pragma warning(pop)

namespace ygg::vk
{
    Frame_context::Frame_context(const Context& context)
        : m_context(context),
        m_linear_host_resource_allocator_provider(m_context.allocator()),
        m_graphics_command_buffer_recycler(m_context.device(), m_context.graphics_queue().queue_family_index,
            VK_COMMAND_POOL_CREATE_TRANSIENT_BIT),
        m_async_compute_command_buffer_recycler(m_context.device(), m_context.graphics_queue().queue_family_index,
            VK_COMMAND_POOL_CREATE_TRANSIENT_BIT)
    {}

    Frame_context::~Frame_context()
    {
        start_frame();
    }

    void Frame_context::start_frame()
    {
        destroy_all_zombies();
        m_linear_host_resource_allocator_provider.reset();
        m_graphics_command_buffer_recycler.reset();
        m_async_compute_command_buffer_recycler.reset();
    }

    Linear_host_resource_allocator& Frame_context::acquire_linear_host_resource_allocator()
    {
        return m_linear_host_resource_allocator_provider.create_allocator();
    }

    Compute_command_buffer Frame_context::acquire_async_compute_command_buffer()
    {
        auto cmdbuf = m_graphics_command_buffer_recycler.get_or_allocate();
        m_graphics_command_buffer_recycler.recycle(cmdbuf);
        return Compute_command_buffer(cmdbuf, acquire_linear_host_resource_allocator(), m_context.current_frame_in_flight(),
            m_context.compute_queue().queue_family_index);
    }

    Graphics_command_buffer Frame_context::acquire_graphics_command_buffer()
    {
        auto cmdbuf = m_graphics_command_buffer_recycler.get_or_allocate();
        m_graphics_command_buffer_recycler.recycle(cmdbuf);
        return Graphics_command_buffer(cmdbuf, acquire_linear_host_resource_allocator(), m_context.current_frame_in_flight(),
            m_context.graphics_queue().queue_family_index );
    }

    void Frame_context::destroy_all_zombies()
    {
        for (auto z : m_zombie_semaphores) {
            vkDestroySemaphore(m_context.device(), z, nullptr);
        }
        m_zombie_semaphores.clear();
        for (auto z : m_zombie_fences) {
            vkDestroyFence(m_context.device(), z, nullptr);
        }
        m_zombie_fences.clear();
    }

    Context::Context(const Window_system_integration& wsi)
        : m_wsi(wsi)
    {
        if (volkInitialize() != VK_SUCCESS) {
            printf("Volk could not be initialized!"); // TODO: logging?
            std::abort();
        }

        auto wsi_instance_extensions = wsi.query_required_instance_extensions();
        std::vector<const char*> instance_extensions = {};
        for (const auto& wsi_extension : wsi_instance_extensions) {
            instance_extensions.push_back(wsi_extension.c_str());
        }
        std::vector<const char*> instance_layers = {
#if YGG_VULKAN_VALIDATION
            "VK_LAYER_KHRONOS_validation",
#endif
        };

        VpProfileProperties tier_1_profile_props = { VP_YGG_TIER_1_NAME, VP_YGG_TIER_1_SPEC_VERSION };
        VpProfileProperties tier_2_profile_props = { VP_YGG_TIER_2_NAME, VP_YGG_TIER_2_SPEC_VERSION };
        VpProfileProperties tier_3_profile_props = { VP_YGG_TIER_3_NAME, VP_YGG_TIER_3_SPEC_VERSION };

        VkBool32 tier_1_instance_supported = false;
        vpGetInstanceProfileSupport(nullptr, &tier_1_profile_props, &tier_1_instance_supported);
        VkBool32 tier_2_instance_supported = false;
        vpGetInstanceProfileSupport(nullptr, &tier_2_profile_props, &tier_2_instance_supported);
        VkBool32 tier_3_instance_supported = false;
        vpGetInstanceProfileSupport(nullptr, &tier_3_profile_props, &tier_3_instance_supported);

        VkApplicationInfo application_info = {
            .sType = VK_STRUCTURE_TYPE_APPLICATION_INFO,
            .pNext = nullptr,
            .pApplicationName = "Yggdrasil-Application",
            .applicationVersion = VK_MAKE_VERSION(1,0,0),
            .pEngineName = "Yggdrasil",
            .engineVersion = VK_MAKE_VERSION(1,0,0),
            .apiVersion = VK_API_VERSION_1_3
        };
        VkInstanceCreateInfo instance_create_info = {
            .sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO,
            .pNext = nullptr,
            .flags = 0,
            .pApplicationInfo = &application_info,
            .enabledLayerCount = uint32_t(instance_layers.size()),
            .ppEnabledLayerNames = instance_layers.data(),
            .enabledExtensionCount = uint32_t(instance_extensions.size()),
            .ppEnabledExtensionNames = instance_extensions.data()
        };

        VpInstanceCreateInfo profile_instance_create_info = {
            .pCreateInfo = &instance_create_info,
            .flags = VP_INSTANCE_CREATE_MERGE_EXTENSIONS_BIT
        };
        if (tier_1_instance_supported) {
            profile_instance_create_info.pProfile = &tier_1_profile_props;
        }
        if (tier_2_instance_supported) {
            profile_instance_create_info.pProfile = &tier_2_profile_props;
        }
        if (tier_3_instance_supported) {
            profile_instance_create_info.pProfile = &tier_3_profile_props;
        }

        // TODO: add VK_CHECK
        vpCreateInstance(&profile_instance_create_info, nullptr, &m_instance);
        volkLoadInstance(m_instance);
        m_surface = wsi.create_surface(m_instance);

        uint32_t physical_device_count = 0;
        vkEnumeratePhysicalDevices(m_instance, &physical_device_count, nullptr);
        std::vector<VkPhysicalDevice> physical_devices(physical_device_count);
        vkEnumeratePhysicalDevices(m_instance, &physical_device_count, physical_devices.data());

        VkPhysicalDevice fallback{ physical_devices[0] };
        VkPhysicalDevice selected{ fallback };
        for (uint32_t i = 0; i < physical_devices.size(); i++)
        {
            VkPhysicalDeviceProperties properties{};
            vkGetPhysicalDeviceProperties(physical_devices[i], &properties);
            if (properties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU)
            {
                selected = physical_devices[i];
            }
        }
        m_physical_device = selected;

        uint32_t queue_family_count = 0;
        vkGetPhysicalDeviceQueueFamilyProperties(m_physical_device, &queue_family_count, nullptr);
        std::vector<VkQueueFamilyProperties> queue_family_properties(queue_family_count);
        vkGetPhysicalDeviceQueueFamilyProperties(m_physical_device, &queue_family_count, queue_family_properties.data());
        std::vector<VkDeviceQueueCreateInfo> queue_create_infos{};

        constexpr auto queue_family_supports_graphics
        {
            [](const VkQueueFamilyProperties& properties) -> bool {
                return properties.queueFlags & VK_QUEUE_GRAPHICS_BIT;
            }
        };

        constexpr auto queue_family_supports_compute
        {
            [](const VkQueueFamilyProperties& properties) -> bool {
                return properties.queueFlags & VK_QUEUE_COMPUTE_BIT;
            }
        };

        constexpr auto queue_family_supports_transfer
        {
            [](const VkQueueFamilyProperties& properties) -> bool {
                return properties.queueFlags & VK_QUEUE_TRANSFER_BIT;
            }
        };

        VkDeviceQueueCreateInfo queue_create_info{ VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO };
        queue_create_info.queueCount = 1;
        float_t prio{ 1.0f };
        queue_create_info.pQueuePriorities = &prio;

        bool graphics_queue_found{ false };
        bool compute_queue_found{ false };
        bool transfer_queue_found{ false };
        VkBool32 main_queue_supports_present{ false };

        for (uint32_t i = 0; i < queue_family_properties.size(); i++)
        {
            queue_create_info.queueFamilyIndex = i;
            if (queue_family_supports_graphics(queue_family_properties[i]) &&
                queue_family_supports_compute(queue_family_properties[i]) &&
                queue_family_supports_transfer(queue_family_properties[i]) &&
                !graphics_queue_found)
            {
                graphics_queue_found = true;
                m_graphics_queue.queue_family_index = i;
                queue_create_infos.push_back(queue_create_info);
                vkGetPhysicalDeviceSurfaceSupportKHR(m_physical_device, i, m_surface, &main_queue_supports_present);
            }
            else if (queue_family_supports_compute(queue_family_properties[i]) &&
                queue_family_supports_transfer(queue_family_properties[i]) &&
                !queue_family_supports_graphics(queue_family_properties[i]) &&
                !compute_queue_found)
            {
                compute_queue_found = true;
                m_compute_queue.queue_family_index = i;
                queue_create_infos.push_back(queue_create_info);
            }
            else if (queue_family_supports_transfer(queue_family_properties[i]) &&
                !queue_family_supports_compute(queue_family_properties[i]) &&
                !queue_family_supports_graphics(queue_family_properties[i]) &&
                !transfer_queue_found)
            {
                transfer_queue_found = true;
                m_transfer_queue.queue_family_index = i;
                queue_create_infos.push_back(queue_create_info);
            }
        }

        auto wsi_device_extensions = m_wsi.query_required_device_extensions();
        std::vector<const char*> extensions = {};
        for (const auto& ext : wsi_device_extensions) {
            extensions.push_back(ext.c_str());
        }

        VkBool32 tier_1_device_supported = false;
        vpGetPhysicalDeviceProfileSupport(m_instance, m_physical_device, &tier_1_profile_props, &tier_1_device_supported);
        VkBool32 tier_2_device_supported = false;
        vpGetPhysicalDeviceProfileSupport(m_instance, m_physical_device, &tier_2_profile_props, &tier_2_device_supported);
        VkBool32 tier_3_device_supported = false;
        vpGetPhysicalDeviceProfileSupport(m_instance, m_physical_device, &tier_3_profile_props, &tier_3_device_supported);

        VkDeviceCreateInfo device_create_info = {
            .sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO,
            .pNext = nullptr,
            .flags = 0,
            .queueCreateInfoCount = uint32_t(queue_create_infos.size()),
            .pQueueCreateInfos = queue_create_infos.data(),
            .enabledLayerCount = 0,
            .ppEnabledLayerNames = nullptr,
            .enabledExtensionCount = uint32_t(extensions.size()),
            .ppEnabledExtensionNames = extensions.data(),
            .pEnabledFeatures = nullptr
        };

        VpDeviceCreateInfo profile_device_create_info = {
            .pCreateInfo = &device_create_info,
            .flags = VP_DEVICE_CREATE_DISABLE_ROBUST_ACCESS | VP_DEVICE_CREATE_MERGE_EXTENSIONS_BIT
        };

        if (tier_1_device_supported) {
            profile_device_create_info.pProfile = &tier_1_profile_props;
            m_profile = Profile::Tier_1;
        }
        if (tier_2_device_supported) {
            profile_device_create_info.pProfile = &tier_2_profile_props;
            m_profile = Profile::Tier_2;
        }
        if (tier_3_device_supported) {
            profile_device_create_info.pProfile = &tier_3_profile_props;
            m_profile = Profile::Tier_3;
        }

        // TODO: add VK_CHECK
        vpCreateDevice(m_physical_device, &profile_device_create_info, nullptr, &m_device);
        volkLoadDevice(m_device);

        if (graphics_queue_found) {
            vkGetDeviceQueue(m_device, m_graphics_queue.queue_family_index, 0, &m_graphics_queue.queue);
        }
        else {
            printf("There is no main queue available. This is currently not supported."); // TODO: logging?
            std::abort();
        }
        if (compute_queue_found) {
            vkGetDeviceQueue(m_device, m_compute_queue.queue_family_index, 0, &m_compute_queue.queue);
        }
        else {
            m_compute_queue = m_graphics_queue;
        }
        if (transfer_queue_found) {
            vkGetDeviceQueue(m_device, m_transfer_queue.queue_family_index, 0, &m_transfer_queue.queue);
        }
        else {
            m_transfer_queue = m_graphics_queue;
        }

        VmaVulkanFunctions allocator_vk_functions = {
            .vkGetInstanceProcAddr = vkGetInstanceProcAddr,
            .vkGetDeviceProcAddr = vkGetDeviceProcAddr
        };
        VmaAllocatorCreateInfo allocator_create_info = {
            .flags = 0,
            .physicalDevice = m_physical_device,
            .device = m_device,
            .pVulkanFunctions = &allocator_vk_functions,
            .instance = m_instance,
            .vulkanApiVersion = VK_API_VERSION_1_3
        };
        vmaCreateAllocator(&allocator_create_info, &m_allocator);

        m_frame_contexts.reserve(m_max_frames_in_flight);
        for (uint32_t i = 0; i < m_max_frames_in_flight; i++) {
            m_frame_contexts.emplace_back(std::make_unique<Frame_context>(*this));
            
            VkFenceCreateInfo fence_info = {
                .sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO,
                .pNext = nullptr,
                .flags = VK_FENCE_CREATE_SIGNALED_BIT
            };
            vkCreateFence(m_device, &fence_info, nullptr, &m_frame_fences[i]);
        }
    }

    Context::~Context()
    {
        device_wait_idle();

        for (uint32_t i = 0; i < m_max_frames_in_flight; i++) {
            vkDestroyFence(m_device, m_frame_fences[i], nullptr);
        }

        m_frame_contexts.clear();
        vmaDestroyAllocator(m_allocator);
        vkDestroyDevice(m_device, nullptr);
        vkDestroySurfaceKHR(m_instance, m_surface, nullptr);
        vkDestroyInstance(m_instance, nullptr); // TODO: add VK_CHECK
    }

    void Context::end_frame()
    {
        m_current_frame_in_flight += 1;
        m_current_frame_in_flight %= m_max_frames_in_flight;
    }

    void Context::device_wait_idle() const
    {
        vkDeviceWaitIdle(m_device);
    }

    void Context::begin_frame()
    {
        vkWaitForFences(m_device, 1, &m_frame_fences[m_current_frame_in_flight], VK_TRUE, ~0ull);
        vkResetFences(m_device, 1, &m_frame_fences[m_current_frame_in_flight]);
        frame_context().start_frame();
    }

    Image Context::create_image(const Image_info& info, uint32_t initial_queue_family_index) const
    {
        return vk::create_image(info, initial_queue_family_index, m_allocator, m_device);
    }

    Buffer Context::create_buffer(const Buffer_info& info, uint32_t initial_queue_family_index) const
    {
        return vk::create_buffer(info, initial_queue_family_index, m_allocator, m_max_frames_in_flight);
    }

    VkSemaphore Context::create_binary_semaphore() const
    {
        VkSemaphore result = VK_NULL_HANDLE;
        VkSemaphoreTypeCreateInfo type_info = {
            .sType = VK_STRUCTURE_TYPE_SEMAPHORE_TYPE_CREATE_INFO,
            .pNext = nullptr,
            .semaphoreType = VK_SEMAPHORE_TYPE_BINARY,
            .initialValue = 0
        };
        VkSemaphoreCreateInfo info = {
            .sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO,
            .pNext = &type_info,
            .flags = 0
        };
        vkCreateSemaphore(m_device, &info, nullptr, &result);
        return result;
    }

    void Context::destroy_image(Image& image) const
    {
        vk::destroy_image(image, m_allocator, m_device);
    }

    void Context::destroy_buffer(Buffer& buffer) const
    {
        vk::destroy_buffer(buffer, m_allocator, m_max_frames_in_flight);
    }

    VkResult Context::submit_simple(VkQueue queue, VkCommandBuffer cmdbuf,
        VkSemaphore await_sema, VkSemaphore signal_sema, VkFence signal_fence)
    {
        VkSemaphoreSubmitInfo wait_info = {
            .sType = VK_STRUCTURE_TYPE_SEMAPHORE_SUBMIT_INFO,
            .pNext = nullptr,
            .semaphore = await_sema,
            .value = 0,
            .stageMask = VK_PIPELINE_STAGE_2_NONE,
            .deviceIndex = 0
        };
        VkCommandBufferSubmitInfo cmd_info = {
            .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_SUBMIT_INFO,
            .pNext = nullptr,
            .commandBuffer = cmdbuf,
            .deviceMask = 1
        };
        VkSemaphoreSubmitInfo signal_info = {
            .sType = VK_STRUCTURE_TYPE_SEMAPHORE_SUBMIT_INFO,
            .pNext = nullptr,
            .semaphore = signal_sema,
            .value = 0,
            .stageMask = VK_PIPELINE_STAGE_2_NONE,
            .deviceIndex = 0
        };
        VkSubmitInfo2 submit_info = {
            .sType = VK_STRUCTURE_TYPE_SUBMIT_INFO_2,
            .pNext = nullptr,
            .waitSemaphoreInfoCount = await_sema == VK_NULL_HANDLE ? 0u : 1u,
            .pWaitSemaphoreInfos = &wait_info,
            .commandBufferInfoCount = 1,
            .pCommandBufferInfos = &cmd_info,
            .signalSemaphoreInfoCount = signal_sema == VK_NULL_HANDLE ? 0u : 1u,
            .pSignalSemaphoreInfos = &signal_info
        };
        return vkQueueSubmit2(queue, 1, &submit_info, signal_fence);
    }
}
