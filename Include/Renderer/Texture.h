/* NekoEngine
 *
 * Texture.h
 * Author: Alexandru Naiman
 *
 * Texture class definition 
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

#include <Resource/Resource.h>
#include <Resource/TextureResource.h>

enum SamplerFilter : uint8_t
{
	Nearest = 0,
	Linear = 1,
	Cubic = 2
};

enum SamplerAddressMode : uint8_t
{
	Repeat = 0,
	MirroredRepeat = 1,
	ClampToEdge = 2,
	ClampToBorder = 3,
	MirrorClampToEdge = 4
};

enum SamplerMipmapMode : uint8_t
{
	MipmapNearest = 0,
	MipmapLinear = 1
};

enum SamplerBorderColor : uint8_t
{
	OpaqueBlack = 0,
	OpaqueWhite = 1,
	TransparentBlack = 2
};

struct SamplerParams
{
	SamplerFilter minFilter;
	SamplerFilter magFilter;
	SamplerAddressMode addressU;
	SamplerAddressMode addressV;
	SamplerAddressMode addressW;
	SamplerMipmapMode mipmapMode;
	SamplerBorderColor borderColor;
	float minLodBias;
};

class ENGINE_API Texture : public Resource
{
public:
	Texture(TextureResource* res) noexcept;
	Texture(uint32_t width, uint32_t height, uint32_t mipLevels, VkFormat format, uint64_t dataSize, uint8_t *data);
	Texture(VkFormat format, VkImageType type, VkImageUsageFlags usage, VkImageTiling tiling, uint32_t width, uint32_t height, uint32_t depth, bool attachment, VkDeviceMemory memory = VK_NULL_HANDLE, VkSampleCountFlagBits samples = VK_SAMPLE_COUNT_1_BIT, uint32_t mipLevels = 1, uint32_t arrayLayers = 1, bool create = true);

	TextureResource* GetResourceInfo() noexcept { return (TextureResource*)_resourceInfo; }
	int GetResourceId() noexcept { return _resourceInfo->id; }
	virtual int Load() override;
	void SetParameters(SamplerParams &params, float aniso = -1.f) noexcept;
	void GenerateMipmaps();

	virtual ~Texture() noexcept;

	VkImage GetImage() { return _image; }
	VkImageView GetImageView() { return _view; }
	VkDeviceMemory GetMemory() { return _imageMemory; }
	VkSampler GetSampler() { return _sampler; }
	VkFormat GetFormat() { return _format; }
	uint32_t GetWidth() { return _width; }
	uint32_t GetHeight() { return _height; }

	bool CreateView(VkImageAspectFlags aspect, bool forceArray = false);

	static Texture *CreateRenderTarget(VkSampleCountFlagBits samples = VK_SAMPLE_COUNT_1_BIT);
	static Texture *CreateDepthStencilTarget();

private:
	VkImage _image;
	VkDeviceMemory _imageMemory;
	VkImageView _view;
	VkFormat _format;
	VkSampler _sampler;
	VkImageType _type;
	bool _isAttachment, _ownMemory;
	uint32_t _width, _height, _depth, _mipLevels, _arrayLayers;

	VkDeviceSize _GetByteSize(uint32_t width, uint32_t height);
};

#if defined(_MSC_VER)
template class ENGINE_API NArray<Texture *>;
template class ENGINE_API NArray<SamplerParams>;
#endif