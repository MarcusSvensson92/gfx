#include "GfxInternal.h"

#define VMA_IMPLEMENTATION
#include <vk_mem_alloc.h>

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

const VkDeviceSize GFX_STAGING_BUFFER_SIZE = 512 * 1024 * 1024;
const VkDeviceSize GFX_STAGING_BUFFER_MASK = GFX_STAGING_BUFFER_SIZE - 1;
static_assert((GFX_STAGING_BUFFER_SIZE & GFX_STAGING_BUFFER_MASK) == 0, "GFX_STAGING_BUFFER_SIZE must be a power of two!");

static VkDeviceSize AllocateStagingBuffer(GfxDevice device, VkDeviceSize size, VkDeviceSize alignment = 256)
{
    VkDeviceSize head = device->m_StagingBufferHead;
    VkDeviceSize tail = device->m_CommandBuffers[device->m_CommandBufferIndexNext].m_StagingBufferTail;

    VkDeviceSize offset = (head + alignment - 1) & ~(alignment - 1);
    if (offset + size > GFX_STAGING_BUFFER_SIZE)
        offset = 0;

    if (offset + size > GFX_STAGING_BUFFER_SIZE || ((head - tail) & GFX_STAGING_BUFFER_MASK) > ((offset + size - tail) & GFX_STAGING_BUFFER_MASK))
    {
        Print("Error: Staging buffer is out of memory");
        Abort();
    }

    device->m_StagingBufferHead = offset + size;

    return offset;
}

static VKAPI_ATTR VkBool32 VKAPI_CALL DebugCallback(VkDebugReportFlagsEXT flags, VkDebugReportObjectTypeEXT, uint64_t, size_t, int32_t code, const char*, const char* message, void*)
{
    if ((flags & VK_DEBUG_REPORT_ERROR_BIT_EXT) != 0)
    {
        Print("Error: %s", message);
        Abort();
    }
    else if ((flags & (VK_DEBUG_REPORT_WARNING_BIT_EXT | VK_DEBUG_REPORT_PERFORMANCE_WARNING_BIT_EXT)) != 0)
    {
        Print("Warning: %s", message);
    }
    else
    {
        Print("Information: %s", message);
    }
    return VK_FALSE;
}

static void QueueCmd(GfxDevice device, GfxCmdFunction func, const void* user_data, uint32_t user_data_size)
{
    const uint32_t offset = device->m_CmdFunctionUserData.Count();
    device->m_CmdFunctions.Push(func);
    device->m_CmdFunctionUserData.Grow(user_data_size);
    device->m_CmdFunctionUserDataOffsets.Push(offset);
    memcpy(device->m_CmdFunctionUserData.Data() + offset, user_data, user_data_size);
}

struct CmdUploadBufferParams
{
    VkBuffer        m_DstBuffer;
    VkDeviceSize    m_DstOffset;
    VkAccessFlags   m_DstAccessMask;
    VkBuffer        m_SrcBuffer;
    VkDeviceSize    m_SrcOffset;
    VkDeviceSize    m_Size;
};
static void CmdUploadBuffer(VkCommandBuffer cmd, void* user_data)
{
    CmdUploadBufferParams* params = static_cast<CmdUploadBufferParams*>(user_data);

    VkBufferMemoryBarrier pre_copy_barrier = {};
    pre_copy_barrier.sType = VK_STRUCTURE_TYPE_BUFFER_MEMORY_BARRIER;
    pre_copy_barrier.srcAccessMask = 0;
    pre_copy_barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
    pre_copy_barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    pre_copy_barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    pre_copy_barrier.buffer = params->m_DstBuffer;
    pre_copy_barrier.offset = params->m_DstOffset;
    pre_copy_barrier.size = params->m_Size;
    vkCmdPipelineBarrier(cmd, VK_PIPELINE_STAGE_ALL_COMMANDS_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, 0, 0, NULL, 1, &pre_copy_barrier, 0, NULL);

    VkBufferCopy copy_region = {};
    copy_region.srcOffset = params->m_SrcOffset;
    copy_region.dstOffset = params->m_DstOffset;
    copy_region.size = params->m_Size;
    vkCmdCopyBuffer(cmd, params->m_SrcBuffer, params->m_DstBuffer, 1, &copy_region);

    VkBufferMemoryBarrier post_copy_barrier = {};
    post_copy_barrier.sType = VK_STRUCTURE_TYPE_BUFFER_MEMORY_BARRIER;
    post_copy_barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
    post_copy_barrier.dstAccessMask = params->m_DstAccessMask;
    post_copy_barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    post_copy_barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    post_copy_barrier.buffer = params->m_DstBuffer;
    post_copy_barrier.offset = params->m_DstOffset;
    post_copy_barrier.size = params->m_Size;
    vkCmdPipelineBarrier(cmd, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_ALL_COMMANDS_BIT, 0, 0, NULL, 1, &post_copy_barrier, 0, NULL);
}

struct CmdUploadImageParams
{
    VkImage             m_DstImage;
    uint32_t            m_DstWidth;
    uint32_t            m_DstHeight;
    VkImageAspectFlags  m_DstAspectMask;
    VkAccessFlags       m_DstAccessMask;
    VkImageLayout       m_DstLayout;
    VkBuffer            m_SrcBuffer;
    VkDeviceSize        m_SrcOffset;
};
static void CmdUploadImage(VkCommandBuffer cmd, void* user_data)
{
    CmdUploadImageParams* params = static_cast<CmdUploadImageParams*>(user_data);

    VkImageMemoryBarrier barrier = {};
    barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
    barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.image = params->m_DstImage;
    barrier.subresourceRange.aspectMask = params->m_DstAspectMask;
    barrier.subresourceRange.baseMipLevel = 0;
    barrier.subresourceRange.levelCount = 1;
    barrier.subresourceRange.baseArrayLayer = 0;
    barrier.subresourceRange.layerCount = 1;
    barrier.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    barrier.newLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
    barrier.srcAccessMask = 0;
    barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
    vkCmdPipelineBarrier(cmd, VK_PIPELINE_STAGE_ALL_COMMANDS_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, 0, 0, NULL, 0, NULL, 1, &barrier);

    VkBufferImageCopy copy_region = {};
    copy_region.bufferOffset = params->m_SrcOffset;
    copy_region.imageSubresource.aspectMask = params->m_DstAspectMask;
    copy_region.imageSubresource.mipLevel = 0;
    copy_region.imageSubresource.baseArrayLayer = 0;
    copy_region.imageSubresource.layerCount = 1;
    copy_region.imageExtent.width = params->m_DstWidth;
    copy_region.imageExtent.height = params->m_DstHeight;
    copy_region.imageExtent.depth = 1;
    vkCmdCopyBufferToImage(cmd, params->m_SrcBuffer, params->m_DstImage, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &copy_region);

    barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
    barrier.newLayout = params->m_DstLayout;
    barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
    barrier.dstAccessMask = params->m_DstAccessMask;
    vkCmdPipelineBarrier(cmd, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_ALL_COMMANDS_BIT, 0, 0, NULL, 0, NULL, 1, &barrier);
}

struct CmdTransitionImageParams
{
    VkImage             m_DstImage;
    VkImageAspectFlags  m_DstAspectMask;
    VkAccessFlags       m_DstAccessMask;
    VkImageLayout       m_DstLayout;
};
static void CmdTransitionImage(VkCommandBuffer cmd, void* user_data)
{
    CmdTransitionImageParams* params = static_cast<CmdTransitionImageParams*>(user_data);

    VkImageMemoryBarrier barrier = {};
    barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
    barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.image = params->m_DstImage;
    barrier.subresourceRange.aspectMask = params->m_DstAspectMask;
    barrier.subresourceRange.baseMipLevel = 0;
    barrier.subresourceRange.levelCount = 1;
    barrier.subresourceRange.baseArrayLayer = 0;
    barrier.subresourceRange.layerCount = 1;
    barrier.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    barrier.newLayout = params->m_DstLayout;
    barrier.srcAccessMask = 0;
    barrier.dstAccessMask = params->m_DstAccessMask;
    vkCmdPipelineBarrier(cmd, VK_PIPELINE_STAGE_ALL_COMMANDS_BIT, VK_PIPELINE_STAGE_ALL_COMMANDS_BIT, 0, 0, NULL, 0, NULL, 1, &barrier);
}

