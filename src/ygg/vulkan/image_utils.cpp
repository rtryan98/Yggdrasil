// Copyright 2022 Robert Ryan. See Licence.md.

#include "ygg/vulkan/image_utils.h"

#include "ygg/vulkan/resource.h"

#include <volk.h>

namespace ygg::vk::img_utils
{
    VkImageSubresourceRange get_image_subresource_range(VkImageAspectFlags aspect_flags)
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

    VkImageSubresourceRange get_default_image_subresource_range(const Image_info& img_info)
    {
        return get_image_subresource_range(get_default_aspect_mask(img_info.format));
    }

    VkImageAspectFlags get_default_aspect_mask(VkFormat format)
    {
        switch (format) {
        default:                            return VK_IMAGE_ASPECT_COLOR_BIT;
        case VK_FORMAT_D16_UNORM:           ;
        case VK_FORMAT_X8_D24_UNORM_PACK32: ;
        case VK_FORMAT_D32_SFLOAT:          return VK_IMAGE_ASPECT_DEPTH_BIT;
        case VK_FORMAT_S8_UINT:             return VK_IMAGE_ASPECT_STENCIL_BIT;
        case VK_FORMAT_D16_UNORM_S8_UINT:   ;
        case VK_FORMAT_D24_UNORM_S8_UINT:   ;
        case VK_FORMAT_D32_SFLOAT_S8_UINT:  return VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT;
        case VK_FORMAT_UNDEFINED:           return 0;
        }
    }

    VkImageViewType get_default_image_view_type(const Image_info& img_info)
    {
        switch (img_info.type)
        {
        default: ;
        case VK_IMAGE_TYPE_1D:
            if (img_info.array_layers > 1)
                return VK_IMAGE_VIEW_TYPE_1D_ARRAY;
            else
                return VK_IMAGE_VIEW_TYPE_1D;
        case VK_IMAGE_TYPE_2D:
            if (img_info.array_layers > 1)
                return VK_IMAGE_VIEW_TYPE_2D_ARRAY;
            else
                return VK_IMAGE_VIEW_TYPE_2D;
        case VK_IMAGE_TYPE_3D:
            return VK_IMAGE_VIEW_TYPE_3D;
        }
    }

    VkImageCreateFlags get_default_image_create_flags(const Image_info& img_info)
    {
        VkImageCreateFlags flags = 0;
        if ((img_info.array_layers % 6 == 0) && (img_info.type == VK_IMAGE_TYPE_2D))
            flags |= VK_IMAGE_CREATE_CUBE_COMPATIBLE_BIT;
        if (img_info.type == VK_IMAGE_TYPE_3D)
            flags |= VK_IMAGE_CREATE_2D_ARRAY_COMPATIBLE_BIT;

        return flags;
    }
}
