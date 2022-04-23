// Copyright 2022 Robert Ryan. See Licence.md.

#pragma once

#include <cstdint>

#define YGG_FORWARDDECL_VK_HANDLE(object) typedef struct object##_T* object
#define YGG_FORWARDDECL_VK_NON_DISPATCHABLE_HANDLE(object) typedef struct object##_T* object

YGG_FORWARDDECL_VK_NON_DISPATCHABLE_HANDLE(VkBuffer);
YGG_FORWARDDECL_VK_NON_DISPATCHABLE_HANDLE(VkImage);
YGG_FORWARDDECL_VK_HANDLE(VkInstance);
YGG_FORWARDDECL_VK_HANDLE(VkPhysicalDevice);
YGG_FORWARDDECL_VK_HANDLE(VkDevice);
YGG_FORWARDDECL_VK_HANDLE(VkQueue);
YGG_FORWARDDECL_VK_NON_DISPATCHABLE_HANDLE(VkSemaphore);
YGG_FORWARDDECL_VK_HANDLE(VkCommandBuffer);
YGG_FORWARDDECL_VK_NON_DISPATCHABLE_HANDLE(VkFence);
YGG_FORWARDDECL_VK_NON_DISPATCHABLE_HANDLE(VkDeviceMemory);
YGG_FORWARDDECL_VK_NON_DISPATCHABLE_HANDLE(VkEvent);
YGG_FORWARDDECL_VK_NON_DISPATCHABLE_HANDLE(VkQueryPool);
YGG_FORWARDDECL_VK_NON_DISPATCHABLE_HANDLE(VkBufferView);
YGG_FORWARDDECL_VK_NON_DISPATCHABLE_HANDLE(VkImageView);
YGG_FORWARDDECL_VK_NON_DISPATCHABLE_HANDLE(VkShaderModule);
YGG_FORWARDDECL_VK_NON_DISPATCHABLE_HANDLE(VkPipelineCache);
YGG_FORWARDDECL_VK_NON_DISPATCHABLE_HANDLE(VkPipelineLayout);
YGG_FORWARDDECL_VK_NON_DISPATCHABLE_HANDLE(VkPipeline);
YGG_FORWARDDECL_VK_NON_DISPATCHABLE_HANDLE(VkRenderPass);
YGG_FORWARDDECL_VK_NON_DISPATCHABLE_HANDLE(VkDescriptorSetLayout);
YGG_FORWARDDECL_VK_NON_DISPATCHABLE_HANDLE(VkSampler);
YGG_FORWARDDECL_VK_NON_DISPATCHABLE_HANDLE(VkDescriptorSet);
YGG_FORWARDDECL_VK_NON_DISPATCHABLE_HANDLE(VkDescriptorPool);
YGG_FORWARDDECL_VK_NON_DISPATCHABLE_HANDLE(VkFramebuffer);
YGG_FORWARDDECL_VK_NON_DISPATCHABLE_HANDLE(VkCommandPool);
YGG_FORWARDDECL_VK_NON_DISPATCHABLE_HANDLE(VkSwapchainKHR);
YGG_FORWARDDECL_VK_NON_DISPATCHABLE_HANDLE(VkSurfaceKHR);
YGG_FORWARDDECL_VK_NON_DISPATCHABLE_HANDLE(VkAccelerationStructureKHR);
YGG_FORWARDDECL_VK_NON_DISPATCHABLE_HANDLE(VkDebugUtilsMessengerEXT);

YGG_FORWARDDECL_VK_HANDLE(VmaAllocator);
YGG_FORWARDDECL_VK_HANDLE(VmaPool);
YGG_FORWARDDECL_VK_HANDLE(VmaAllocation);
YGG_FORWARDDECL_VK_HANDLE(VmaDefragmentationContext);

typedef uint32_t VkFlags;
typedef VkFlags VkBufferUsage;
typedef VkFlags VkColorComponentFlags;
typedef VkFlags VkCullModeFlags;
typedef VkFlags VkDependencyFlags;
typedef VkFlags VkImageAspectFlags;
typedef VkFlags VkImageCreateFlags;
typedef VkFlags VkImageUsage;
typedef VkFlags VkMemoryPropertyFlags;
typedef VkFlags VkPipelineCreateFlags;
typedef VkFlags VkShaderStageFlags;

typedef uint64_t VkFlags64;
typedef VkFlags64 VkAccessFlags2;
typedef VkFlags64 VkPipelineStageFlags2;

typedef uint64_t VkDeviceSize;

enum VkAttachmentLoadOp : int32_t;
enum VkAttachmentStoreOp : int32_t;
enum VkBlendFactor : int32_t;
enum VkBlendOp : int32_t;
enum VkColorSpaceKHR : int32_t;
enum VkCommandBufferLevel : int32_t;
enum VkCommandPoolResetFlagBits : int32_t;
enum VkCompareOp : int32_t;
enum VkDynamicState : int32_t;
enum VkFilter : int32_t;
enum VkFrontFace : int32_t;
enum VkIndexType : int32_t;
enum VkImageLayout : int32_t;
enum VkImageType : int32_t;
enum VkImageViewType : int32_t;
enum VkFormat : int32_t;
enum VkLogicOp : int32_t;
enum VkPipelineBindPoint : int32_t;
enum VkPolygonMode : int32_t;
enum VkPrimitiveTopology : int32_t;
enum VkResult : int32_t;
enum VkShaderStageFlagBits : int32_t;
enum VkStencilOp : int32_t;
enum VkTessellationDomainOrigin : int32_t;
enum VkVertexInputRate : int32_t;

struct VkBufferMemoryBarrier2;
struct VkImageSubresourceRange;
struct VkImageMemoryBarrier2;
struct VkMemoryBarrier2;

union VkClearValue;
