// Copyright 2022 Robert Ryan. See Licence.md.

#pragma once

#include "ygg/vulkan/vk_forward_decl.h"

namespace ygg::vk::img_utils
{
    VkImageSubresourceRange get_full_image_subresource_range(VkImageAspectFlags aspect_flags);
}