struct CmdGenerateMipmapParams
{
    VkImage             m_DstImage;
    uint32_t            m_DstWidth;
    uint32_t            m_DstHeight;
    uint32_t            m_DstMipCount;
    VkImageAspectFlags  m_DstAspectMask;
    VkAccessFlags       m_DstAccessMask;
    VkImageLayout       m_DstLayout;
    VkBuffer            m_SrcBuffer;
    VkDeviceSize        m_SrcOffset;
};
static void CmdGenerateMipmap(VkCommandBuffer cmd, void* user_data)
{
    CmdGenerateMipmapParams* params = static_cast<CmdGenerateMipmapParams*>(user_data);

    VkImageMemoryBarrier barrier = {};
    barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
    barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.image = params->m_DstImage;
    barrier.subresourceRange.aspectMask = params->m_DstAspectMask;
    barrier.subresourceRange.baseMipLevel = 0;
    barrier.subresourceRange.levelCount = params->m_DstMipCount;
    barrier.subresourceRange.baseArrayLayer = 0;
    barrier.subresourceRange.layerCount = 1;
    barrier.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    barrier.newLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
    barrier.srcAccessMask = 0;
    barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
    vkCmdPipelineBarrier(cmd, VK_PIPELINE_STAGE_ALL_COMMANDS_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, 0, 0, NULL, 0, NULL, 1, &barrier);

    VkBufferImageCopy copy_region = {};
    copy_region.bufferOffset = params->m_SrcOffset;
    copy_region.imageSubresource.aspectMask = params->m_DstAspectMask;
    copy_region.imageSubresource.mipLevel = 0;
    copy_region.imageSubresource.baseArrayLayer = 0;
    copy_region.imageSubresource.layerCount = 1;
    copy_region.imageExtent.width = params->m_DstWidth;
    copy_region.imageExtent.height = params->m_DstHeight;
    copy_region.imageExtent.depth = 1;
    vkCmdCopyBufferToImage(cmd, params->m_SrcBuffer, params->m_DstImage, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &copy_region);

    uint32_t width = params->m_DstWidth;
    uint32_t height = params->m_DstHeight;
    for (uint32_t mip = 1; mip < params->m_DstMipCount; ++mip)
    {
        barrier.subresourceRange.baseMipLevel = mip - 1;
        barrier.subresourceRange.levelCount = 1;
        barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
        barrier.newLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
        barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
        barrier.dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
        vkCmdPipelineBarrier(cmd, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, 0, 0, NULL, 0, NULL, 1, &barrier);

        VkImageBlit region = {};
        region.srcOffsets[1].x = width;
        region.srcOffsets[1].y = height;
        region.dstOffsets[1].x = Max(width >> 1, 1);
        region.dstOffsets[1].y = Max(height >> 1, 1);
        region.srcOffsets[1].z = region.dstOffsets[1].z = 1;
        region.srcSubresource.mipLevel = mip - 1;
        region.dstSubresource.mipLevel = mip;
        region.srcSubresource.aspectMask = region.dstSubresource.aspectMask = params->m_DstAspectMask;
        region.srcSubresource.baseArrayLayer = region.dstSubresource.baseArrayLayer = 0;
        region.srcSubresource.layerCount = region.dstSubresource.layerCount = 1;
        vkCmdBlitImage(cmd, params->m_DstImage, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, params->m_DstImage, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &region, VK_FILTER_LINEAR);

        barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
        barrier.newLayout = params->m_DstLayout;
        barrier.srcAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
        barrier.dstAccessMask = params->m_DstAccessMask;
        vkCmdPipelineBarrier(cmd, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_ALL_COMMANDS_BIT, 0, 0, NULL, 0, NULL, 1, &barrier);

        width = Max(width >> 1, 1);
        height = Max(height >> 1, 1);
    }

    barrier.subresourceRange.baseMipLevel = params->m_DstMipCount - 1;
    barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
    barrier.newLayout = params->m_DstLayout;
    barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
    barrier.dstAccessMask = params->m_DstAccessMask;
    vkCmdPipelineBarrier(cmd, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_ALL_COMMANDS_BIT, 0, 0, NULL, 0, NULL, 1, &barrier);
}


static void CreateSwapchain(GfxDevice device, uint32_t width, uint32_t height, uint32_t image_count)
{
	VkSurfaceCapabilitiesKHR surface_capabilities = {};
	VK(vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device->m_PhysicalDevice, device->m_Surface, &surface_capabilities));
	device->m_SwapchainImageExtent.width = Clamp(width, surface_capabilities.minImageExtent.width, surface_capabilities.maxImageExtent.width);
	device->m_SwapchainImageExtent.height = Clamp(height, surface_capabilities.minImageExtent.height, surface_capabilities.maxImageExtent.height);
	device->m_SwapchainImageCount = surface_capabilities.maxImageCount == 0 ? Max(image_count, surface_capabilities.minImageCount) : Clamp(image_count, surface_capabilities.minImageCount, surface_capabilities.maxImageCount);

	uint32_t surface_format_count = 0;
	VK(vkGetPhysicalDeviceSurfaceFormatsKHR(device->m_PhysicalDevice, device->m_Surface, &surface_format_count, NULL));
	Array<VkSurfaceFormatKHR> surface_formats(surface_format_count);
	VK(vkGetPhysicalDeviceSurfaceFormatsKHR(device->m_PhysicalDevice, device->m_Surface, &surface_format_count, surface_formats.Data()));
    device->m_SwapchainSurfaceFormat = surface_formats[0];
    for (uint32_t i = 0; i < surface_format_count; ++i)
    {
        if (surface_formats[i].format == VK_FORMAT_R8G8B8A8_UNORM ||
            surface_formats[i].format == VK_FORMAT_B8G8R8A8_UNORM)
        {
            device->m_SwapchainSurfaceFormat = surface_formats[i];
            break;
        }
    }
    
	VkSwapchainCreateInfoKHR swapchain_info = {};
	swapchain_info.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
	swapchain_info.surface = device->m_Surface;
	swapchain_info.minImageCount = device->m_SwapchainImageCount;
	swapchain_info.imageFormat = device->m_SwapchainSurfaceFormat.format;
	swapchain_info.imageColorSpace = device->m_SwapchainSurfaceFormat.colorSpace;
	swapchain_info.imageExtent = device->m_SwapchainImageExtent;
	swapchain_info.imageArrayLayers = 1;
	swapchain_info.imageUsage = VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
	swapchain_info.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
	swapchain_info.preTransform = surface_capabilities.currentTransform;
	swapchain_info.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
	swapchain_info.presentMode = VK_PRESENT_MODE_FIFO_KHR;
	swapchain_info.clipped = VK_TRUE;
	VK(vkCreateSwapchainKHR(device->m_Device, &swapchain_info, NULL, &device->m_Swapchain));

	VK(vkGetSwapchainImagesKHR(device->m_Device, device->m_Swapchain, &device->m_SwapchainImageCount, NULL));
    Array<VkImage> swapchain_images(device->m_SwapchainImageCount);
	VK(vkGetSwapchainImagesKHR(device->m_Device, device->m_Swapchain, &device->m_SwapchainImageCount, swapchain_images.Data()));

    device->m_SwapchainTextures.Resize(device->m_SwapchainImageCount);
    for (uint32_t i = 0; i < device->m_SwapchainImageCount; ++i)
    {
        device->m_SwapchainTextures[i].m_Image = swapchain_images[i];
        device->m_SwapchainTextures[i].m_Width = width;
        device->m_SwapchainTextures[i].m_Height = height;
        device->m_SwapchainTextures[i].m_Format = device->m_SwapchainSurfaceFormat.format;

        VkImageViewCreateInfo image_view_info = {};
        image_view_info.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
        image_view_info.image = device->m_SwapchainTextures[i].m_Image;
        image_view_info.viewType = VK_IMAGE_VIEW_TYPE_2D;
        image_view_info.format = device->m_SwapchainSurfaceFormat.format;
        image_view_info.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
        image_view_info.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
        image_view_info.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
        image_view_info.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
        image_view_info.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        image_view_info.subresourceRange.baseMipLevel = 0;
        image_view_info.subresourceRange.levelCount = 1;
        image_view_info.subresourceRange.baseArrayLayer = 0;
        image_view_info.subresourceRange.layerCount = 1;
        VK(vkCreateImageView(device->m_Device, &image_view_info, NULL, &device->m_SwapchainTextures[i].m_ImageView));

        CmdTransitionImageParams transition_params;
        transition_params.m_DstImage = device->m_SwapchainTextures[i].m_Image;
        transition_params.m_DstAspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        transition_params.m_DstAccessMask = VK_ACCESS_MEMORY_READ_BIT;
        transition_params.m_DstLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
        QueueCmd(device, &CmdTransitionImage, &transition_params, sizeof(CmdTransitionImageParams));
    }
    device->m_SwapchainImageIndex = 0;

    device->m_CommandBuffers.Resize(device->m_SwapchainImageCount);
    for (uint32_t i = 0; i < device->m_SwapchainImageCount; ++i)
    {
        memset(&device->m_CommandBuffers[i], 0, sizeof(GfxCommandBuffer_T));

        VkCommandBufferAllocateInfo command_buffer_info = {};
        command_buffer_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
        command_buffer_info.commandPool = device->m_CommandPool;
        command_buffer_info.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
        command_buffer_info.commandBufferCount = 1;
        VK(vkAllocateCommandBuffers(device->m_Device, &command_buffer_info, &device->m_CommandBuffers[i].m_CommandBuffer));

        VkFenceCreateInfo fence_info = {};
        fence_info.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
        fence_info.flags = VK_FENCE_CREATE_SIGNALED_BIT;
        VK(vkCreateFence(device->m_Device, &fence_info, NULL, &device->m_CommandBuffers[i].m_CommandBufferFence));

        VkSemaphoreCreateInfo semaphore_info = {};
        semaphore_info.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
        VK(vkCreateSemaphore(device->m_Device, &semaphore_info, NULL, &device->m_CommandBuffers[i].m_CommandBufferSemaphore));
        VK(vkCreateSemaphore(device->m_Device, &semaphore_info, NULL, &device->m_CommandBuffers[i].m_PresentSemaphore));

        const VkDescriptorPoolSize descriptor_pool_sizes[] =
        {
            { VK_DESCRIPTOR_TYPE_SAMPLER, 65535 },
            { VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 65535 },
            { VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, 65535 },
            { VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 65535 },
            { VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER, 65535 },
            { VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER, 65535 },
            { VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 65535 },
            { VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 65535 },
            { VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, 65535 },
            { VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC, 65535 },
        };
        VkDescriptorPoolCreateInfo pool_info = {};
        pool_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
        pool_info.poolSizeCount = ARRAY_COUNT(descriptor_pool_sizes);
        pool_info.pPoolSizes = descriptor_pool_sizes;
        pool_info.maxSets = 65535;
        VK(vkCreateDescriptorPool(device->m_Device, &pool_info, NULL, &device->m_CommandBuffers[i].m_DescriptorPool));

        device->m_CommandBuffers[i].m_Device = device;
    }
    device->m_CommandBufferIndexCurr = 0;
    device->m_CommandBufferIndexNext = (device->m_CommandBufferIndexCurr + 1) % device->m_SwapchainImageCount;
}
static void DestroySwapchain(GfxDevice device)
{
    for (uint32_t i = 0; i < device->m_SwapchainImageCount; ++i)
    {
        vkDestroySemaphore(device->m_Device, device->m_CommandBuffers[i].m_PresentSemaphore, NULL);
        vkDestroySemaphore(device->m_Device, device->m_CommandBuffers[i].m_CommandBufferSemaphore, NULL);
        vkDestroyFence(device->m_Device, device->m_CommandBuffers[i].m_CommandBufferFence, NULL);
        vkFreeCommandBuffers(device->m_Device, device->m_CommandPool, 1, &device->m_CommandBuffers[i].m_CommandBuffer);
        vkDestroyDescriptorPool(device->m_Device, device->m_CommandBuffers[i].m_DescriptorPool, NULL);

        vkDestroyImageView(device->m_Device, device->m_SwapchainTextures[i].m_ImageView, NULL);
    }

	vkDestroySwapchainKHR(device->m_Device, device->m_Swapchain, NULL);
}

