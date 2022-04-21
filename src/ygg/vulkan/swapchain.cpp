// Copyright 2022 Robert Ryan. See Licence.md.

#include "ygg/vulkan/swapchain.h"

#include "ygg/vulkan/context.h"
#include "ygg/vulkan/image_utils.h"
#include "ygg/vulkan/window_system_integration.h"

#include <array>
#include <volk.h>

namespace ygg::vk
{
    Swapchain::Swapchain(const Context& context, const Window_system_integration& wsi)
        : m_context(context), m_wsi(wsi),
        m_usage(VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT)
    {
        recreate();
    }

    Swapchain::~Swapchain()
    {
        cleanup();
    }

    VkResult Swapchain::try_acquire_index(VkSemaphore semaphore)
    {
        return vkAcquireNextImageKHR(m_context.device(), m_swapchain, ~0ull, semaphore, VK_NULL_HANDLE, &m_acquired_idx);
    }

    // TODO: This should be more custom than that.
    VkPresentModeKHR select_present_mode(const std::span<VkPresentModeKHR>& modes)
    {
        for (const auto& mode : modes) {
            if (mode == VK_PRESENT_MODE_MAILBOX_KHR) return VK_PRESENT_MODE_MAILBOX_KHR;
        }
        return VK_PRESENT_MODE_FIFO_KHR;
    }

    // TODO: This is very makeshift. Allow HDR and other format/colorspace combos.
    VkSurfaceFormatKHR select_image_format_and_color_space(const std::span<VkSurfaceFormatKHR>& formats)
    {
        for (const auto& sf : formats) {
            if (sf.format == VK_FORMAT_B8G8R8A8_SRGB && sf.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) {
                return sf;
            }
            else if (sf.format == VK_FORMAT_R8G8B8A8_SRGB && sf.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) {
                return sf;
            }
        }
        // TODO: error?
        return VkSurfaceFormatKHR();
    }

