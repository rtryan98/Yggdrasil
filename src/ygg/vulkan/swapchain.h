// Copyright 2022 Robert Ryan. See Licence.md.

#pragma once

#include "ygg/vulkan/resource.h"
#include "ygg/vulkan/vk_forward_decl.h"

#include <span>
#include <vector>

namespace ygg::vk
{
    class Context;
    class Window_system_integration;

    /**
     * @brief VkSwapchainKHR wrapper.
    */
    class Swapchain
    {
    public:
        /**
         * @brief Constructs a Swapchain instance which binds to the given Context and WSI.
        */
        explicit Swapchain(const Context& context, const Window_system_integration& wsi);
        ~Swapchain();

        /**
         * @brief Attempts to acquire the next swapchain image index and signals the binary semaphore on success.
         * @return The VkResult value returned by vkAcquireNextImageKHR.
        */
        VkResult try_acquire_index(VkSemaphore semaphore);

        /**
         * @brief Attempts to acquire the next swapchain image index and signals the binary semaphore on success.
         * @details If the acquire was not possible due to a resize, minimize or similar non-fatal
         * failure, it will cause a flush and attempt to recreate the swapchain. On success, this
         * returns VK_SUCCESS. On failure, this returns any error that can be returned by vkAcquireNextImageKHR.
        */
        VkResult try_acquire_index_recreate_on_resize(VkSemaphore semaphore);

        /**
         * @brief Recreates the internal swapchain.
         * @details This should only be used when the swapchain is not in use by any command buffer.
        */
        void recreate();

        /**
         * @brief Attempts to present the currently acquired swapchain image to the given queue
         * once the given binary semaphore has been signalled.
         * @return Any VkResult that can be returned by vkQueuePresentKHR.
        */
        VkResult try_present(VkQueue queue, VkSemaphore await_semaphore) const;

        /**
         * @brief Attempts to present the currently acquired swapchain image to the given queue
         * once the given binary semaphores have been signalled.
         * @return Any VkResult that can be returned by vkQueuePresentKHR.
        */
        VkResult try_present(VkQueue queue, std::span<VkSemaphore> await_semaphores) const;

        /**
         * @brief Attempts to present the currently acquired swapchain image to the given queue
         * once the given binary semaphore has been signalled.
         * @details If the present was not successful due to a non-fatal failure, it will attempt
         * to recreate the Swapchain. If the recreation is not possible, it will return the error.
        */
        VkResult try_present_recreate_on_resize(VkQueue queue, VkSemaphore await_semaphore);

        /**
         * @brief Attempts to present the currently acquired swapchain image to the given queue
         * once the given binary semaphores have been signalled.
         * @details If the present was not successful due to a non-fatal failure, it will attempt
         * to recreate the Swapchain. If the recreation is not possible, it will return the error.
        */
        VkResult try_present_recreate_on_resize(VkQueue queue, std::span<VkSemaphore> await_semaphores);

        /**
         * @brief Returns a constructed Image primitive from the currently acquired swapchain image.
         * @details This should only be called after an Image has been acquired. Using a non-acquired
         * swapchain image or one acquired from a previous frame can cause problems during rendering,
         * presentation or both.
        */
        Image image() const;

    private:
        void post_swapchain_create();
        void cleanup();

    private:
        const Context& m_context;
        const Window_system_integration& m_wsi;
        VkSwapchainKHR m_swapchain = nullptr;
        VkFormat m_format = {};
        VkColorSpaceKHR m_color_space = {};
        VkImageUsage m_usage;
        uint32_t m_acquired_idx = ~0u;
        uint32_t m_width = 0;
        uint32_t m_height = 0;
        std::vector<VkImage> m_images = {};
        std::vector<VkImageView> m_image_views = {};
    };
}