GfxDevice GfxCreateDevice(const GfxCreateDeviceParams& params)
{
	const char* instance_extensions[] =
	{
		VK_KHR_SURFACE_EXTENSION_NAME,
#if defined(VK_USE_PLATFORM_WIN32_KHR)
		VK_KHR_WIN32_SURFACE_EXTENSION_NAME,
#endif
		VK_EXT_DEBUG_REPORT_EXTENSION_NAME,
	};
	const char* device_extensions[] =
	{
		VK_KHR_SWAPCHAIN_EXTENSION_NAME
	};
	const char* validation_layer = "VK_LAYER_LUNARG_standard_validation";

	GfxDevice device = New<GfxDevice_T>();

	uint32_t instance_extension_properties_count = 0;
	VK(vkEnumerateInstanceExtensionProperties(NULL, &instance_extension_properties_count, NULL));
	Array<VkExtensionProperties> instance_extension_properties(instance_extension_properties_count);
	VK(vkEnumerateInstanceExtensionProperties(NULL, &instance_extension_properties_count, instance_extension_properties.Data()));

	for (uint32_t i = 0; i < ARRAY_COUNT(instance_extensions); ++i)
	{
		bool extension_supported = false;
		for (uint32_t j = 0; j < instance_extension_properties_count; ++j)
		{
			if (strcmp(instance_extensions[i], instance_extension_properties[j].extensionName) == 0)
			{
				extension_supported = true;
				break;
			}
		}
        if (!extension_supported)
        {
            Print("Error: All instance extensions are not supported");
            Abort();
        }
	}

    if (params.m_EnableValidationLayer)
    {
        uint32_t instance_layer_properties_count = 0;
        VK(vkEnumerateInstanceLayerProperties(&instance_layer_properties_count, NULL));
        Array<VkLayerProperties> instance_layer_properties(instance_layer_properties_count);
        VK(vkEnumerateInstanceLayerProperties(&instance_layer_properties_count, instance_layer_properties.Data()));

        bool validation_layer_supported = false;
        for (uint32_t i = 0; i < instance_layer_properties_count; ++i)
        {
            if (strcmp(validation_layer, instance_layer_properties[i].layerName) == 0)
            {
                validation_layer_supported = true;
                break;
            }
        }
        if (!validation_layer_supported)
        {
            Print("Error: Validation layer is not supported");
            Abort();
        }
    }

	VkApplicationInfo app_info = {};
	app_info.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
	app_info.apiVersion = VK_API_VERSION_1_0;

	VkInstanceCreateInfo instance_info = {};
	instance_info.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
	instance_info.pApplicationInfo = &app_info;
	instance_info.enabledExtensionCount = ARRAY_COUNT(instance_extensions);
	instance_info.ppEnabledExtensionNames = instance_extensions;
    if (params.m_EnableValidationLayer)
    {
        instance_info.enabledLayerCount = 1;
        instance_info.ppEnabledLayerNames = &validation_layer;
    }
	VK(vkCreateInstance(&instance_info, NULL, &device->m_Instance));

    device->m_DebugCallback = VK_NULL_HANDLE;
    if (params.m_EnableValidationLayer)
    {
        PFN_vkCreateDebugReportCallbackEXT vkCreateDebugReportCallbackEXT = reinterpret_cast<PFN_vkCreateDebugReportCallbackEXT>(vkGetInstanceProcAddr(device->m_Instance, "vkCreateDebugReportCallbackEXT"));
        
        VkDebugReportCallbackCreateInfoEXT callback_info = {};
        callback_info.sType = VK_STRUCTURE_TYPE_DEBUG_REPORT_CALLBACK_CREATE_INFO_EXT;
        callback_info.flags = VK_DEBUG_REPORT_ERROR_BIT_EXT | VK_DEBUG_REPORT_WARNING_BIT_EXT;
        callback_info.pfnCallback = DebugCallback;
        VK(vkCreateDebugReportCallbackEXT(device->m_Instance, &callback_info, NULL, &device->m_DebugCallback));
    }

#if defined(VK_USE_PLATFORM_WIN32_KHR)
	VkWin32SurfaceCreateInfoKHR surface_info = {};
	surface_info.sType = VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR;
	surface_info.hinstance = GetModuleHandle(NULL);
	surface_info.hwnd = static_cast<HWND>(params.m_WindowHandle);
	VK(vkCreateWin32SurfaceKHR(device->m_Instance, &surface_info, NULL, &device->m_Surface));
#endif

	uint32_t physical_device_count = 0;
	VK(vkEnumeratePhysicalDevices(device->m_Instance, &physical_device_count, NULL));
	Array<VkPhysicalDevice> physical_devices(physical_device_count);
	VK(vkEnumeratePhysicalDevices(device->m_Instance, &physical_device_count, physical_devices.Data()));

	device->m_PhysicalDevice = VK_NULL_HANDLE;
	device->m_GraphicsQueueIndex = ~0U;
	for (uint32_t i = 0; i < physical_devices.Count(); ++i)
	{
		uint32_t device_extension_properties_count = 0;
		VK(vkEnumerateDeviceExtensionProperties(physical_devices[i], NULL, &device_extension_properties_count, NULL));
		Array<VkExtensionProperties> device_extension_properties(device_extension_properties_count);
		VK(vkEnumerateDeviceExtensionProperties(physical_devices[i], NULL, &device_extension_properties_count, device_extension_properties.Data()));

		bool all_extensions_supported = true;
		for (uint32_t j = 0; j < ARRAY_COUNT(device_extensions); ++j)
		{
			bool extension_supported = false;
			for (uint32_t k = 0; k < device_extension_properties_count; ++k)
			{
				if (strcmp(device_extensions[j], device_extension_properties[k].extensionName) == 0)
				{
					extension_supported = true;
					break;
				}
			}
			if (!extension_supported)
			{
				all_extensions_supported = false;
				break;
			}
		}
		if (!all_extensions_supported)
			continue;

        if (params.m_EnableValidationLayer)
        {
            uint32_t device_layer_properties_count = 0;
            VK(vkEnumerateDeviceLayerProperties(physical_devices[i], &device_layer_properties_count, NULL));
            Array<VkLayerProperties> device_layer_properties(device_layer_properties_count);
            VK(vkEnumerateDeviceLayerProperties(physical_devices[i], &device_layer_properties_count, device_layer_properties.Data()));

            bool validation_layer_supported = false;
            for (uint32_t j = 0; j < device_layer_properties_count; ++j)
            {
                if (strcmp(validation_layer, device_layer_properties[j].layerName) == 0)
                {
                    validation_layer_supported = true;
                    break;
                }
            }
            if (!validation_layer_supported)
                continue;
        }

		uint32_t queue_family_properties_count = 0;
		vkGetPhysicalDeviceQueueFamilyProperties(physical_devices[i], &queue_family_properties_count, NULL);
		Array<VkQueueFamilyProperties> queue_family_properties(queue_family_properties_count);
		vkGetPhysicalDeviceQueueFamilyProperties(physical_devices[i], &queue_family_properties_count, queue_family_properties.Data());

		for (uint32_t j = 0; j < queue_family_properties_count; ++j)
		{
			VkBool32 queue_flags_supported = (queue_family_properties[j].queueFlags & VK_QUEUE_GRAPHICS_BIT) != 0;

			VkBool32 surface_supported = VK_FALSE;
			VK(vkGetPhysicalDeviceSurfaceSupportKHR(physical_devices[i], j, device->m_Surface, &surface_supported));

			if (queue_flags_supported && surface_supported)
			{
				device->m_GraphicsQueueIndex = j;
				break;
			}
		}
		if (device->m_GraphicsQueueIndex == ~0U)
			continue;

		device->m_PhysicalDevice = physical_devices[i];
		break;
	}
    if (device->m_PhysicalDevice == VK_NULL_HANDLE)
    {
        Print("Error: No compatible physical device was found");
        Abort();
    }

	const float queue_priority = 1.0f;
	VkDeviceQueueCreateInfo queue_info = {};
	queue_info.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
	queue_info.queueFamilyIndex = device->m_GraphicsQueueIndex;
	queue_info.queueCount = 1;
	queue_info.pQueuePriorities = &queue_priority;

    VkPhysicalDeviceFeatures device_features = {};
    device_features.shaderStorageImageExtendedFormats = VK_TRUE;

	VkDeviceCreateInfo device_info = {};
	device_info.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
	device_info.queueCreateInfoCount = 1;
	device_info.pQueueCreateInfos = &queue_info;
	device_info.enabledExtensionCount = ARRAY_COUNT(device_extensions);
	device_info.ppEnabledExtensionNames = device_extensions;
    device_info.pEnabledFeatures = &device_features;
    if (params.m_EnableValidationLayer)
    {
        device_info.enabledLayerCount = 1;
        device_info.ppEnabledLayerNames = &validation_layer;
    }
	VK(vkCreateDevice(device->m_PhysicalDevice, &device_info, NULL, &device->m_Device));

	vkGetDeviceQueue(device->m_Device, device->m_GraphicsQueueIndex, 0, &device->m_GraphicsQueue);

    VkCommandPoolCreateInfo command_pool_info = {};
    command_pool_info.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    command_pool_info.flags = VK_COMMAND_POOL_CREATE_TRANSIENT_BIT | VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
    command_pool_info.queueFamilyIndex = device->m_GraphicsQueueIndex;
    VK(vkCreateCommandPool(device->m_Device, &command_pool_info, NULL, &device->m_CommandPool));

	VmaAllocatorCreateInfo allocator_info = {};
	allocator_info.flags = VMA_ALLOCATOR_CREATE_EXTERNALLY_SYNCHRONIZED_BIT;
	allocator_info.physicalDevice = device->m_PhysicalDevice;
	allocator_info.device = device->m_Device;
	allocator_info.pAllocationCallbacks = NULL;
	VK(vmaCreateAllocator(&allocator_info, &device->m_Allocator));

	VkBufferCreateInfo staging_buffer_info = {};
	staging_buffer_info.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
	staging_buffer_info.size = GFX_STAGING_BUFFER_SIZE;
	staging_buffer_info.usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT | VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;
	staging_buffer_info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

	VmaAllocationCreateInfo staging_buffer_allocation_info = {};
	staging_buffer_allocation_info.usage = VMA_MEMORY_USAGE_CPU_ONLY;
	staging_buffer_allocation_info.flags = VMA_ALLOCATION_CREATE_MAPPED_BIT;

	VmaAllocationInfo allocation_info = {};
	VK(vmaCreateBuffer(device->m_Allocator, &staging_buffer_info, &staging_buffer_allocation_info, &device->m_StagingBuffer.m_Buffer, &device->m_StagingBuffer.m_Allocation, &allocation_info));
	device->m_StagingBufferMemory = allocation_info.deviceMemory;
	device->m_StagingBufferMemoryOffset = allocation_info.offset;
	device->m_StagingBufferMappedData = (uint8_t*)allocation_info.pMappedData;
	device->m_StagingBufferHead = 0;

    CreateSwapchain(device, params.m_BackBufferWidth, params.m_BackBufferHeight, params.m_DesiredBackBufferCount);

	return device;
}
void GfxDestroyDevice(GfxDevice device)
{
    DestroySwapchain(device);

	vmaDestroyBuffer(device->m_Allocator, device->m_StagingBuffer.m_Buffer, device->m_StagingBuffer.m_Allocation);
	vmaDestroyAllocator(device->m_Allocator);
    vkDestroyCommandPool(device->m_Device, device->m_CommandPool, NULL);
	vkDestroyDevice(device->m_Device, NULL);
	vkDestroySurfaceKHR(device->m_Instance, device->m_Surface, NULL);
    if (device->m_DebugCallback != VK_NULL_HANDLE)
    {
        PFN_vkDestroyDebugReportCallbackEXT vkDestroyDebugReportCallbackEXT = reinterpret_cast<PFN_vkDestroyDebugReportCallbackEXT>(vkGetInstanceProcAddr(device->m_Instance, "vkDestroyDebugReportCallbackEXT"));
        vkDestroyDebugReportCallbackEXT(device->m_Instance, device->m_DebugCallback, NULL);
    }
	vkDestroyInstance(device->m_Instance, NULL);

	Delete<GfxDevice_T>(device);
}

