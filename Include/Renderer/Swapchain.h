/* NekoEngine
 *
 * Swapchain.h
 * Author: Alexandru Naiman
 *
 * Vulkan swapchain
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

#pragma once

#include <vulkan/vulkan.h>
#include <vector>

typedef struct SWAPCHAIN_INFO
{
	VkSurfaceCapabilitiesKHR capabilities;
	std::vector<VkSurfaceFormatKHR> formats;
	std::vector<VkPresentModeKHR> presentModes;
	VkSurfaceKHR surface;
	VkAllocationCallbacks *allocator;
	VkDevice device;
	uint32_t graphicsFamily;
	uint32_t presentFamily;
} SwapchainInfo;

class Swapchain
{
public:
	Swapchain(SwapchainInfo &info);

	VkImage GetImage(uint32_t index) { return _images[index]; }
	VkFormat GetImageFormat() { return _imageFormat; }
	size_t GetImageCount() { return _images.size(); }

	bool Resize() { return _Create(); }

	uint32_t AcquireNextImage(VkSemaphore signalSemaphore);
	uint32_t Present(VkSemaphore waitSemaphore, uint32_t imageIndex, VkQueue presentQueue);

	virtual ~Swapchain();

private:
	VkSwapchainKHR _swapchain;
	VkExtent2D _extent;
	VkFormat _imageFormat;
	SwapchainInfo _info;
	std::vector<VkImage> _images;

	bool _Create();
};
