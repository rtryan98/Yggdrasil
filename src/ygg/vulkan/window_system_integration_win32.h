// Copyright 2022 Robert Ryan. See Licence.md.

#pragma once

#include "ygg/vulkan/window_system_integration.h"

typedef struct HINSTANCE__* HINSTANCE;
typedef struct HWND__* HWND;

namespace ygg
{
    class Window_win32;
}

namespace ygg::vk
{
    /**
     * @brief Win32 implementation of Vulkan WSI.
    */
    class Window_system_integration_win32 : public Window_system_integration
    {
    public:
        /**
         * @brief Creates a WSI-instance using a Win32 window.
         * @param window The window this WSI-instance is bound to.
        */
        Window_system_integration_win32(const Window_win32& window);

        /**
         * @brief Query required for the Context's instance creation.
        */
        virtual std::vector<std::string> query_required_instance_extensions() const override;

        /**
         * @brief Query required for the Context's device creation.
         * @details This will in most cases only return `VK_KHR_SWAPCHAIN_EXTENSION_NAME`.
        */
        virtual std::vector<std::string> query_required_device_extensions() const override;

        /**
         * @brief Surface creation required by the Context.
        */
        virtual VkSurfaceKHR create_surface(VkInstance) const override;

        /**
         * @brief Query whether or not the current windowing systems surface is closed.
        */
        virtual bool is_closed() const override;

        /**
         * @brief Query current windowing systems surface width.
        */
        virtual uint32_t get_width() const override;

        /**
         * @brief Query current windowing systems surface height.
        */
        virtual uint32_t get_height() const override;

    private:
        const Window_win32& m_window;
    };
}