void GfxResizeSwapchain(GfxDevice device, uint32_t width, uint32_t height)
{
	DestroySwapchain(device);
	CreateSwapchain(device, width, height, device->m_SwapchainImageCount);
}

void GfxWaitForGpu(GfxDevice device)
{
    vkDeviceWaitIdle(device->m_Device);
}

GfxCommandBuffer GfxBeginFrame(GfxDevice device)
{
    GfxCommandBuffer cmd = &device->m_CommandBuffers[device->m_CommandBufferIndexCurr];

	VK(vkAcquireNextImageKHR(device->m_Device, device->m_Swapchain, UINT64_MAX, cmd->m_PresentSemaphore, VK_NULL_HANDLE, &device->m_SwapchainImageIndex));

    VK(vkWaitForFences(device->m_Device, 1, &cmd->m_CommandBufferFence, VK_TRUE, UINT64_MAX));
    VK(vkResetFences(device->m_Device, 1, &cmd->m_CommandBufferFence));

	VkCommandBufferBeginInfo cmd_begin_info = {};
	cmd_begin_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	cmd_begin_info.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
	VK(vkBeginCommandBuffer(cmd->m_CommandBuffer, &cmd_begin_info));

	{
        const uint32_t function_count = device->m_CmdFunctions.Count();
        for (uint32_t i = 0; i < function_count; ++i)
        {
            GfxCmdFunction func = device->m_CmdFunctions[i];
            void* user_data = static_cast<void*>(&device->m_CmdFunctionUserData[device->m_CmdFunctionUserDataOffsets[i]]);
            (*func)(cmd->m_CommandBuffer, user_data);
        }

        device->m_CmdFunctions.Clear();
        device->m_CmdFunctionUserData.Clear();
        device->m_CmdFunctionUserDataOffsets.Clear();
	}

    VK(vkResetDescriptorPool(device->m_Device, cmd->m_DescriptorPool, 0));

	return cmd;
}
void GfxEndFrame(GfxDevice device)
{
    GfxCommandBuffer cmd = &device->m_CommandBuffers[device->m_CommandBufferIndexCurr];

	VK(vkEndCommandBuffer(cmd->m_CommandBuffer));

    cmd->m_StagingBufferTail = device->m_StagingBufferHead;

	VkPipelineStageFlags wait_stage_flags = VK_PIPELINE_STAGE_ALL_COMMANDS_BIT;
	VkSubmitInfo submit_info = {};
	submit_info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
	submit_info.waitSemaphoreCount = 1;
	submit_info.pWaitSemaphores = &cmd->m_PresentSemaphore;
	submit_info.pWaitDstStageMask = &wait_stage_flags;
	submit_info.signalSemaphoreCount = 1;
	submit_info.pSignalSemaphores = &cmd->m_CommandBufferSemaphore;
	submit_info.commandBufferCount = 1;
	submit_info.pCommandBuffers = &cmd->m_CommandBuffer;
	VK(vkQueueSubmit(device->m_GraphicsQueue, 1, &submit_info, cmd->m_CommandBufferFence));

	VkPresentInfoKHR present_info = {};
	present_info.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
	present_info.waitSemaphoreCount = 1;
	present_info.pWaitSemaphores = &cmd->m_CommandBufferSemaphore;
	present_info.swapchainCount = 1;
	present_info.pSwapchains = &device->m_Swapchain;
	present_info.pImageIndices = &device->m_SwapchainImageIndex;
	VK(vkQueuePresentKHR(device->m_GraphicsQueue, &present_info));

	device->m_CommandBufferIndexCurr = device->m_CommandBufferIndexNext;
    device->m_CommandBufferIndexNext = (device->m_CommandBufferIndexCurr + 1) % device->m_SwapchainImageCount;
}

