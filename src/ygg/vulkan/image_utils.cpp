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

    Format_info get_format_info(VkFormat format)
    {
        using enum Format_type;
        switch (format)
        {
        default:                                 ; //     Format                              Type           Name                         red    green  blue   alpha  depth  stncl  signed srgb   size block
        case VK_FORMAT_UNDEFINED:                return { VK_FORMAT_UNDEFINED,                Undefined,     "UNDEFINED",                 false, false, false, false, false, false, false, false, 0,   0 };
        case VK_FORMAT_R4G4_UNORM_PACK8:         return { VK_FORMAT_R4G4_UNORM_PACK8,         Normalized,    "R4G4_UNORM_PACK8",          true,  true,  false, false, false, false, false, false, 1,   1 };
        case VK_FORMAT_R4G4B4A4_UNORM_PACK16:    return { VK_FORMAT_R4G4B4A4_UNORM_PACK16,    Normalized,    "R4G4B4A4_UNORM_PACK16",     true,  true,  true,  true,  false, false, false, false, 2,   1 };
        case VK_FORMAT_B4G4R4A4_UNORM_PACK16:    return { VK_FORMAT_B4G4R4A4_UNORM_PACK16,    Normalized,    "B4G4R4A4_UNORM_PACK16",     true,  true,  true,  true,  false, false, false, false, 2,   1 };
        case VK_FORMAT_R5G5B5A1_UNORM_PACK16:    return { VK_FORMAT_R5G5B5A1_UNORM_PACK16,    Normalized,    "R5G5B5A1_UNORM_PACK16",     true,  true,  true,  true,  false, false, false, false, 2,   1 };
        case VK_FORMAT_B5G5R5A1_UNORM_PACK16:    return { VK_FORMAT_B5G5R5A1_UNORM_PACK16,    Normalized,    "B5G5R5A1_UNORM_PACK16",     true,  true,  true,  true,  false, false, false, false, 2,   1 };
        case VK_FORMAT_A1R5G5B5_UNORM_PACK16:    return { VK_FORMAT_A1R5G5B5_UNORM_PACK16,    Normalized,    "A1R5G5B5_UNORM_PACK16",     true,  true,  true,  true,  false, false, false, false, 2,   1 };
        //                                                Format                              Type           Name                         red    green  blue   alpha  depth  stncl  signed srgb   size block
        case VK_FORMAT_R8_UNORM:                 return { VK_FORMAT_R8_UNORM,                 Normalized,    "R8_UNORM",                  true,  false, false, false, false, false, false, false, 1,   1 };
        case VK_FORMAT_R8_SNORM:                 return { VK_FORMAT_R8_SNORM,                 Normalized,    "R8_SNORM",                  true,  false, false, false, false, false, true,  false, 1,   1 };
        case VK_FORMAT_R8_UINT:                  return { VK_FORMAT_R8_UINT,                  Int,           "R8_UINT",                   true,  false, false, false, false, false, false, false, 1,   1 };
        case VK_FORMAT_R8_SINT:                  return { VK_FORMAT_R8_SINT,                  Int,           "R8_SINT",                   true,  false, false, false, false, false, true,  false, 1,   1 };
        case VK_FORMAT_R8_SRGB:                  return { VK_FORMAT_R8_SRGB,                  Normalized,    "R8_SRGB",                   true,  false, false, false, false, false, false, true,  1,   1 };
        //                                                Format                              Type           Name                         red    green  blue   alpha  depth  stncl  signed srgb   size block
        case VK_FORMAT_R8G8_UNORM:               return { VK_FORMAT_R8G8_UNORM,               Normalized,    "R8G8_UNORM",                true,  true,  false, false, false, false, false, false, 2,   1 };
        case VK_FORMAT_R8G8_SNORM:               return { VK_FORMAT_R8G8_SNORM,               Normalized,    "R8G8_SNORM",                true,  true,  false, false, false, false, true,  false, 2,   1 };
        case VK_FORMAT_R8G8_UINT:                return { VK_FORMAT_R8G8_UINT,                Int,           "R8G8_UINT",                 true,  true,  false, false, false, false, false, false, 2,   1 };
        case VK_FORMAT_R8G8_SINT:                return { VK_FORMAT_R8G8_SINT,                Int,           "R8G8_SINT",                 true,  true,  false, false, false, false, true,  false, 2,   1 };
        case VK_FORMAT_R8G8_SRGB:                return { VK_FORMAT_R8G8_SRGB,                Normalized,    "R8G8_SRGB",                 true,  true,  false, false, false, false, false, true,  2,   1 };
        //                                                Format                              Type           Name                         red    green  blue   alpha  depth  stncl  signed srgb   size block
        case VK_FORMAT_R8G8B8A8_UNORM:           return { VK_FORMAT_R8G8B8A8_UNORM,           Normalized,    "R8G8B8A8_UNORM",            true,  true,  true,  true,  false, false, false, false, 4,   1 };
        case VK_FORMAT_R8G8B8A8_SNORM:           return { VK_FORMAT_R8G8B8A8_SNORM,           Normalized,    "R8G8B8A8_SNORM",            true,  true,  true,  true,  false, false, true,  false, 4,   1 };
        case VK_FORMAT_R8G8B8A8_UINT:            return { VK_FORMAT_R8G8B8A8_UINT,            Int,           "R8G8B8A8_UINT",             true,  true,  true,  true,  false, false, false, false, 4,   1 };
        case VK_FORMAT_R8G8B8A8_SINT:            return { VK_FORMAT_R8G8B8A8_SINT,            Int,           "R8G8B8A8_SINT",             true,  true,  true,  true,  false, false, true,  false, 4,   1 };
        case VK_FORMAT_R8G8B8A8_SRGB:            return { VK_FORMAT_R8G8B8A8_SRGB,            Normalized,    "R8G8B8A8_SRGB",             true,  true,  true,  true,  false, false, false, true,  4,   1 };
        //                                                Format                              Type           Name                         red    green  blue   alpha  depth  stncl  signed srgb   size block
        case VK_FORMAT_B8G8R8A8_UNORM:           return { VK_FORMAT_B8G8R8A8_UNORM,           Normalized,    "B8G8R8A8_UNORM",            true,  true,  true,  true,  false, false, false, false, 4,   1 };
        case VK_FORMAT_B8G8R8A8_SNORM:           return { VK_FORMAT_B8G8R8A8_SNORM,           Normalized,    "B8G8R8A8_SNORM",            true,  true,  true,  true,  false, false, true,  false, 4,   1 };
        case VK_FORMAT_B8G8R8A8_UINT:            return { VK_FORMAT_B8G8R8A8_UINT,            Int,           "B8G8R8A8_UINT",             true,  true,  true,  true,  false, false, false, false, 4,   1 };
        case VK_FORMAT_B8G8R8A8_SINT:            return { VK_FORMAT_B8G8R8A8_SINT,            Int,           "B8G8R8A8_SINT",             true,  true,  true,  true,  false, false, true,  false, 4,   1 };
        case VK_FORMAT_B8G8R8A8_SRGB:            return { VK_FORMAT_B8G8R8A8_SRGB,            Normalized,    "B8G8R8A8_SRGB",             true,  true,  true,  true,  false, false, false, true,  4,   1 };
        //                                                Format                              Type           Name                         red    green  blue   alpha  depth  stncl  signed srgb   size block
        case VK_FORMAT_A8B8G8R8_UNORM_PACK32:    return { VK_FORMAT_A8B8G8R8_UNORM_PACK32,    Normalized,    "A8B8G8R8_UNORM_PACK32",     true,  true,  true,  true,  false, false, false, false, 4,   1 };
        case VK_FORMAT_A8B8G8R8_SNORM_PACK32:    return { VK_FORMAT_A8B8G8R8_SNORM_PACK32,    Normalized,    "A8B8G8R8_SNORM_PACK32",     true,  true,  true,  true,  false, false, true,  false, 4,   1 };
        case VK_FORMAT_A8B8G8R8_UINT_PACK32:     return { VK_FORMAT_A8B8G8R8_UINT_PACK32,     Int,           "A8B8G8R8_UINT_PACK32",      true,  true,  true,  true,  false, false, false, false, 4,   1 };
        case VK_FORMAT_A8B8G8R8_SINT_PACK32:     return { VK_FORMAT_A8B8G8R8_SINT_PACK32,     Int,           "A8B8G8R8_SINT_PACK32",      true,  true,  true,  true,  false, false, true,  false, 4,   1 };
        case VK_FORMAT_A8B8G8R8_SRGB_PACK32:     return { VK_FORMAT_A8B8G8R8_SRGB_PACK32,     Normalized,    "A8B8G8R8_SRGB_PACK32",      true,  true,  true,  true,  false, false, false, true,  4,   1 };
        //                                                Format                              Type           Name                         red    green  blue   alpha  depth  stncl  signed srgb   size block
        case VK_FORMAT_A2R10G10B10_UNORM_PACK32: return { VK_FORMAT_A2R10G10B10_UNORM_PACK32, Normalized,    "A2R10G10B10_UNORM_PACK32",  true,  true,  true,  true,  false, false, false, false, 4,   1 };
        case VK_FORMAT_A2R10G10B10_UINT_PACK32:  return { VK_FORMAT_A2R10G10B10_UINT_PACK32,  Int,           "A2R10G10B10_UINT_PACK32",   true,  true,  true,  true,  false, false, false, false, 4,   1 };
        case VK_FORMAT_A2B10G10R10_UNORM_PACK32: return { VK_FORMAT_A2B10G10R10_UNORM_PACK32, Normalized,    "A2B10G10R10_UNORM_PACK32",  true,  true,  true,  true,  false, false, false, false, 4,   1 };
        case VK_FORMAT_A2B10G10R10_UINT_PACK32:  return { VK_FORMAT_A2B10G10R10_UINT_PACK32,  Int,           "A2B10G10R10_UINT_PACK32",   true,  true,  true,  true,  false, false, false, false, 4,   1 };
        //                                                Format                              Type           Name                         red    green  blue   alpha  depth  stncl  signed srgb   size block
        case VK_FORMAT_R16_UNORM:                return { VK_FORMAT_R16_UNORM,                Normalized,    "R16_UNORM",                 true,  false, false, false, false, false, false, false, 2,   1 };
        case VK_FORMAT_R16_SNORM:                return { VK_FORMAT_R16_SNORM,                Normalized,    "R16_SNORM",                 true,  false, false, false, false, false, true,  false, 2,   1 };
        case VK_FORMAT_R16_UINT:                 return { VK_FORMAT_R16_UINT,                 Int,           "R16_UINT",                  true,  false, false, false, false, false, false, false, 2,   1 };
        case VK_FORMAT_R16_SINT:                 return { VK_FORMAT_R16_SINT,                 Int,           "R16_SINT",                  true,  false, false, false, false, false, true,  false, 2,   1 };
        case VK_FORMAT_R16_SFLOAT:               return { VK_FORMAT_R16_SFLOAT,               Float,         "R16_SFLOAT",                true,  false, false, false, false, false, true,  false, 2,   1 };
        //                                                Format                              Type           Name                         red    green  blue   alpha  depth  stncl  signed srgb   size block
        case VK_FORMAT_R16G16_UNORM:             return { VK_FORMAT_R16G16_UNORM,             Normalized,    "R16G16_UNORM",              true,  true , false, false, false, false, false, false, 4,   1 };
        case VK_FORMAT_R16G16_SNORM:             return { VK_FORMAT_R16G16_SNORM,             Normalized,    "R16G16_SNORM",              true,  true , false, false, false, false, true,  false, 4,   1 };
        case VK_FORMAT_R16G16_UINT:              return { VK_FORMAT_R16G16_UINT,              Int,           "R16G16_UINT",               true,  true , false, false, false, false, false, false, 4,   1 };
        case VK_FORMAT_R16G16_SINT:              return { VK_FORMAT_R16G16_SINT,              Int,           "R16G16_SINT",               true,  true , false, false, false, false, true,  false, 4,   1 };
        case VK_FORMAT_R16G16_SFLOAT:            return { VK_FORMAT_R16G16_SFLOAT,            Float,         "R16G16_SFLOAT",             true,  true , false, false, false, false, true,  false, 4,   1 };
        //                                                Format                              Type           Name                         red    green  blue   alpha  depth  stncl  signed srgb   size block
        case VK_FORMAT_R16G16B16A16_UNORM:       return { VK_FORMAT_R16G16B16A16_UNORM,       Normalized,    "R16G16B16A16_UNORM",        true,  true,  true,  true,  false, false, false, false, 8,   1 };
        case VK_FORMAT_R16G16B16A16_SNORM:       return { VK_FORMAT_R16G16B16A16_SNORM,       Normalized,    "R16G16B16A16_SNORM",        true,  true,  true,  true,  false, false, true,  false, 8,   1 };
        case VK_FORMAT_R16G16B16A16_UINT:        return { VK_FORMAT_R16G16B16A16_UINT,        Int,           "R16G16B16A16_UINT",         true,  true,  true,  true,  false, false, false, false, 8,   1 };
        case VK_FORMAT_R16G16B16A16_SINT:        return { VK_FORMAT_R16G16B16A16_SINT,        Int,           "R16G16B16A16_SINT",         true,  true,  true,  true,  false, false, true,  false, 8,   1 };
        case VK_FORMAT_R16G16B16A16_SFLOAT:      return { VK_FORMAT_R16G16B16A16_SFLOAT,      Float,         "R16G16B16A16_SFLOAT",       true,  true,  true,  true,  false, false, true,  false, 8,   1 };
        //                                                Format                              Type           Name                         red    green  blue   alpha  depth  stncl  signed srgb   size block
        case VK_FORMAT_R32_UINT:                 return { VK_FORMAT_R32_UINT,                 Int,           "R32_UINT",                  true,  false, false, false, false, false, false, false, 4,   1 };
        case VK_FORMAT_R32_SINT:                 return { VK_FORMAT_R32_SINT,                 Int,           "R32_SINT",                  true,  false, false, false, false, false, true,  false, 4,   1 };
        case VK_FORMAT_R32_SFLOAT:               return { VK_FORMAT_R32_SFLOAT,               Float,         "R32_SFLOAT",                true,  false, false, false, false, false, true,  false, 4,   1 };
        //                                                Format                              Type           Name                         red    green  blue   alpha  depth  stncl  signed srgb   size block
        case VK_FORMAT_R32G32_UINT:              return { VK_FORMAT_R32G32_UINT,              Int,           "R32G32_UINT",               true,  true,  false, false, false, false, false, false, 8,   1 };
        case VK_FORMAT_R32G32_SINT:              return { VK_FORMAT_R32G32_SINT,              Int,           "R32G32_SINT",               true,  true,  false, false, false, false, true,  false, 8,   1 };
        case VK_FORMAT_R32G32_SFLOAT:            return { VK_FORMAT_R32G32_SFLOAT,            Float,         "R32G32_SFLOAT",             true,  true,  false, false, false, false, true,  false, 8,   1 };
        //                                                Format                              Type           Name                         red    green  blue   alpha  depth  stncl  signed srgb   size block
        case VK_FORMAT_R32G32B32A32_UINT:        return { VK_FORMAT_R32G32B32A32_UINT,        Int,           "R32G32B32A32_UINT",         true,  true,  true,  true,  false, false, false, false, 16,  1 };
        case VK_FORMAT_R32G32B32A32_SINT:        return { VK_FORMAT_R32G32B32A32_SINT,        Int,           "R32G32B32A32_SINT",         true,  true,  true,  true,  false, false, true,  false, 16,  1 };
        case VK_FORMAT_R32G32B32A32_SFLOAT:      return { VK_FORMAT_R32G32B32A32_SFLOAT,      Float,         "R32G32B32A32_SFLOAT",       true,  true,  true,  true,  false, false, true,  false, 16,  1 };
        //                                                Format                              Type           Name                         red    green  blue   alpha  depth  stncl  signed srgb   size block
        case VK_FORMAT_E5B9G9R9_UFLOAT_PACK32:   return { VK_FORMAT_E5B9G9R9_UFLOAT_PACK32,   Float,         "E5B9G9R9_UFLOAT_PACK32",    true,  true,  true,  false, false, false, false, false, 32,  1 };
        //                                                Format                              Type           Name                         red    green  blue   alpha  depth  stncl  signed srgb   size block
        case VK_FORMAT_D16_UNORM:                return { VK_FORMAT_D16_UNORM,                Depth_stencil, "D16_UNORM",                 false, false, false, false, true,  false, false, false, 2,   1 };
        case VK_FORMAT_X8_D24_UNORM_PACK32:      return { VK_FORMAT_X8_D24_UNORM_PACK32,      Depth_stencil, "X8_D24_UNORM_PACK32",       false, false, false, false, true,  false, false, false, 4,   1 };
        case VK_FORMAT_D32_SFLOAT:               return { VK_FORMAT_D32_SFLOAT,               Depth_stencil, "D32_SFLOAT",                false, false, false, false, true,  false, false, false, 4,   1 };
        case VK_FORMAT_S8_UINT:                  return { VK_FORMAT_S8_UINT,                  Depth_stencil, "S8_UINT",                   false, false, false, false, false, true,  false, false, 1,   1 };
        case VK_FORMAT_D24_UNORM_S8_UINT:        return { VK_FORMAT_D24_UNORM_S8_UINT,        Depth_stencil, "D24_UNORM_S8_UINT",         false, false, false, false, true,  true,  false, false, 4,   1 };
        case VK_FORMAT_D32_SFLOAT_S8_UINT:       return { VK_FORMAT_D32_SFLOAT_S8_UINT,       Depth_stencil, "D32_SFLOAT_S8_UINT",        false, false, false, false, true,  true,  false, false, 8,   1 };
        //                                                Format                              Type           Name                         red    green  blue   alpha  depth  stncl  signed srgb   size block
        case VK_FORMAT_BC1_RGB_UNORM_BLOCK:      return { VK_FORMAT_BC1_RGB_UNORM_BLOCK,      Normalized,    "BC1_RGB_UNORM_BLOCK",       true,  true,  true,  false, false, false, false, false, 8,   4 };
        case VK_FORMAT_BC1_RGB_SRGB_BLOCK:       return { VK_FORMAT_BC1_RGB_SRGB_BLOCK,       Normalized,    "BC1_RGB_SRGB_BLOCK",        true,  true,  true,  false, false, false, false, true,  8,   4 };
        case VK_FORMAT_BC1_RGBA_UNORM_BLOCK:     return { VK_FORMAT_BC1_RGBA_UNORM_BLOCK,     Normalized,    "BC1_RGBA_UNORM_BLOCK",      true,  true,  true,  true,  false, false, false, false, 8,   4 };
        case VK_FORMAT_BC1_RGBA_SRGB_BLOCK:      return { VK_FORMAT_BC1_RGBA_SRGB_BLOCK,      Normalized,    "BC1_RGBA_SRGB_BLOCK",       true,  true,  true,  true,  false, false, false, true,  8,   4 };
        case VK_FORMAT_BC2_UNORM_BLOCK:          return { VK_FORMAT_BC2_UNORM_BLOCK,          Normalized,    "BC2_UNORM_BLOCK",           true,  true,  true,  true,  false, false, false, false, 16,  4 };
        case VK_FORMAT_BC2_SRGB_BLOCK:           return { VK_FORMAT_BC2_SRGB_BLOCK,           Normalized,    "BC2_SRGB_BLOCK",            true,  true,  true,  true,  false, false, false, true,  16,  4 };
        case VK_FORMAT_BC3_UNORM_BLOCK:          return { VK_FORMAT_BC3_UNORM_BLOCK,          Normalized,    "BC3_UNORM_BLOCK",           true,  true,  true,  true,  false, false, false, false, 16,  4 };
        case VK_FORMAT_BC3_SRGB_BLOCK:           return { VK_FORMAT_BC3_SRGB_BLOCK,           Normalized,    "BC3_SRGB_BLOCK",            true,  true,  true,  true,  false, false, false, true,  16,  4 };
        case VK_FORMAT_BC4_UNORM_BLOCK:          return { VK_FORMAT_BC4_UNORM_BLOCK,          Normalized,    "BC4_UNORM_BLOCK",           true,  false, false, false, false, false, false, false, 8,   4 };
        case VK_FORMAT_BC4_SNORM_BLOCK:          return { VK_FORMAT_BC4_SNORM_BLOCK,          Normalized,    "BC4_SNORM_BLOCK",           true,  false, false, false, false, false, true,  false, 8,   4 };
        case VK_FORMAT_BC5_UNORM_BLOCK:          return { VK_FORMAT_BC5_UNORM_BLOCK,          Normalized,    "BC5_UNORM_BLOCK",           true,  true,  false, false, false, false, false, false, 16,  4 };
        case VK_FORMAT_BC5_SNORM_BLOCK:          return { VK_FORMAT_BC5_SNORM_BLOCK,          Normalized,    "BC5_SNORM_BLOCK",           true,  true,  false, false, false, false, true,  false, 16,  4 };
        case VK_FORMAT_BC6H_UFLOAT_BLOCK:        return { VK_FORMAT_BC6H_UFLOAT_BLOCK,        Float,         "BC6H_UFLOAT_BLOCK",         true,  true,  true,  false, false, false, false, false, 16,  4 };
        case VK_FORMAT_BC6H_SFLOAT_BLOCK:        return { VK_FORMAT_BC6H_SFLOAT_BLOCK,        Float,         "BC6H_SFLOAT_BLOCK",         true,  true,  true,  false, false, false, true,  false, 16,  4 };
        case VK_FORMAT_BC7_UNORM_BLOCK:          return { VK_FORMAT_BC7_UNORM_BLOCK,          Normalized,    "BC7_UNORM_BLOCK",           true,  true,  true,  true,  false, false, false, false, 16,  4 };
        case VK_FORMAT_BC7_SRGB_BLOCK:           return { VK_FORMAT_BC7_SRGB_BLOCK,           Normalized,    "BC7_SRGB_BLOCK",            true,  true,  true,  true,  false, false, false, true,  16,  4 };
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

    uint32_t format_size(VkFormat format)
    {
        auto info = get_format_info(format);
        return info.texel_size;
    }
}
