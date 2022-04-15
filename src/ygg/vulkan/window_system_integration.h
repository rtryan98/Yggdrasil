// Copyright 2022 Robert Ryan. See Licence.md.

#pragma once

#include "ygg/vulkan/vk_forward_decl.h"

#include <cstdint>
#include <string>
#include <vector>

namespace ygg::vk
{
    /**
     * @brief Base class to integrate Vulkan WSI with.
     * @details Using this class directly will create a headless context.
     * Deriving classes must implement all functionality and hook them up to the windowing system.
    */
    class Window_system_integration
    {
    public:
        /**
         * @brief Query required for the Context's instance creation.
        */
        virtual std::vector<std::string> query_required_instance_extensions() const { return {}; };

        /**
         * @brief Query required for the Context's device creation.
         * @details This will in most cases only return `VK_KHR_SWAPCHAIN_EXTENSION_NAME`.
        */
        virtual std::vector<std::string> query_required_device_extensions() const { return {}; };

        /**
         * @brief Surface creation required by the Context.
        */
        virtual VkSurfaceKHR create_surface(VkInstance) const { return nullptr; };

        /**
         * @brief Query whether or not the current windowing systems surface is closed.
        */
        virtual bool is_closed() const { return true; };

        /**
         * @brief Query current windowing systems surface width.
        */
        virtual uint32_t get_width() const { return 0; };

        /**
         * @brief Query current windowing systems surface height.
        */
        virtual uint32_t get_height() const { return 0; };
    };
}