GfxAllocation GfxAllocateUploadBuffer(GfxDevice device, size_t size)
{
    GfxAllocation allocation;
    allocation.m_Buffer = &device->m_StagingBuffer;
    allocation.m_Offset = AllocateStagingBuffer(device, size);
    allocation.m_Data = static_cast<uint8_t*>(device->m_StagingBufferMappedData) + allocation.m_Offset;
    return allocation;
}

uint32_t GfxGetBackBufferCount(GfxDevice device)
{
    return device->m_SwapchainImageCount;
}
uint32_t GfxGetBackBufferIndex(GfxDevice device)
{
    return device->m_SwapchainImageIndex;
}
GfxTexture GfxGetBackBuffer(GfxDevice device, uint32_t index)
{
    return &device->m_SwapchainTextures[index];
}

GfxBuffer GfxCreateBuffer(GfxDevice device, const GfxCreateBufferParams& params)
{
	GfxBuffer buffer = New<GfxBuffer_T>();
	buffer->m_Size = params.m_Size;

	VkBufferCreateInfo buffer_info = {};
	buffer_info.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
	buffer_info.size = params.m_Size;
	buffer_info.usage = ToVkBufferUsageMask(params.m_Usage);
	buffer_info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

	VmaAllocationCreateInfo buffer_allocation_info = {};
	buffer_allocation_info.usage = VMA_MEMORY_USAGE_GPU_ONLY;

	VK(vmaCreateBuffer(device->m_Allocator, &buffer_info, &buffer_allocation_info, &buffer->m_Buffer, &buffer->m_Allocation, NULL));

	if (params.m_Data != NULL)
	{
        VkDeviceSize staging_buffer_offset = AllocateStagingBuffer(device, params.m_Size);
        memcpy(device->m_StagingBufferMappedData + staging_buffer_offset, params.m_Data, params.m_Size);

        CmdUploadBufferParams upload_params;
        upload_params.m_DstBuffer = buffer->m_Buffer;
        upload_params.m_DstOffset = 0;
        upload_params.m_DstAccessMask = VK_ACCESS_MEMORY_READ_BIT;
        upload_params.m_SrcBuffer = device->m_StagingBuffer.m_Buffer;
        upload_params.m_SrcOffset = staging_buffer_offset;
        upload_params.m_Size = params.m_Size;
        QueueCmd(device, &CmdUploadBuffer, &upload_params, sizeof(CmdUploadBufferParams));
	}

	return buffer;
}
void GfxDestroyBuffer(GfxDevice device, GfxBuffer buffer)
{
	vmaDestroyBuffer(device->m_Allocator, buffer->m_Buffer, buffer->m_Allocation);

	Delete<GfxBuffer_T>(buffer);
}

GfxTexture GfxCreateTexture(GfxDevice device, const GfxCreateTextureParams& params)
{
	GfxTexture texture = New<GfxTexture_T>();
	texture->m_Width = params.m_Width;
	texture->m_Height = params.m_Height;
    texture->m_Depth = params.m_Depth;
	texture->m_Format = ToVkFormat(params.m_Format);

	VkImageCreateInfo image_info = {};
	image_info.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
	image_info.imageType = ToVkImageType(params.m_Type);
	image_info.extent.width = texture->m_Width;
	image_info.extent.height = texture->m_Height;
	image_info.extent.depth = texture->m_Depth;
	image_info.mipLevels = params.m_GenerateMipmaps ? MipCount(params.m_Width, params.m_Height, 1) : 1;
	image_info.arrayLayers = 1;
	image_info.format = texture->m_Format;
	image_info.samples = VK_SAMPLE_COUNT_1_BIT;
	image_info.tiling = VK_IMAGE_TILING_OPTIMAL;
	image_info.usage = ToVkImageUsageMask(params.m_Usage);
	image_info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
	image_info.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;

	VmaAllocationCreateInfo image_allocation_info = {};
	image_allocation_info.usage = VMA_MEMORY_USAGE_GPU_ONLY;
	VK(vmaCreateImage(device->m_Allocator, &image_info, &image_allocation_info, &texture->m_Image, &texture->m_Allocation, NULL));

	VkImageViewCreateInfo image_view_info = {};
	image_view_info.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
	image_view_info.image = texture->m_Image;
	image_view_info.viewType = ToVkImageViewType(params.m_Type);
	image_view_info.format = texture->m_Format;
	image_view_info.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
	image_view_info.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
	image_view_info.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
	image_view_info.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
	image_view_info.subresourceRange.aspectMask = ToVkImageAspectMask(texture->m_Format);
	image_view_info.subresourceRange.baseMipLevel = 0;
	image_view_info.subresourceRange.levelCount = VK_REMAINING_MIP_LEVELS;
	image_view_info.subresourceRange.baseArrayLayer = 0;
	image_view_info.subresourceRange.layerCount = VK_REMAINING_ARRAY_LAYERS;
	VK(vkCreateImageView(device->m_Device, &image_view_info, NULL, &texture->m_ImageView));

	if (params.m_Data != NULL)
	{
        VkDeviceSize staging_buffer_offset = AllocateStagingBuffer(device, params.m_DataSize);
        memcpy(device->m_StagingBufferMappedData + staging_buffer_offset, params.m_Data, params.m_DataSize);

        if (params.m_GenerateMipmaps)
        {
            CmdGenerateMipmapParams mipmap_params;
            mipmap_params.m_DstImage = texture->m_Image;
            mipmap_params.m_DstWidth = texture->m_Width;
            mipmap_params.m_DstHeight = texture->m_Height;
            mipmap_params.m_DstMipCount = image_info.mipLevels;
            mipmap_params.m_DstAspectMask = ToVkImageAspectMask(texture->m_Format);
            mipmap_params.m_DstAccessMask = ToVkAccessMask(params.m_InitialState);
            mipmap_params.m_DstLayout = ToVkImageLayout(params.m_InitialState);
            mipmap_params.m_SrcBuffer = device->m_StagingBuffer.m_Buffer;
            mipmap_params.m_SrcOffset = staging_buffer_offset;
            QueueCmd(device, &CmdGenerateMipmap, &mipmap_params, sizeof(CmdGenerateMipmapParams));
        }
        else
        {
            CmdUploadImageParams upload_params;
            upload_params.m_DstImage = texture->m_Image;
            upload_params.m_DstWidth = texture->m_Width;
            upload_params.m_DstHeight = texture->m_Height;
            upload_params.m_DstAspectMask = ToVkImageAspectMask(texture->m_Format);
            upload_params.m_DstAccessMask = ToVkAccessMask(params.m_InitialState);
            upload_params.m_DstLayout = ToVkImageLayout(params.m_InitialState);
            upload_params.m_SrcBuffer = device->m_StagingBuffer.m_Buffer;
            upload_params.m_SrcOffset = staging_buffer_offset;
            QueueCmd(device, &CmdUploadImage, &upload_params, sizeof(CmdUploadImageParams));
        }
	}
	else
	{
        CmdTransitionImageParams transition_params;
        transition_params.m_DstImage = texture->m_Image;
        transition_params.m_DstAspectMask = ToVkImageAspectMask(texture->m_Format);
        transition_params.m_DstAccessMask = ToVkAccessMask(params.m_InitialState);
        transition_params.m_DstLayout = ToVkImageLayout(params.m_InitialState);
        QueueCmd(device, &CmdTransitionImage, &transition_params, sizeof(CmdTransitionImageParams));
	}

	return texture;
}
void GfxDestroyTexture(GfxDevice device, GfxTexture texture)
{
	vkDestroyImageView(device->m_Device, texture->m_ImageView, NULL);
	vmaDestroyImage(device->m_Allocator, texture->m_Image, texture->m_Allocation);

	Delete<GfxTexture_T>(texture);
}
GfxTexture GfxLoadTexture(GfxDevice device, const char* filepath)
{
    const size_t filepath_len = strlen(filepath);

    size_t blob_filepath_offset = 0;
    while (blob_filepath_offset < filepath_len &&
        (filepath[blob_filepath_offset] == '.' ||
         filepath[blob_filepath_offset] == '/' ||
         filepath[blob_filepath_offset] == '\\'))
    {
        ++blob_filepath_offset;
    }
    String blob_filepath("Data/%s.blob", filepath + blob_filepath_offset);

    void* image_data = NULL;
    void* blob_data = NULL;
    size_t image_size = 0;
    size_t blob_size = 0;
    bool image_loaded = ReadFile(filepath, "rb", &image_data, &image_size);
    bool blob_loaded = ReadFile(blob_filepath.Data(), "rb", &blob_data, &blob_size);
    if (!image_loaded && !blob_loaded)
    {
        Print("Error: Failed to read from file %s", filepath);
        return NULL;
    }

    bool create_new_blob = false;
    if (image_loaded && blob_loaded)
    {
        uint64_t image_checksum = GfxHash(static_cast<const char*>(image_data), image_size);
        uint64_t blob_checksum = *static_cast<const uint64_t*>(blob_data);
        create_new_blob = image_checksum != blob_checksum;
    }
    else if (image_loaded)
    {
        create_new_blob = true;
    }

    Blob blob;
    if (create_new_blob)
    {
        int width, height, component_count;
        stbi_uc* pixel_data = stbi_load_from_memory(static_cast<const stbi_uc*>(image_data), static_cast<int>(image_size), &width, &height, &component_count, STBI_rgb_alpha);
        ASSERT(pixel_data);
        size_t pixel_data_size = width * height * 4;

        blob.m_Size =
            sizeof(uint64_t) +                      // Checksum
            sizeof(uint32_t) +                      // Width
            sizeof(uint32_t) +                      // Height
            sizeof(uint64_t) + pixel_data_size;     // Pixel data
        blob.m_Data = Alloc(blob.m_Size);

        WriteStream stream(blob.m_Data, blob.m_Size);
        stream.WriteUint64(GfxHash(static_cast<const char*>(image_data), image_size));
        stream.WriteUint32(static_cast<uint32_t>(width));
        stream.WriteUint32(static_cast<uint32_t>(height));
        stream.Write(pixel_data, pixel_data_size);
        ASSERT(stream.IsEndOfStream());

        stbi_image_free(pixel_data);

        if (!WriteFile(blob_filepath.Data(), "wb", blob.m_Data, blob.m_Size))
        {
            Print("Error: Failed to write to file %s", blob_filepath.Data());
        }
    }
    else
    {
        blob.m_Data = blob_data;
        blob.m_Size = blob_size;
    }

    ReadStream stream(blob.m_Data, blob.m_Size);
    stream.ReadUint64(); // Checksum

    uint32_t width = stream.ReadUint32();
    uint32_t height = stream.ReadUint32();

    size_t pixel_data_size;
    const void* pixel_data = stream.Read(&pixel_data_size);

    GfxCreateTextureParams texture_params;
    texture_params.m_Width = width;
    texture_params.m_Height = height;
    texture_params.m_Format = GFX_FORMAT_R8G8B8A8_UNORM;
    texture_params.m_Usage = GFX_TEXTURE_USAGE_SAMPLE_BIT;
    texture_params.m_InitialState = GFX_TEXTURE_STATE_SHADER_READ;
    texture_params.m_Data = pixel_data;
    texture_params.m_DataSize = pixel_data_size;
    texture_params.m_GenerateMipmaps = true;
    GfxTexture texture = GfxCreateTexture(device, texture_params);

    if (create_new_blob)
        DestroyBlob(blob);
    if (image_loaded)
        Free(image_data);
    if (blob_loaded)
        Free(blob_data);

    Print("Loaded %s", create_new_blob ? filepath : blob_filepath.Data());

    return texture;
}

