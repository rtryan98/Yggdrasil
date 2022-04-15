// Copyright 2022 Robert Ryan. See Licence.md.

#include "ygg/vulkan/window_system_integration_win32.h"

#include "ygg/window/window_win32.h"

#include <volk.h>

#if defined(WIN32) || defined(_WIN32) || defined(__WIN32__) || defined(__NT__)
namespace ygg::vk
{
    Window_system_integration_win32::Window_system_integration_win32(const Window_win32& window)
        : m_window(window)
    {}

    std::vector<std::string> Window_system_integration_win32::query_required_instance_extensions() const
    {
        return std::vector<std::string>({ VK_KHR_SWAPCHAIN_EXTENSION_NAME });
    }

    std::vector<std::string> Window_system_integration_win32::query_required_device_extensions() const
    {
        return std::vector<std::string>({ VK_KHR_WIN32_SURFACE_EXTENSION_NAME, VK_KHR_SURFACE_EXTENSION_NAME });
    }

    VkSurfaceKHR Window_system_integration_win32::create_surface(VkInstance instance) const
    {
        VkWin32SurfaceCreateInfoKHR create_info = {
            .sType = VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR,
            .pNext = nullptr,
            .flags = 0,
            .hinstance = m_window.hinstance(),
            .hwnd = m_window.hwnd()
        };
        VkSurfaceKHR surface = VK_NULL_HANDLE;
        vkCreateWin32SurfaceKHR(instance, &create_info, nullptr, &surface);
        return surface;
    }

    bool Window_system_integration_win32::is_closed() const
    {
        return m_window.window_data().is_closed;
    }

    uint32_t Window_system_integration_win32::get_width() const
    {
        return m_window.window_data().width;
    }

    uint32_t Window_system_integration_win32::get_height() const
    {
        return m_window.window_data().height;
    }
}
#endif
