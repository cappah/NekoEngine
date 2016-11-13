/* NekoEngine
 *
 * VKUtil.h
 * Author: Alexandru Naiman
 *
 * Vulkan helper functions
 *
 * -----------------------------------------------------------------------------
 *
 * Copyright (c) 2015-2016, Alexandru Naiman
 *
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice,
 * this list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 * this list of conditions and the following disclaimer in the documentation
 * and/or other materials provided with the distribution.
 *
 * 3. Neither the name of the copyright holder nor the names of its contributors
 * may be used to endorse or promote products derived from this software without
 * specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY ALEXANDRU NAIMAN "AS IS" AND ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARANTIES OF
 * MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
 * IN NO EVENT SHALL ALEXANDRU NAIMAN BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA,
 * OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
 * LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
 * NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,
 * EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#pragma once

#include <vulkan/vulkan.h>

#if defined(ENGINE_INTERNAL) || defined(VKUTIL_USE_NE_LOGGER)
#include <System/Logger.h>
#define VKUTIL_ERR(a) { Logger::Log("VKUtil", LOG_CRITICAL, a); return false; }
#define VKUTIL_ERR_HANDLE(a) { Logger::Log("VKUtil", LOG_CRITICAL, a); return VK_NULL_HANDLE; }
#elif defined(VKUTIL_USE_EXCEPTIONS)
#include <stdexcept>
#define VKUTIL_ERR(a) throw std::runtime_error(a);
#define VKUTIL_ERR_HANDLE(a) { throw std::runtime_error(a); }
#else
#include <stdio.h>
#define VKUTIL_ERR(a) { fprintf(stderr, "VKUtil: %s\n", a); return false; }
#define VKUTIL_ERR_HANDLE(a) { fprintf(stderr, "VKUtil: %s\n", a); return VK_NULL_HANDLE; }
#endif

class VKUtil
{
public:
	static inline void Initialize(VkDevice device, VkPhysicalDevice physicalDevice, VkQueue graphicsQueue, VkCommandPool graphicsCommandPool, VkQueue computeQueue, VkCommandPool computeCommandPool, VkAllocationCallbacks *allocator = nullptr, VkQueue transferQueue = VK_NULL_HANDLE, VkCommandPool transferCommandPool = VK_NULL_HANDLE)
	{
		_device = device;
		_graphicsQueue = graphicsQueue;
		_computeQueue = computeQueue;
		_transferQueue = transferQueue;
		_physicalDevice = physicalDevice;
		_graphicsCommandPool = graphicsCommandPool;
		_computeCommandPool = computeCommandPool;
		_transferCommandPool = transferCommandPool;
		_allocator = allocator;
	}

	static inline VkDevice GetDevice() { return _device; }
	static inline VkAllocationCallbacks *GetAllocator() { return _allocator; }
	static inline VkCommandPool GetGraphicsCommandPool() { return _graphicsCommandPool; }
	static inline VkCommandPool GetComputeCommandPool() { return _computeCommandPool; }
	static inline VkCommandPool GetTransferCommandPool() { return _transferCommandPool; }
	static inline VkQueue GetGraphicsQueue() { return _graphicsQueue; }
	static inline VkQueue GetComputeQueue() { return _computeQueue; }
	static inline VkQueue GetTransferQueue() { return _transferQueue; }

	static inline uint32_t GetMemoryType(uint32_t filter, VkMemoryPropertyFlags properties)
	{
		VkPhysicalDeviceMemoryProperties memProperties;
		vkGetPhysicalDeviceMemoryProperties(_physicalDevice, &memProperties);

		for (uint32_t i = 0; i < memProperties.memoryTypeCount; ++i)
			if ((filter & (1 << i)) && ((memProperties.memoryTypes[i].propertyFlags & properties) == properties))
				return i;

		// warn

		return 0;
	}

	// Command buffers

	static inline VkCommandBuffer CreateCommandBuffer(VkCommandBufferLevel level, VkCommandPool commandPool = _graphicsCommandPool)
	{
		VkCommandBuffer commandBuffer;

		VkCommandBufferAllocateInfo allocInfo = {};
		allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
		allocInfo.commandPool = commandPool;
		allocInfo.level = level;
		allocInfo.commandBufferCount = 1;

		if (vkAllocateCommandBuffers(_device, &allocInfo, &commandBuffer) != VK_SUCCESS)
			VKUTIL_ERR_HANDLE("Failed to allocate command buffer");

		return commandBuffer;
	}

	static inline void FreeCommandBuffer(VkCommandBuffer commandBuffer, VkCommandPool commandPool = _graphicsCommandPool)
	{
		vkFreeCommandBuffers(_device, commandPool, 1, &commandBuffer);
	}

	static inline VkCommandBuffer CreateOneShotCmdBuffer(VkCommandPool pool = _graphicsCommandPool)
	{
		VkCommandBufferAllocateInfo allocInfo = {};
		allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
		allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
		allocInfo.commandPool = pool;
		allocInfo.commandBufferCount = 1;

		VkCommandBuffer commandBuffer;
		if(vkAllocateCommandBuffers(_device, &allocInfo, &commandBuffer) != VK_SUCCESS)
			VKUTIL_ERR_HANDLE("Failed to allocate command buffer");

		VkCommandBufferBeginInfo beginInfo = {};
		beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
		beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

		vkBeginCommandBuffer(commandBuffer, &beginInfo);

		return commandBuffer;
	}

	static inline void ExecuteOneShotCmdBuffer(VkCommandBuffer commandBuffer, VkCommandPool pool = _graphicsCommandPool, VkQueue queue = _graphicsQueue, VkSemaphore wait = VK_NULL_HANDLE, VkPipelineStageFlags waitDst = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT)
	{
		vkEndCommandBuffer(commandBuffer);

		VkSubmitInfo submitInfo = {};
		submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
		submitInfo.commandBufferCount = 1;
		submitInfo.pCommandBuffers = &commandBuffer;

		if (wait != VK_NULL_HANDLE)
		{
			VkPipelineStageFlags flags[] = { waitDst };
			submitInfo.pWaitDstStageMask = flags;
			submitInfo.waitSemaphoreCount = 1;
			submitInfo.pWaitSemaphores = &wait;
		}

		vkQueueSubmit(queue, 1, &submitInfo, VK_NULL_HANDLE);

		if (wait == VK_NULL_HANDLE) vkQueueWaitIdle(queue);

		vkFreeCommandBuffers(_device, pool, 1, &commandBuffer);
	}

	// Buffers 

	static inline bool CreateBuffer(VkBuffer &buffer, VkDeviceMemory &memory, VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, VkDeviceSize offset = 0, VkSharingMode sharingMode = VK_SHARING_MODE_EXCLUSIVE)
	{
		VkBufferCreateInfo bufferInfo = {};
		bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
		bufferInfo.size = size;
		bufferInfo.usage = usage;
		bufferInfo.sharingMode = sharingMode;

		if (vkCreateBuffer(_device, &bufferInfo, nullptr, &buffer) != VK_SUCCESS)
			VKUTIL_ERR("Failed to create buffer");

		if (memory == VK_NULL_HANDLE)
		{
			VkMemoryRequirements memReq;
			vkGetBufferMemoryRequirements(_device, buffer, &memReq);

			VkMemoryAllocateInfo allocInfo = {};
			allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
			allocInfo.allocationSize = memReq.size;
			allocInfo.memoryTypeIndex = GetMemoryType(memReq.memoryTypeBits, properties);

			if (vkAllocateMemory(_device, &allocInfo, nullptr, &memory) != VK_SUCCESS)
				VKUTIL_ERR("Failed to allocate memory");
		}

		if (vkBindBufferMemory(_device, buffer, memory, 0) != VK_SUCCESS)
			VKUTIL_ERR("Failed to bind buffer memory");

		return true;
	}

	static inline void CopyBuffer(VkBuffer src, VkBuffer dst, VkDeviceSize size, VkDeviceSize srcOffset = 0, VkDeviceSize dstOffset = 0, VkCommandBuffer cmdBuffer = VK_NULL_HANDLE, bool submit = false, VkCommandPool pool = _graphicsCommandPool, VkQueue queue = _graphicsQueue)
	{
		if (cmdBuffer == VK_NULL_HANDLE)
		{
			cmdBuffer = CreateOneShotCmdBuffer();
			submit = true;
		}

		VkBufferCopy copyRegion = {};
		copyRegion.srcOffset = srcOffset;
		copyRegion.dstOffset = dstOffset;
		copyRegion.size = size;
		vkCmdCopyBuffer(cmdBuffer, src, dst, 1, &copyRegion);

		if (submit)
			ExecuteOneShotCmdBuffer(cmdBuffer, pool, queue);
	}

	// Images

	static inline bool CreateImage(VkImage &image, VkDeviceMemory &memory, int width, int height, int depth,
		VkMemoryPropertyFlags properties,
		VkFormat format = VK_FORMAT_R8G8B8A8_UNORM,
		VkImageType type = VK_IMAGE_TYPE_2D,
		VkImageUsageFlags usage = VK_IMAGE_USAGE_SAMPLED_BIT,
		VkImageTiling tiling = VK_IMAGE_TILING_LINEAR,
		uint32_t mipLevels = 1,
		uint32_t arrayLayers = 1,
		VkImageCreateFlags createFlags = 0,
		VkSampleCountFlagBits samples = VK_SAMPLE_COUNT_1_BIT)
	{
		VkImageCreateInfo imgCreateInfo = {};
		imgCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
		imgCreateInfo.format = format;
		imgCreateInfo.imageType = type;
		imgCreateInfo.initialLayout = VK_IMAGE_LAYOUT_PREINITIALIZED;
		imgCreateInfo.mipLevels = mipLevels;
		imgCreateInfo.arrayLayers = arrayLayers;
		imgCreateInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
		imgCreateInfo.samples = samples;
		imgCreateInfo.usage = usage;
		imgCreateInfo.tiling = tiling;
		imgCreateInfo.extent.width = width;
		imgCreateInfo.extent.height = height;
		imgCreateInfo.extent.depth = depth;
		imgCreateInfo.flags = createFlags;

		if (vkCreateImage(_device, &imgCreateInfo, _allocator, &image) != VK_SUCCESS)
			VKUTIL_ERR("Failed to create image");

		VkMemoryRequirements memReq;
		vkGetImageMemoryRequirements(_device, image, &memReq);

		VkMemoryAllocateInfo allocInfo = {};
		allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
		allocInfo.allocationSize = memReq.size;
		allocInfo.memoryTypeIndex = GetMemoryType(memReq.memoryTypeBits, properties);

		if (vkAllocateMemory(_device, &allocInfo, nullptr, &memory) != VK_SUCCESS)
			VKUTIL_ERR("Failed to allocate image memory");

		vkBindImageMemory(_device, image, memory, 0);

		return true;
	}

	static inline bool CreateImageView(VkImageView &imageView, VkImage image, VkImageViewType type = VK_IMAGE_VIEW_TYPE_2D, 
		VkFormat format = VK_FORMAT_R8G8B8A8_UNORM, VkImageAspectFlags aspect = VK_IMAGE_ASPECT_COLOR_BIT,
		int baseMip = 0, int mipLevels = 1, int baseLayer = 0, int layerCount = 1)
	{
		VkImageViewCreateInfo viewInfo = {};
		viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
		viewInfo.image = image;
		viewInfo.viewType = type;
		viewInfo.format = format;
		viewInfo.subresourceRange.aspectMask = aspect;
		viewInfo.subresourceRange.baseMipLevel = baseMip;
		viewInfo.subresourceRange.levelCount = mipLevels;
		viewInfo.subresourceRange.baseArrayLayer = baseLayer;
		viewInfo.subresourceRange.layerCount = layerCount;
		viewInfo.components = { VK_COMPONENT_SWIZZLE_R, VK_COMPONENT_SWIZZLE_G, VK_COMPONENT_SWIZZLE_B, VK_COMPONENT_SWIZZLE_A };

		if (vkCreateImageView(_device, &viewInfo, _allocator, &imageView) != VK_SUCCESS)
			VKUTIL_ERR("Failed to create image view");

		return true;
	}

	static inline bool CreateSampler(VkSampler &sampler, VkFilter minFilter = VK_FILTER_LINEAR, VkFilter magFilter = VK_FILTER_LINEAR,
		VkSamplerAddressMode addressModeU = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE, VkSamplerAddressMode addressModeV = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE,
		VkSamplerAddressMode addressModeW = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE, float maxAnisotropy = 16.f, VkSamplerMipmapMode mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR,
		float minLodBias = 0.f, float minLod = 0.f, float maxLod = 0.f, VkBorderColor borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK,
		VkBool32 unnormalizedCoordinates = VK_FALSE, VkBool32 enableCompare = VK_FALSE, VkCompareOp compareOp = VK_COMPARE_OP_ALWAYS)
	{
		VkSamplerCreateInfo samplerInfo = {};
		samplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
		samplerInfo.minFilter = minFilter;
		samplerInfo.magFilter = magFilter;
		samplerInfo.addressModeU = addressModeU;
		samplerInfo.addressModeV = addressModeV;
		samplerInfo.addressModeW = addressModeW;
		samplerInfo.borderColor = borderColor;
		samplerInfo.unnormalizedCoordinates = unnormalizedCoordinates;
		samplerInfo.compareEnable = enableCompare;
		samplerInfo.compareOp = compareOp;
		samplerInfo.mipmapMode = mipmapMode;
		samplerInfo.mipLodBias = minLodBias;
		samplerInfo.minLod = minLod;
		samplerInfo.maxLod = maxLod;

		if (maxAnisotropy < .1f)
			samplerInfo.anisotropyEnable = VK_FALSE;
		else
		{
			samplerInfo.anisotropyEnable = VK_TRUE;
			samplerInfo.maxAnisotropy = 16;
		}

		if (vkCreateSampler(_device, &samplerInfo, _allocator, &sampler) != VK_SUCCESS)
			VKUTIL_ERR("Failed to create sampler");

		return true;
	}

	static inline void CopyImage(VkImage src, VkImage dst, uint32_t width, uint32_t height, VkCommandBuffer cmdBuffer = VK_NULL_HANDLE, bool submit = false, VkCommandPool pool = _graphicsCommandPool, VkQueue queue = _graphicsQueue)
	{
		if (cmdBuffer == VK_NULL_HANDLE)
		{
			cmdBuffer = CreateOneShotCmdBuffer();
			submit = true;
		}

		VkImageSubresourceLayers subResource = {};
		subResource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		subResource.baseArrayLayer = 0;
		subResource.mipLevel = 0;
		subResource.layerCount = 1;

		VkImageCopy region = {};
		region.srcSubresource = subResource;
		region.dstSubresource = subResource;
		region.srcOffset = { 0, 0, 0 };
		region.dstOffset = { 0, 0, 0 };
		region.extent.width = width;
		region.extent.height = height;
		region.extent.depth = 1;

		vkCmdCopyImage(cmdBuffer, src, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, dst, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &region);

		if (submit)
			ExecuteOneShotCmdBuffer(cmdBuffer, pool, queue);
	}

	static inline void BlitImage(VkImage src, VkImage dst, int32_t srcWidth, int32_t srcHeight, int32_t dstWidth, int32_t dstHeight, VkFilter filter, VkCommandBuffer cmdBuffer = VK_NULL_HANDLE, bool submit = false, VkCommandPool pool = _graphicsCommandPool, VkQueue queue = _graphicsQueue)
	{
		if (cmdBuffer == VK_NULL_HANDLE)
		{
			cmdBuffer = CreateOneShotCmdBuffer();
			submit = true;
		}

		VkImageSubresourceLayers subResource = {};
		subResource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		subResource.baseArrayLayer = 0;
		subResource.mipLevel = 0;
		subResource.layerCount = 1;

		VkImageBlit region = {};
		region.srcSubresource = subResource;
		region.dstSubresource = subResource;
		region.srcOffsets[0] = { 0, 0, 0 };
		region.srcOffsets[1] = { srcWidth, srcHeight, 1 };
		region.dstOffsets[0] = { 0, 0, 0 };
		region.dstOffsets[1] = { dstWidth, dstHeight, 1 };

		vkCmdBlitImage(cmdBuffer, src, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, dst, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &region, filter);

		if (submit)
			ExecuteOneShotCmdBuffer(cmdBuffer, pool, queue);
	}

	static inline void CopyBufferToImage(VkBuffer src, VkImage dst, uint32_t width, uint32_t height, VkCommandBuffer cmdBuffer = VK_NULL_HANDLE, bool submit = false, VkCommandPool pool = _graphicsCommandPool, VkQueue queue = _graphicsQueue)
	{
		VkImageSubresourceLayers subResource = {};
		subResource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		subResource.baseArrayLayer = 0;
		subResource.mipLevel = 0;
		subResource.layerCount = 1;
		CopyBufferToImage(src, dst, width, height, 0, subResource, cmdBuffer, submit, pool, queue);
	}

	static inline void CopyBufferToImage(VkBuffer src, VkImage dst, uint32_t width, uint32_t height, VkDeviceSize bufferOffset, VkImageSubresourceLayers &subResource, VkCommandBuffer cmdBuffer = VK_NULL_HANDLE, bool submit = false, VkCommandPool pool = _graphicsCommandPool, VkQueue queue = _graphicsQueue)
	{
		if (cmdBuffer == VK_NULL_HANDLE)
		{
			cmdBuffer = CreateOneShotCmdBuffer();
			submit = true;
		}

		VkBufferImageCopy region = {};
		region.bufferImageHeight = height;
		region.bufferRowLength = width;
		region.imageSubresource = subResource;
		region.imageOffset = { 0, 0, 0 };
		region.bufferOffset = bufferOffset;
		region.imageExtent.width = width;
		region.imageExtent.height = height;
		region.imageExtent.depth = 1;

		vkCmdCopyBufferToImage(cmdBuffer, src, dst, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &region);

		if (submit)
			ExecuteOneShotCmdBuffer(cmdBuffer, pool, queue);
	}

	static inline void TransitionImageLayout(VkImage image, VkImageLayout oldLayout, VkImageLayout newLayout, VkImageAspectFlags aspect = VK_IMAGE_ASPECT_COLOR_BIT, VkCommandBuffer cmdBuffer = VK_NULL_HANDLE, bool submit = false, VkCommandPool cmdPool = _graphicsCommandPool, VkQueue queue = _graphicsQueue)
	{
		VkImageSubresourceRange range{};
		range.aspectMask = aspect;
		range.baseMipLevel = 0;
		range.levelCount = 1;
		range.baseArrayLayer = 0;
		range.layerCount = 1;
		TransitionImageLayout(image, oldLayout, newLayout, range, cmdBuffer, submit, cmdPool, queue);
	}
	
	static inline void TransitionImageLayout(VkImage image, VkImageLayout oldLayout, VkImageLayout newLayout, VkImageSubresourceRange &range, VkCommandBuffer cmdBuffer = VK_NULL_HANDLE, bool submit = false, VkCommandPool cmdPool = _graphicsCommandPool, VkQueue queue = _graphicsQueue)
	{
		if (cmdBuffer == VK_NULL_HANDLE)
		{
			cmdBuffer = CreateOneShotCmdBuffer();
			submit = true;
		}

		VkImageMemoryBarrier barrier = {};
		barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
		barrier.oldLayout = oldLayout;
		barrier.newLayout = newLayout;
		barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		barrier.image = image;
		barrier.subresourceRange = range;

		switch (oldLayout)
		{
			case VK_IMAGE_LAYOUT_PREINITIALIZED:
				barrier.srcAccessMask = VK_ACCESS_HOST_WRITE_BIT;
			break;
			case VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL:
				barrier.srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
			break;
			case VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL:
				barrier.srcAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
			break;
			case VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL:
				barrier.srcAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
			break;
			case VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL:
				barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
			break;
			case VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL:
				barrier.srcAccessMask = VK_ACCESS_SHADER_READ_BIT;
			break;
			default:
				barrier.srcAccessMask = 0;
			break;
		}

		switch (newLayout)
		{
			case VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL:
				barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
			break;
			case VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL:
				barrier.dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
			break;
			case VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL:
				barrier.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
			break;
			case VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL:
				barrier.dstAccessMask |= VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
			break;
			case VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL:
				barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
				barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
			break;
			default:
				barrier.dstAccessMask = 0;
			break;
		}

		vkCmdPipelineBarrier(cmdBuffer, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, 0, 0, nullptr, 0, nullptr, 1, &barrier);

		if (submit)
			ExecuteOneShotCmdBuffer(cmdBuffer, cmdPool, queue);
	}

private:
	static VkDevice _device;
	static VkQueue _graphicsQueue;
	static VkQueue _computeQueue;
	static VkQueue _transferQueue;
	static VkCommandPool _graphicsCommandPool;
	static VkCommandPool _computeCommandPool;
	static VkCommandPool _transferCommandPool;
	static VkPhysicalDevice _physicalDevice;
	static VkAllocationCallbacks *_allocator;
};

#define VKUTIL_OBJS VkDevice VKUtil::_device = VK_NULL_HANDLE; \
	VkPhysicalDevice VKUtil::_physicalDevice = VK_NULL_HANDLE; \
	VkQueue VKUtil::_graphicsQueue = VK_NULL_HANDLE; \
	VkQueue VKUtil::_computeQueue = VK_NULL_HANDLE; \
	VkQueue VKUtil::_transferQueue = VK_NULL_HANDLE; \
	VkCommandPool VKUtil::_graphicsCommandPool = VK_NULL_HANDLE; \
	VkCommandPool VKUtil::_computeCommandPool = VK_NULL_HANDLE; \
	VkCommandPool VKUtil::_transferCommandPool = VK_NULL_HANDLE; \
	VkAllocationCallbacks *VKUtil::_allocator = nullptr