GfxSampler GfxCreateSampler(GfxDevice device, const GfxCreateSamplerParams& params)
{
	VkSamplerCreateInfo sampler_info = {};
	sampler_info.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
	sampler_info.magFilter = ToVkFilter(params.m_MagFilter);
	sampler_info.minFilter = ToVkFilter(params.m_MinFilter);
	sampler_info.mipmapMode = ToVkSamplerMipmapMode(params.m_MipFilter);
	sampler_info.addressModeU = ToVkSamplerAddressMode(params.m_AddressModeU);
	sampler_info.addressModeV = ToVkSamplerAddressMode(params.m_AddressModeV);
	sampler_info.addressModeW = ToVkSamplerAddressMode(params.m_AddressModeW);
	sampler_info.mipLodBias = params.m_MipLodBias;
	sampler_info.anisotropyEnable = params.m_AnisotropyEnable;
	sampler_info.maxAnisotropy = params.m_MaxAnisotropy;
	sampler_info.compareEnable = params.m_CompareEnable;
	sampler_info.compareOp = ToVkCompareOp(params.m_CompareOp);
	sampler_info.minLod = params.m_MinLod;
	sampler_info.maxLod = params.m_MaxLod;
	sampler_info.borderColor = ToVkBorderColor(params.m_BorderColor);

	VkSampler sampler = VK_NULL_HANDLE;
	VK(vkCreateSampler(device->m_Device, &sampler_info, NULL, &sampler));
	return reinterpret_cast<GfxSampler>(sampler);
}
void GfxDestroySampler(GfxDevice device, GfxSampler sampler)
{
	vkDestroySampler(device->m_Device, reinterpret_cast<VkSampler>(sampler), NULL);
}

GfxRenderSetup GfxCreateRenderSetup(GfxDevice device, GfxTechnique tech, const GfxCreateRenderSetupParams& params)
{
	GfxRenderSetup setup = New<GfxRenderSetup_T>();

	const uint32_t color_attachment_count = params.m_ColorAttachmentCount;
	const uint32_t depth_attachment_count = params.m_DepthAttachment != NULL ? 1 : 0;

	setup->m_Extent.width = color_attachment_count ? params.m_ColorAttachments[0]->m_Width : params.m_DepthAttachment->m_Width;
	setup->m_Extent.height = color_attachment_count ? params.m_ColorAttachments[0]->m_Height : params.m_DepthAttachment->m_Height;

    Array<VkImageView> image_views(color_attachment_count + depth_attachment_count);
	for (uint32_t i = 0; i < color_attachment_count; ++i)
	{
        image_views[i] = params.m_ColorAttachments[i]->m_ImageView;
	}
	if (depth_attachment_count)
	{
        image_views[color_attachment_count] = params.m_DepthAttachment->m_ImageView;
	}

	VkFramebufferCreateInfo framebuffer_info = {};
	framebuffer_info.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
	framebuffer_info.renderPass = tech->m_RenderPass;
	framebuffer_info.attachmentCount = image_views.Count();
	framebuffer_info.pAttachments = image_views.Data();
	framebuffer_info.width = setup->m_Extent.width;
	framebuffer_info.height = setup->m_Extent.height;
	framebuffer_info.layers = 1;
	VK(vkCreateFramebuffer(device->m_Device, &framebuffer_info, NULL, &setup->m_Framebuffer));

#ifdef _DEBUG
    setup->m_ImageViews.Resize(image_views.Count());
    memcpy(setup->m_ImageViews.Data(), image_views.Data(), sizeof(VkImageView) * image_views.Count());

    setup->m_Technique = tech;
    if (setup->m_Technique->m_RenderSetupTail)
    {
        setup->m_Technique->m_RenderSetupTail->m_Next = setup;
        setup->m_Prev = setup->m_Technique->m_RenderSetupTail;
    }
    else
    {
        setup->m_Technique->m_RenderSetupHead = setup;
        setup->m_Prev = NULL;
    }
    setup->m_Technique->m_RenderSetupTail = setup;
    setup->m_Next = NULL;
#endif

	return setup;
}
void GfxDestroyRenderSetup(GfxDevice device, GfxRenderSetup setup)
{
#ifdef _DEBUG
    if (setup->m_Technique->m_RenderSetupHead == setup)
        setup->m_Technique->m_RenderSetupHead = setup->m_Next;
    if (setup->m_Technique->m_RenderSetupTail == setup)
        setup->m_Technique->m_RenderSetupTail = setup->m_Prev;
    if (setup->m_Next)
        setup->m_Next->m_Prev = setup->m_Prev;
    if (setup->m_Prev)
        setup->m_Prev->m_Next = setup->m_Next;
#endif
	vkDestroyFramebuffer(device->m_Device, setup->m_Framebuffer, NULL);
	Delete<GfxRenderSetup_T>(setup);
}

void GfxCmdBeginTechnique(GfxCommandBuffer cmd, GfxTechnique tech)
{
    vkCmdBindPipeline(cmd->m_CommandBuffer, tech->m_BindPoint, tech->m_Pipeline);

    cmd->m_Technique = tech;

    cmd->m_DescriptorWrites.Resize(tech->m_ShaderBindingCount);
    cmd->m_DescriptorBufferInfo.Resize(tech->m_ShaderBindingCount);
    cmd->m_DescriptorImageInfo.Resize(tech->m_ShaderBindingCount);
    cmd->m_IsDescriptorSetDirty = false;
}
void GfxCmdEndTechnique(GfxCommandBuffer cmd)
{
    if (cmd->m_RenderSetup)
        vkCmdEndRenderPass(cmd->m_CommandBuffer);

    cmd->m_Technique = NULL;
    cmd->m_RenderSetup = NULL;
}

