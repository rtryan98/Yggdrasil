// Copyright 2022 Robert Ryan. See Licence.md.

#include "ygg/vulkan/image_utils.h"

#include <volk.h>

namespace ygg::vk::img_utils
{
    VkImageSubresourceRange get_full_image_subresource_range(VkImageAspectFlags aspect_flags)
    {
        VkImageSubresourceRange result = {
            .aspectMask = aspect_flags,
            .baseMipLevel = 0,
            .levelCount = VK_REMAINING_MIP_LEVELS,
            .baseArrayLayer = 0,
            .layerCount = VK_REMAINING_ARRAY_LAYERS
        };
        return result;
    }
}
