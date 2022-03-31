// Copyright 2022 Robert Ryan. See Licence.md.

#pragma once

#include "ygg/vulkan/vk_forward_decl.h"

namespace ygg::vk
{
    struct Image_info;

    namespace img_utils
    {
        /**
         * @brief Creates a `VkImageSubresourceRange` for an entire image.
         * @details Selects all mip levels and all array layers.
         * @param aspect_flags Image aspect flags to use for the subresource range.
         * @return A `VkImageAspectFlags` for the full image with the given `VkImageAspectFlags`.
        */
        VkImageSubresourceRange get_image_subresource_range(VkImageAspectFlags aspect_flags);

        /**
         * @brief Convenience function for speeding up creating a subresource range for an image.
         * @details Equivalent to calling `get_image_subresource_range(get_default_aspect_mask(img_info.format))`.
         * @param img_info The `Image_info` to derive the `VkImageSubresourceRange` from.
         * @return A `VkImageSubresourceRange` for the full image derived from the given `Image_info`.
        */
        VkImageSubresourceRange get_default_image_subresource_range(const Image_info& img_info);

        /**
         * @brief Convenience function for speeding up selection of the aspect mask.
         * @details Returns `VK_IMAGE_ASPECT_COLOR_BIT` for all formats except depth / stencil
         * and undefined formats. Doesn't do any plane selection for multi-planar formats. Does
         * not split up depth-stencil aspects in combined depth-stencil formats.
         * @param format The format from which to derive the `VkImageAspectFlags` from.
         * @return `VkImageAspectFlags` for the image derived from the given `VkFormat`.
        */
        VkImageAspectFlags get_default_aspect_mask(VkFormat format);

        /**
         * @brief Convenience function for speeding up selecting the image view type.
         * @details Does not return a `VkImageView` of type `VK_IMAGE_VIEW_TYPE_CUBE`
         * or `VK_IMAGE_VIEW_TYPE_CUBE_ARRAY`.
         * @param img_info The `Image_info` to derive the `VkImageViewType` from.
         * @return A `VkImageViewType` for the image derived from the given `Image_info`.
        */
        VkImageViewType get_default_image_view_type(const Image_info& img_info);

        /**
         * @brief Convenience function for speeding up selecting `VkImageCreateFlags`.
         * @details Returns `VK_IMAGE_CREATE_CUBE_COMPATIBLE_BIT` for 2D-images that
         * use multiples of 6 layers and `VK_IMAGE_CREATE_2D_ARRAY_COMPATIBLE_BIT` for
         * 3D images.
         * @param img_info The `Image_info` to derive the `VkImageCreateFlags` from.
         * @return `VkImageCreateFlags` for the image derived from the given `Image_info`.
        */
        VkImageCreateFlags get_default_image_create_flags(const Image_info& img_info);
    }
}