void GfxCmdSetRenderSetup(GfxCommandBuffer cmd, GfxRenderSetup setup)
{
    ASSERT(cmd->m_Technique->m_BindPoint == VK_PIPELINE_BIND_POINT_GRAPHICS);

    VkRenderPassBeginInfo render_pass_info = {};
    render_pass_info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    render_pass_info.renderPass = cmd->m_Technique->m_RenderPass;
    render_pass_info.framebuffer = setup->m_Framebuffer;
    render_pass_info.renderArea.offset = { 0, 0 };
    render_pass_info.renderArea.extent = setup->m_Extent;
    vkCmdBeginRenderPass(cmd->m_CommandBuffer, &render_pass_info, VK_SUBPASS_CONTENTS_INLINE);

    VkViewport viewport = { 0.f, 0.f, static_cast<float>(setup->m_Extent.width), static_cast<float>(setup->m_Extent.height), 0.f, 1.f };
    VkRect2D scissor = { { 0, 0 }, { setup->m_Extent.width, setup->m_Extent.height } };
    vkCmdSetViewport(cmd->m_CommandBuffer, 0, 1, &viewport);
    vkCmdSetScissor(cmd->m_CommandBuffer, 0, 1, &scissor);

    cmd->m_RenderSetup = setup;
}
void GfxCmdSetViewport(GfxCommandBuffer cmd, float x, float y, float w, float h, float min_z, float max_z)
{
    VkViewport viewport = { x, y, w, h, min_z, max_z };
    vkCmdSetViewport(cmd->m_CommandBuffer, 0, 1, &viewport);
}
void GfxCmdSetScissor(GfxCommandBuffer cmd, int32_t x, int32_t y, uint32_t w, uint32_t h)
{
    VkRect2D scissor = { { x, y }, { w, h } };
    vkCmdSetScissor(cmd->m_CommandBuffer, 0, 1, &scissor);
}
void GfxCmdClearColor(GfxCommandBuffer cmd, uint32_t attachment, const float color[4])
{
    VkClearRect clear_rect = {};
    clear_rect.baseArrayLayer = 0;
    clear_rect.layerCount = 1;
    clear_rect.rect.offset.x = 0;
    clear_rect.rect.offset.y = 0;
    clear_rect.rect.extent.width = cmd->m_RenderSetup->m_Extent.width;
    clear_rect.rect.extent.height = cmd->m_RenderSetup->m_Extent.height;

    VkClearAttachment clear_attachment = {};
    clear_attachment.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    clear_attachment.colorAttachment = attachment;
    clear_attachment.clearValue.color.float32[0] = color[0];
    clear_attachment.clearValue.color.float32[1] = color[1];
    clear_attachment.clearValue.color.float32[2] = color[2];
    clear_attachment.clearValue.color.float32[3] = color[3];
    vkCmdClearAttachments(cmd->m_CommandBuffer, 1, &clear_attachment, 1, &clear_rect);
}
void GfxCmdClearDepth(GfxCommandBuffer cmd, float depth, uint32_t stencil)
{
    VkClearRect clear_rect = {};
    clear_rect.baseArrayLayer = 0;
    clear_rect.layerCount = 1;
    clear_rect.rect.offset.x = 0;
    clear_rect.rect.offset.y = 0;
    clear_rect.rect.extent.width = cmd->m_RenderSetup->m_Extent.width;
    clear_rect.rect.extent.height = cmd->m_RenderSetup->m_Extent.height;

    VkClearAttachment clear_attachment = {};
    clear_attachment.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT;
    clear_attachment.clearValue.depthStencil.depth = depth;
    clear_attachment.clearValue.depthStencil.stencil = stencil;
    vkCmdClearAttachments(cmd->m_CommandBuffer, 1, &clear_attachment, 1, &clear_rect);
}

void GfxCmdBindVertexBuffer(GfxCommandBuffer cmd, uint32_t binding, GfxBuffer buffer, uint64_t offset)
{
    vkCmdBindVertexBuffers(cmd->m_CommandBuffer, binding, 1, &buffer->m_Buffer, &offset);
}
void GfxCmdBindIndexBuffer(GfxCommandBuffer cmd, GfxBuffer buffer, uint64_t offset, uint32_t stride)
{
    ASSERT(stride == sizeof(uint16_t) || stride == sizeof(uint32_t));
    vkCmdBindIndexBuffer(cmd->m_CommandBuffer, buffer->m_Buffer, offset, stride == sizeof(uint16_t) ? VK_INDEX_TYPE_UINT16 : VK_INDEX_TYPE_UINT32);
}

void GfxCmdSetBuffer(GfxCommandBuffer cmd, uint64_t hash, GfxBuffer buffer, uint64_t offset, uint64_t size)
{
    const GfxTechnique_T::ShaderBinding* binding = cmd->m_Technique->m_ShaderBindings.Find(hash);
    ASSERT(binding);
    ASSERT(binding->m_Type == VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER ||
           binding->m_Type == VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER ||
           binding->m_Type == VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER ||
           binding->m_Type == VK_DESCRIPTOR_TYPE_STORAGE_BUFFER ||
           binding->m_Type == VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC ||
           binding->m_Type == VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC);

    cmd->m_DescriptorBufferInfo[binding->m_Binding].buffer = buffer->m_Buffer;
    cmd->m_DescriptorBufferInfo[binding->m_Binding].offset = static_cast<VkDeviceSize>(offset);
    cmd->m_DescriptorBufferInfo[binding->m_Binding].range = static_cast<VkDeviceSize>(size);

    cmd->m_DescriptorWrites[binding->m_Binding].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    cmd->m_DescriptorWrites[binding->m_Binding].pNext = NULL;
    cmd->m_DescriptorWrites[binding->m_Binding].dstBinding = binding->m_Binding;
    cmd->m_DescriptorWrites[binding->m_Binding].dstArrayElement = 0;
    cmd->m_DescriptorWrites[binding->m_Binding].descriptorCount = 1;
    cmd->m_DescriptorWrites[binding->m_Binding].descriptorType = binding->m_Type;
    cmd->m_DescriptorWrites[binding->m_Binding].pBufferInfo = &cmd->m_DescriptorBufferInfo[binding->m_Binding];
    cmd->m_DescriptorWrites[binding->m_Binding].pImageInfo = NULL;
    cmd->m_DescriptorWrites[binding->m_Binding].pTexelBufferView = NULL;

    cmd->m_IsDescriptorSetDirty = true;
}
void GfxCmdSetTexture(GfxCommandBuffer cmd, uint64_t hash, GfxTexture texture, GfxTextureState state)
{
    const GfxTechnique_T::ShaderBinding* binding = cmd->m_Technique->m_ShaderBindings.Find(hash);
    ASSERT(binding);
    ASSERT(binding->m_Type == VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE ||
           binding->m_Type == VK_DESCRIPTOR_TYPE_STORAGE_IMAGE);

    cmd->m_DescriptorImageInfo[binding->m_Binding].imageView = texture->m_ImageView;
    cmd->m_DescriptorImageInfo[binding->m_Binding].imageLayout = ToVkImageLayout(state);
    cmd->m_DescriptorImageInfo[binding->m_Binding].sampler = NULL;

    cmd->m_DescriptorWrites[binding->m_Binding].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    cmd->m_DescriptorWrites[binding->m_Binding].pNext = NULL;
    cmd->m_DescriptorWrites[binding->m_Binding].dstBinding = binding->m_Binding;
    cmd->m_DescriptorWrites[binding->m_Binding].dstArrayElement = 0;
    cmd->m_DescriptorWrites[binding->m_Binding].descriptorCount = 1;
    cmd->m_DescriptorWrites[binding->m_Binding].descriptorType = binding->m_Type;
    cmd->m_DescriptorWrites[binding->m_Binding].pBufferInfo = NULL;
    cmd->m_DescriptorWrites[binding->m_Binding].pImageInfo = &cmd->m_DescriptorImageInfo[binding->m_Binding];
    cmd->m_DescriptorWrites[binding->m_Binding].pTexelBufferView = NULL;

    cmd->m_IsDescriptorSetDirty = true;
}
void GfxCmdSetSampler(GfxCommandBuffer cmd, uint64_t hash, GfxSampler sampler)
{
    const GfxTechnique_T::ShaderBinding* binding = cmd->m_Technique->m_ShaderBindings.Find(hash);
    ASSERT(binding);
    ASSERT(binding->m_Type == VK_DESCRIPTOR_TYPE_SAMPLER);

    cmd->m_DescriptorImageInfo[binding->m_Binding].imageView = NULL;
    cmd->m_DescriptorImageInfo[binding->m_Binding].imageLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    cmd->m_DescriptorImageInfo[binding->m_Binding].sampler = reinterpret_cast<VkSampler>(sampler);

    cmd->m_DescriptorWrites[binding->m_Binding].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    cmd->m_DescriptorWrites[binding->m_Binding].pNext = NULL;
    cmd->m_DescriptorWrites[binding->m_Binding].dstBinding = binding->m_Binding;
    cmd->m_DescriptorWrites[binding->m_Binding].dstArrayElement = 0;
    cmd->m_DescriptorWrites[binding->m_Binding].descriptorCount = 1;
    cmd->m_DescriptorWrites[binding->m_Binding].descriptorType = binding->m_Type;
    cmd->m_DescriptorWrites[binding->m_Binding].pBufferInfo = NULL;
    cmd->m_DescriptorWrites[binding->m_Binding].pImageInfo = &cmd->m_DescriptorImageInfo[binding->m_Binding];
    cmd->m_DescriptorWrites[binding->m_Binding].pTexelBufferView = NULL;

    cmd->m_IsDescriptorSetDirty = true;
}
void* GfxCmdAllocUploadBuffer(GfxCommandBuffer cmd, uint64_t hash, uint32_t size)
{
    const GfxTechnique_T::ShaderBinding* binding = cmd->m_Technique->m_ShaderBindings.Find(hash);
    ASSERT(binding);
    ASSERT(binding->m_Type == VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER ||
           binding->m_Type == VK_DESCRIPTOR_TYPE_STORAGE_BUFFER);

    VkDeviceSize offset = AllocateStagingBuffer(cmd->m_Device, size);

    cmd->m_DescriptorBufferInfo[binding->m_Binding].buffer = cmd->m_Device->m_StagingBuffer.m_Buffer;
    cmd->m_DescriptorBufferInfo[binding->m_Binding].offset = offset;
    cmd->m_DescriptorBufferInfo[binding->m_Binding].range = size;

    cmd->m_DescriptorWrites[binding->m_Binding].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    cmd->m_DescriptorWrites[binding->m_Binding].pNext = NULL;
    cmd->m_DescriptorWrites[binding->m_Binding].dstBinding = binding->m_Binding;
    cmd->m_DescriptorWrites[binding->m_Binding].dstArrayElement = 0;
    cmd->m_DescriptorWrites[binding->m_Binding].descriptorCount = 1;
    cmd->m_DescriptorWrites[binding->m_Binding].descriptorType = binding->m_Type;
    cmd->m_DescriptorWrites[binding->m_Binding].pBufferInfo = &cmd->m_DescriptorBufferInfo[binding->m_Binding];
    cmd->m_DescriptorWrites[binding->m_Binding].pImageInfo = NULL;
    cmd->m_DescriptorWrites[binding->m_Binding].pTexelBufferView = NULL;

    cmd->m_IsDescriptorSetDirty = true;

    return cmd->m_Device->m_StagingBufferMappedData + offset;
}