    VkResult Swapchain::try_acquire_index_recreate_on_resize(VkSemaphore semaphore)
    {
        do {
            auto result = try_acquire_index(semaphore);
            if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR) {
                m_context.device_wait_idle();
                recreate();
                continue;
            }
            else if (result != VK_SUCCESS) {
                return result;
            }
        } while (false);
        return VK_SUCCESS;
    }

    void Swapchain::recreate()
    {
        if (m_wsi.is_closed()) {
            return;
        }
        cleanup();

        VkSurfaceCapabilitiesKHR caps = {};
        // TODO: add VK_CHECK
        vkGetPhysicalDeviceSurfaceCapabilitiesKHR(m_context.physical_device(), m_context.surface(), &caps);

        uint32_t present_mode_count = 0;
        // TODO: add VK_CHECK
        vkGetPhysicalDeviceSurfacePresentModesKHR(m_context.physical_device(), m_context.surface(),
            &present_mode_count, nullptr);
        std::vector<VkPresentModeKHR> available_present_modes = {};
        available_present_modes.resize(present_mode_count);
        vkGetPhysicalDeviceSurfacePresentModesKHR(m_context.physical_device(), m_context.surface(),
            &present_mode_count, available_present_modes.data());

        uint32_t format_count = 0;
        vkGetPhysicalDeviceSurfaceFormatsKHR(m_context.physical_device(), m_context.surface(),
            &format_count, nullptr);
        std::vector<VkSurfaceFormatKHR> formats = {};
        formats.resize(format_count);
        vkGetPhysicalDeviceSurfaceFormatsKHR(m_context.physical_device(), m_context.surface(),
            &format_count, formats.data());
        auto surface_format = select_image_format_and_color_space(formats);
        m_format = surface_format.format;
        m_color_space = surface_format.colorSpace;

        // TODO: default queue?
        uint32_t queue_family_index = m_context.graphics_queue().queue_family_index;

        if (caps.currentExtent.width != 0xFFFFFFFF && caps.currentExtent.height != 0xFFFFFFFF) {
            m_width = caps.currentExtent.width;
            m_height = caps.currentExtent.height;
        }
        else {
            m_width = m_wsi.get_width();
            m_height = m_wsi.get_height();
        }

        VkSwapchainCreateInfoKHR swapchain_create_info = {
            .sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR,
            .pNext = nullptr,
            .flags = 0,
            .surface = m_context.surface(),
            .minImageCount = (caps.maxImageCount > 2u) ? 3u : 2u,
            .imageFormat = surface_format.format,
            .imageColorSpace = surface_format.colorSpace,
            .imageExtent = { .width = m_width, .height = m_height },
            .imageArrayLayers = 1,
            .imageUsage = m_usage,
            .imageSharingMode = VK_SHARING_MODE_EXCLUSIVE,
            .queueFamilyIndexCount = 1,
            .pQueueFamilyIndices = &queue_family_index,
            .preTransform = VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR,
            .compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR,
            .presentMode = select_present_mode(available_present_modes),
            .clipped = VK_TRUE,
            .oldSwapchain = VK_NULL_HANDLE
        };
        // TODO: add VK_CHECK
        vkCreateSwapchainKHR(m_context.device(), &swapchain_create_info, nullptr, &m_swapchain);

        post_swapchain_create();
    }

    VkResult Swapchain::try_present(VkQueue queue, VkSemaphore await_semaphore) const
    {
        auto arr = std::to_array<VkSemaphore> ({ await_semaphore });
        return try_present(queue, arr);;
    }

    VkResult Swapchain::try_present(VkQueue queue, std::span<VkSemaphore> await_semaphores) const
    {
        VkPresentInfoKHR present_info = {
            .sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR,
            .pNext = nullptr,
            .waitSemaphoreCount = uint32_t(await_semaphores.size()),
            .pWaitSemaphores = await_semaphores.data(),
            .swapchainCount = 1,
            .pSwapchains = &m_swapchain,
            .pImageIndices = &m_acquired_idx,
            .pResults = nullptr
        };
        return vkQueuePresentKHR(queue, &present_info);
    }

    VkResult Swapchain::try_present_recreate_on_resize(VkQueue queue, VkSemaphore await_semaphore)
    {
        auto arr = std::to_array<VkSemaphore>({ await_semaphore });
        return try_present_recreate_on_resize(queue, arr);
    }

    VkResult Swapchain::try_present_recreate_on_resize(VkQueue queue, std::span<VkSemaphore> await_semaphores)
    {
        do {
            auto result = try_present(queue, await_semaphores);
            if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR) {
                m_context.device_wait_idle();
                recreate();
            }
            else if (result != VK_SUCCESS) {
                return result;
            }
        } while (false);
        return VK_SUCCESS;
    }

    Image Swapchain::image() const
    {
        return {
            .allocated_image = {
                .handle = m_images[m_acquired_idx],
                .default_view = m_image_views[m_acquired_idx],
                .allocation = nullptr
            },
            .info = {
                .width = m_width,
                .height = m_height,
                .depth = 1,
                .mip_levels = 1,
                .array_layers = 1,
                .format = m_format,
                .usage = m_usage,
                .type = VK_IMAGE_TYPE_2D
            }
        };
    }

    void Swapchain::post_swapchain_create()
    {
        uint32_t image_count = 0;
        // TODO: add VK_CHECK
        vkGetSwapchainImagesKHR(m_context.device(), m_swapchain, &image_count, nullptr);
        m_images.resize(image_count);
        vkGetSwapchainImagesKHR(m_context.device(), m_swapchain, &image_count, m_images.data());
        m_image_views.resize(image_count);
        for (uint32_t i = 0; i < image_count; i++) {
            VkImageViewUsageCreateInfo usage_create_info = {
                .sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_USAGE_CREATE_INFO,
                .pNext = nullptr,
                .usage = m_usage
            };
            VkImageViewCreateInfo view_create_info = {
                .sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
                .pNext = &usage_create_info,
                .flags = 0,
                .image = m_images[i],
                .viewType = VK_IMAGE_VIEW_TYPE_2D,
                .format = VK_FORMAT_B8G8R8A8_SRGB,
                .components = {},
                .subresourceRange = img_utils::get_image_subresource_range(VK_IMAGE_ASPECT_COLOR_BIT)
            };
            // TODO: add VK_CHECK
            vkCreateImageView(m_context.device(), &view_create_info, nullptr, &m_image_views[i]);
        }
    }

    void Swapchain::cleanup()
    {
        if (m_swapchain != VK_NULL_HANDLE) {
            vkDestroySwapchainKHR(m_context.device(), m_swapchain, nullptr);
            m_swapchain = VK_NULL_HANDLE;
        }
        for (auto i = 0; i < m_image_views.size(); i++) {
            if (m_image_views[i] != VK_NULL_HANDLE) {
                vkDestroyImageView(m_context.device(), m_image_views[i], nullptr);
                m_image_views[i] = VK_NULL_HANDLE;
            }
        }
        m_image_views.clear();
        m_images.clear();
    }
}
