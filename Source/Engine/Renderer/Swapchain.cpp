/* NekoEngine
 *
 * Swapchain.cpp
 * Author: Alexandru Naiman
 *
 * Vulkan swapchain
 *
 * -----------------------------------------------------------------------------
 *
 * Copyright (c) 2015-2017, Alexandru Naiman
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
 * and/or other materials provided with the distribution.]
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

#include <algorithm>

#include <Engine/Engine.h>
#include <System/Logger.h>
#include <Renderer/VKUtil.h>
#include <Renderer/Swapchain.h>

#define SWAPCHAIN_MODULE	"Swapchain"

using namespace std;

Swapchain::Swapchain(SwapchainInfo &info)
{
	_swapchain = VK_NULL_HANDLE;
	_info = info;

	if (!_Create())
	{ DIE("Failed to create swapchain !"); }
}

bool Swapchain::_Create()
{
	VkSurfaceFormatKHR surfaceFormat{ VK_FORMAT_B8G8R8A8_UNORM, VK_COLOR_SPACE_SRGB_NONLINEAR_KHR };
	VkPresentModeKHR presentMode{ VK_PRESENT_MODE_FIFO_KHR };

	for (VkPresentModeKHR &mode : _info.presentModes)
		if (mode == VK_PRESENT_MODE_MAILBOX_KHR)
			presentMode = VK_PRESENT_MODE_MAILBOX_KHR;

	_extent = { Engine::GetConfiguration().Engine.ScreenWidth, Engine::GetConfiguration().Engine.ScreenHeight };
	_extent.width = std::max(_info.capabilities.minImageExtent.width, std::min(_info.capabilities.maxImageExtent.width, _extent.width));
	_extent.height = std::max(_info.capabilities.minImageExtent.height, std::min(_info.capabilities.maxImageExtent.height, _extent.height));

	uint32_t imageCount{ _info.capabilities.minImageCount + 1 };
	if (_info.capabilities.maxImageCount > 0 && imageCount > _info.capabilities.maxImageCount)
		imageCount = _info.capabilities.maxImageCount;

	VkSwapchainCreateInfoKHR createInfo{};
	createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
	createInfo.surface = _info.surface;
	createInfo.minImageCount = imageCount;
	createInfo.imageFormat = surfaceFormat.format;
	createInfo.imageColorSpace = surfaceFormat.colorSpace;
	createInfo.imageExtent = _extent;
	createInfo.imageArrayLayers = 1;
	createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT;

	uint32_t queueFamilyIndices[]{ (uint32_t)_info.graphicsFamily, (uint32_t)_info.presentFamily };

	if (_info.graphicsFamily != _info.presentFamily)
	{
		createInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
		createInfo.queueFamilyIndexCount = 2;
		createInfo.pQueueFamilyIndices = queueFamilyIndices;
	}
	else
	{
		createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
		createInfo.queueFamilyIndexCount = 0;
		createInfo.pQueueFamilyIndices = nullptr;
	}

	createInfo.preTransform = _info.capabilities.currentTransform;
	createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
	createInfo.presentMode = presentMode;
	createInfo.clipped = VK_TRUE;
	createInfo.oldSwapchain = _swapchain;

	VkSwapchainKHR newSwapchain{};
	if (vkCreateSwapchainKHR(_info.device, &createInfo, _info.allocator, &newSwapchain) != VK_SUCCESS)
	{
		Logger::Log(SWAPCHAIN_MODULE, LOG_CRITICAL, "Failed to create swapchain");
		return false;
	}

	*&_swapchain = newSwapchain;

	if (vkGetSwapchainImagesKHR(_info.device, _swapchain, &imageCount, nullptr) != VK_SUCCESS)
	{
		Logger::Log(SWAPCHAIN_MODULE, LOG_CRITICAL, "vkGetSwapchainImagesKHR call failed");
		return false;
	}

	_images.resize(imageCount);
	if (vkGetSwapchainImagesKHR(_info.device, _swapchain, &imageCount, _images.data()) != VK_SUCCESS)
	{
		Logger::Log(SWAPCHAIN_MODULE, LOG_CRITICAL, "vkGetSwapchainImagesKHR call failed");
		return false;
	}

	_imageFormat = surfaceFormat.format;

	return true;
}

uint32_t Swapchain::AcquireNextImage(VkSemaphore signalSemaphore)
{
	uint32_t imageIndex{};
	VkResult res{ vkAcquireNextImageKHR(_info.device, _swapchain, std::numeric_limits<uint64_t>::max(), signalSemaphore, VK_NULL_HANDLE, &imageIndex) };

	if (res == VK_ERROR_OUT_OF_DATE_KHR)
	{
		_Create();
		return UINT32_MAX;
	}
	else if (res != VK_SUCCESS && res != VK_SUBOPTIMAL_KHR)
	{ DIE("Failed to acquire image"); }

	return imageIndex;
}

uint32_t Swapchain::Present(VkSemaphore waitSemaphore, uint32_t imageIndex, VkQueue presentQueue)
{
	VkSemaphore signalSemaphores[]{ waitSemaphore };

	VkPresentInfoKHR presentInfo{};
	presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
	presentInfo.waitSemaphoreCount = 1;
	presentInfo.pWaitSemaphores = signalSemaphores;
	presentInfo.swapchainCount = 1;
	presentInfo.pSwapchains = &_swapchain;
	presentInfo.pImageIndices = &imageIndex;
	presentInfo.pResults = nullptr;

	VkResult res = vkQueuePresentKHR(presentQueue, &presentInfo);

	if (res == VK_ERROR_OUT_OF_DATE_KHR || res == VK_SUBOPTIMAL_KHR)
	{
		_Create();
		return UINT32_MAX;
	}
	else if (res != VK_SUCCESS)
	{ DIE("Failed to present image"); }

	return 0;
}

Swapchain::~Swapchain()
{
	vkDestroySwapchainKHR(_info.device, _swapchain, _info.allocator);
}