static void UpdateDescriptorSet(GfxCommandBuffer cmd)
{
    if (cmd->m_IsDescriptorSetDirty)
    {
        VkDescriptorSetAllocateInfo alloc_info = {};
        alloc_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
        alloc_info.descriptorPool = cmd->m_DescriptorPool;
        alloc_info.descriptorSetCount = 1;
        alloc_info.pSetLayouts = &cmd->m_Technique->m_DescriptorSetLayout;
        VkDescriptorSet set = VK_NULL_HANDLE;
        VK(vkAllocateDescriptorSets(cmd->m_Device->m_Device, &alloc_info, &set));

        for (uint32_t i = 0; i < cmd->m_Technique->m_ShaderBindingCount; ++i)
        {
            cmd->m_DescriptorWrites[i].dstSet = set;
        }
        vkUpdateDescriptorSets(cmd->m_Device->m_Device, cmd->m_DescriptorWrites.Count(), cmd->m_DescriptorWrites.Data(), 0, NULL);

        vkCmdBindDescriptorSets(cmd->m_CommandBuffer, cmd->m_Technique->m_BindPoint, cmd->m_Technique->m_PipelineLayout, 0, 1, &set, 0, NULL);

        cmd->m_IsDescriptorSetDirty = false;
    }
}

void GfxCmdDraw(GfxCommandBuffer cmd, uint32_t vertex_count, uint32_t instance_count, uint32_t first_vertex)
{
    UpdateDescriptorSet(cmd);
    vkCmdDraw(cmd->m_CommandBuffer, vertex_count, instance_count, first_vertex, 0);
}
void GfxCmdDrawIndexed(GfxCommandBuffer cmd, uint32_t index_count, uint32_t instance_count, uint32_t first_index, int32_t vertex_offset)
{
    UpdateDescriptorSet(cmd);
    vkCmdDrawIndexed(cmd->m_CommandBuffer, index_count, instance_count, first_index, vertex_offset, 0);
}
void GfxCmdDispatch(GfxCommandBuffer cmd, uint32_t group_count_x, uint32_t group_count_y, uint32_t group_count_z)
{
    UpdateDescriptorSet(cmd);
    vkCmdDispatch(cmd->m_CommandBuffer, group_count_x, group_count_y, group_count_z);
}

void GfxCmdCopyBuffer(GfxCommandBuffer cmd, GfxBuffer dst_buffer, uint64_t dst_offset, GfxBuffer src_buffer, uint64_t src_offset, uint64_t size)
{
    VkBufferCopy copy_region;
    copy_region.srcOffset = src_offset;
    copy_region.dstOffset = dst_offset;
    copy_region.size = size;
    vkCmdCopyBuffer(cmd->m_CommandBuffer, src_buffer->m_Buffer, dst_buffer->m_Buffer, 1, &copy_region);
}
void GfxCmdBlitTexture(GfxCommandBuffer cmd, GfxTexture dst_texture, GfxTexture src_texture)
{
    VkImageBlit region = {};
    region.srcSubresource.aspectMask = ToVkImageAspectMask(src_texture->m_Format);
    region.dstSubresource.aspectMask = ToVkImageAspectMask(src_texture->m_Format);
    region.srcSubresource.mipLevel = region.dstSubresource.mipLevel = 0;
    region.srcSubresource.baseArrayLayer = region.dstSubresource.baseArrayLayer = 0;
    region.srcSubresource.layerCount = region.dstSubresource.layerCount = 1;
    region.srcOffsets[1].x = src_texture->m_Width;
    region.srcOffsets[1].y = src_texture->m_Height;
    region.dstOffsets[1].x = dst_texture->m_Width;
    region.dstOffsets[1].y = dst_texture->m_Height;
    region.srcOffsets[1].z = region.dstOffsets[1].z = 1;
    vkCmdBlitImage(cmd->m_CommandBuffer, src_texture->m_Image, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, dst_texture->m_Image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &region, VK_FILTER_NEAREST);
}

void GfxCmdTransitionBuffer(GfxCommandBuffer cmd, GfxBuffer buffer, GfxBufferAccess old_access, GfxBufferAccess new_access, uint64_t offset, uint64_t size)
{
    VkBufferMemoryBarrier buffer_barrier = {};
    buffer_barrier.sType = VK_STRUCTURE_TYPE_BUFFER_MEMORY_BARRIER;
    buffer_barrier.srcAccessMask = ToVkAccessMask(old_access);
    buffer_barrier.dstAccessMask = ToVkAccessMask(new_access);
    buffer_barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    buffer_barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    buffer_barrier.buffer = buffer->m_Buffer;
    buffer_barrier.offset = offset;
    buffer_barrier.size = size == ~0ULL ? buffer->m_Size - offset : size;
    vkCmdPipelineBarrier(cmd->m_CommandBuffer, ToVkPipelineStageMask(old_access), ToVkPipelineStageMask(new_access), 0, 0, NULL, 1, &buffer_barrier, 0, NULL);
}
void GfxCmdTransitionTexture(GfxCommandBuffer cmd, GfxTexture texture, GfxTextureState old_state, GfxTextureState new_state, uint32_t base_mip, uint32_t mip_count, uint32_t base_layer, uint32_t layer_count)
{
    VkImageMemoryBarrier image_barrier = {};
    image_barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
    image_barrier.oldLayout = ToVkImageLayout(old_state);
    image_barrier.newLayout = ToVkImageLayout(new_state);
    image_barrier.srcAccessMask = ToVkAccessMask(old_state);
    image_barrier.dstAccessMask = ToVkAccessMask(new_state);
    image_barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    image_barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    image_barrier.image = texture->m_Image;
    image_barrier.subresourceRange.aspectMask = ToVkImageAspectMask(texture->m_Format);
    image_barrier.subresourceRange.baseMipLevel = base_mip;
    image_barrier.subresourceRange.levelCount = mip_count;
    image_barrier.subresourceRange.baseArrayLayer = base_layer;
    image_barrier.subresourceRange.layerCount = layer_count;
    vkCmdPipelineBarrier(cmd->m_CommandBuffer, ToVkPipelineStageMask(old_state), ToVkPipelineStageMask(new_state), 0, 0, NULL, 0, NULL, 1, &image_barrier);
}
