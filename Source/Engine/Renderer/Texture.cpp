/* NekoEngine
 *
 * Texture.cpp
 * Author: Alexandru Naiman
 *
 * Texture class implementation 
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

#include <string>
#include <string.h>

#include <Engine/Engine.h>
#include <Renderer/VKUtil.h>
#include <Renderer/Texture.h>
#include <Renderer/DebugMarker.h>
#include <System/AssetLoader/AssetLoader.h>
#include <System/VFS/VFS.h>

#define TEX_MODULE	"Texture"

using namespace std;

VkFilter _SamplerFilter[3] =
{
	VK_FILTER_NEAREST,
	VK_FILTER_LINEAR,
	VK_FILTER_CUBIC_IMG
};

VkSamplerAddressMode _AddressMode[5] =
{
	VK_SAMPLER_ADDRESS_MODE_REPEAT,
	VK_SAMPLER_ADDRESS_MODE_MIRRORED_REPEAT,
	VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE,
	VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER,
	VK_SAMPLER_ADDRESS_MODE_MIRRORED_REPEAT
};

VkSamplerMipmapMode _MipmapMode[2] =
{
	VK_SAMPLER_MIPMAP_MODE_NEAREST,
	VK_SAMPLER_MIPMAP_MODE_LINEAR
};

VkBorderColor _BorderColor[3] = 
{
	VK_BORDER_COLOR_FLOAT_OPAQUE_BLACK,
	VK_BORDER_COLOR_FLOAT_OPAQUE_WHITE,
	VK_BORDER_COLOR_FLOAT_TRANSPARENT_BLACK
};

Texture::Texture(TextureResource *res) noexcept
{
	_resourceInfo = res;
	_image = VK_NULL_HANDLE;
	_imageMemory = VK_NULL_HANDLE;
	_view = VK_NULL_HANDLE;
	_format = VK_FORMAT_UNDEFINED;
	_sampler = VK_NULL_HANDLE;
	_type = VK_IMAGE_TYPE_2D;
	_isAttachment = false;
	_width = _height = _depth = _mipLevels = 0;
	_arrayLayers = 1;
	_ownMemory = true;
}

Texture::Texture(uint32_t width, uint32_t height, uint32_t mipLevels, VkFormat format, uint64_t dataSize, uint8_t *data)
{
	_resourceInfo = nullptr;
	_image = VK_NULL_HANDLE;
	_view = VK_NULL_HANDLE;
	_sampler = VK_NULL_HANDLE;
	_imageMemory = VK_NULL_HANDLE;
	_format = format;
	_type = VK_IMAGE_TYPE_2D;
	_isAttachment = false;
	_width = width;
	_height = height;
	_depth = 1;
	_mipLevels = mipLevels;
	_arrayLayers = 1;
	_ownMemory = true;

	VKUtil::CreateImage(_image, _imageMemory, _width, _height, _depth, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
		_format, _type, VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT, VK_IMAGE_TILING_OPTIMAL, _mipLevels, 1, 0, VK_SAMPLE_COUNT_1_BIT);

	Buffer *stagingBuffer = Renderer::GetInstance()->GetStagingBuffer(dataSize);

	uint8_t *ptr = stagingBuffer->Map(0, dataSize);
	if (!ptr) { DIE("Failed to map buffer"); }
	memcpy(ptr, data, dataSize);
	stagingBuffer->Unmap();

	VKUtil::TransitionImageLayout(_image, VK_IMAGE_LAYOUT_PREINITIALIZED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
	VKUtil::CopyBufferToImage(stagingBuffer->GetHandle(), _image, _width, _height);
	VKUtil::TransitionImageLayout(_image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

	VKUtil::CreateImageView(_view, _image, VK_IMAGE_VIEW_TYPE_2D, _format, VK_IMAGE_ASPECT_COLOR_BIT,
		0, _mipLevels);
	VKUtil::CreateSampler(_sampler, VK_FILTER_LINEAR, VK_FILTER_LINEAR,
		VK_SAMPLER_ADDRESS_MODE_REPEAT, VK_SAMPLER_ADDRESS_MODE_REPEAT, VK_SAMPLER_ADDRESS_MODE_REPEAT, 16.f,
		VK_SAMPLER_MIPMAP_MODE_LINEAR, 0.f, 0.f, (float)_mipLevels);

	Renderer::GetInstance()->FreeStagingBuffer(stagingBuffer);
}

Texture::Texture(VkFormat format, VkImageType type, VkImageUsageFlags usage, VkImageTiling tiling, uint32_t width, uint32_t height, uint32_t depth, bool attachment, VkDeviceMemory memory, VkSampleCountFlagBits samples, uint32_t mipLevels, uint32_t arrayLayers, bool create)
{
	_resourceInfo = nullptr;
	_image = VK_NULL_HANDLE;
	_view = VK_NULL_HANDLE;
	_sampler = VK_NULL_HANDLE;
	_imageMemory = memory;
	_format = format;
	_type = type;
	_isAttachment = attachment;
	_width = width;
	_height = height;
	_depth = depth;
	_mipLevels = mipLevels;
	_arrayLayers = arrayLayers;
	_ownMemory = _imageMemory == VK_NULL_HANDLE;

	if (!create)
		return;

	if (!VKUtil::CreateImage(_image, _imageMemory, _width, _height, _depth, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
		_format, _type, usage, tiling, _mipLevels, _arrayLayers, 0, samples))
	{ DIE("Out of resources"); }
}

int Texture::Load()
{
	bool tga{ false };
	NString path(GetResourceInfo()->filePath);
	path.Append(".dds");
	uint8_t *mem{ nullptr }, *imgData{ nullptr };
	VkDeviceSize size{ 0 }, imgDataSize{ 0 };
	VFSFile *file{ VFS::Open(path) };
	Buffer *stagingBuffer{ nullptr };
	VkImageViewType imageViewType{};
	uint32_t fileMipLevels{ 0 };

	if (!file)
	{
		path[path.Length() - 3] = 't';
		path[path.Length() - 2] = 'g';
		path[path.Length() - 1] = 'a';

		file = VFS::Open(path);

		if (!file)
		{
			Logger::Log(TEX_MODULE, LOG_CRITICAL, "Failed to load texture id %d, file name [%s]. Reason: unsupported texture format.", GetResourceInfo()->id, *GetResourceInfo()->filePath);
			return ENGINE_FAIL;
		}

		tga = true;
	}

	if (file->Seek(0, SEEK_END) == ENGINE_FAIL)
	{
		file->Close();
		Logger::Log(TEX_MODULE, LOG_CRITICAL, "Seek failed for file [%].", *GetResourceInfo()->filePath);
		return ENGINE_FAIL;
	}
	size = file->Tell();
	if (file->Seek(0, SEEK_SET) == ENGINE_FAIL)
	{
		file->Close();
		Logger::Log(TEX_MODULE, LOG_CRITICAL, "Seek failed for file [%].", *GetResourceInfo()->filePath);
		return ENGINE_FAIL;
	}

	mem = (uint8_t*)calloc((size_t)size, sizeof(uint8_t));
	if (file->Read(mem, sizeof(uint8_t), size) == 0)
	{
		file->Close();
		free(mem);
		Logger::Log(TEX_MODULE, LOG_CRITICAL, "Failed to read file [%].", *GetResourceInfo()->filePath);
		return ENGINE_FAIL;
	}

	if (tga)
	{
		uint8_t bpp;
		if (AssetLoader::LoadTGA(mem, size, _width, _height, bpp, &imgData, imgDataSize) != ENGINE_OK)
		{
			Logger::Log(TEX_MODULE, LOG_CRITICAL, "Failed to load TGA file for texture %s", GetResourceInfo()->name.c_str());
			return ENGINE_FAIL;
		}

		bpp /= 8;

		if (bpp == 3)
			_format = VK_FORMAT_R8G8B8_UNORM;
		else
			_format = VK_FORMAT_R8G8B8A8_UNORM;

		fileMipLevels = 1;

		if (GetResourceInfo()->textureType == TextureResourceType::TEXTURE_CUBEMAP)
		{
			uint32_t size{ _width / 4 };
			uint32_t rowSize{ bpp * size };
			uint32_t imageSize{ size * size * bpp };

			uint32_t centerRowOffset{ 4 * imageSize };
			uint32_t bottomRowOffset{ 8 * imageSize };

			uint8_t *cubemap{ (uint8_t *)calloc(imageSize, 6) };
			if (!cubemap)
				return false;

			for (uint32_t i = 0; i < size; i++)
			{
				uint32_t dstOffset{ rowSize * i };
				uint32_t rowOffset{ (4 * rowSize * i) };

				memcpy(cubemap + dstOffset, imgData + centerRowOffset + rowOffset + (rowSize * 2), rowSize);
				memcpy((cubemap + imageSize) + dstOffset, imgData + centerRowOffset + rowOffset, rowSize);
				memcpy((cubemap + imageSize * 2) + dstOffset, imgData + rowOffset + rowSize, rowSize);
				memcpy((cubemap + imageSize * 3) + dstOffset, imgData + bottomRowOffset + rowOffset + rowSize, rowSize);
				memcpy((cubemap + imageSize * 4) + dstOffset, imgData + centerRowOffset + rowOffset + rowSize, rowSize);
				memcpy((cubemap + imageSize * 5) + dstOffset, imgData + centerRowOffset + rowOffset + (rowSize * 3), rowSize);
			}

			free(imgData);
			imgDataSize = imageSize * 6;
			imgData = cubemap;
			_width = _height = size;
		}
	}
	else
	{
		uint32_t fmt{};
		if (AssetLoader::LoadDDS(mem, size, _width, _height, _depth, fmt, fileMipLevels, &imgData, imgDataSize) != ENGINE_OK)
		{
			Logger::Log(TEX_MODULE, LOG_CRITICAL, "Failed to load DDS file for texture %s", GetResourceInfo()->name.c_str());
			return ENGINE_FAIL;
		}
		_format = (VkFormat)fmt;
	}

	_mipLevels = fileMipLevels;

	if ((_mipLevels == 1) && (GetResourceInfo()->textureType != TextureResourceType::TEXTURE_CUBEMAP))
		_mipLevels = (int)floor(std::log2(std::max(_width, _height))) + 1;

	if (!VKUtil::CreateImage(_image, _imageMemory, _width, _height, 1,
		VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, _format, VK_IMAGE_TYPE_2D,
		VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT, VK_IMAGE_TILING_OPTIMAL, _mipLevels,
		GetResourceInfo()->textureType == TextureResourceType::TEXTURE_CUBEMAP ? 6 : 1,
		GetResourceInfo()->textureType == TextureResourceType::TEXTURE_CUBEMAP ? VK_IMAGE_CREATE_CUBE_COMPATIBLE_BIT : 0))
	{
		free(mem); if (tga) free(imgData);
		return ENGINE_OUT_OF_RESOURCES;
	}

	if ((stagingBuffer = new Buffer(imgDataSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, nullptr, VK_MEMORY_PROPERTY_HOST_COHERENT_BIT | VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT)) == nullptr)
	{
		free(mem); if (tga) free(imgData);;
		return ENGINE_OUT_OF_RESOURCES;
	}

	uint8_t *ptr{ stagingBuffer->Map() };
	if (!ptr)
	{
		free(mem); if (tga) free(imgData);
		return ENGINE_OUT_OF_RESOURCES;
	}
	memcpy(ptr, imgData, imgDataSize);
	stagingBuffer->Unmap();

	free(mem); if (tga) free(imgData);

	VkCommandBuffer uploadCmdBuffer{ VKUtil::CreateOneShotCmdBuffer() };

	VkImageSubresourceRange range{};
	range.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	range.baseMipLevel = 0;
	range.baseArrayLayer = 0;
	range.levelCount = fileMipLevels;
	range.layerCount = GetResourceInfo()->textureType == TextureResourceType::TEXTURE_CUBEMAP ? 6 : 1;

	VKUtil::TransitionImageLayout(_image, VK_IMAGE_LAYOUT_PREINITIALIZED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, range, uploadCmdBuffer);

	VkDeviceSize buffOffset{ 0 };

	for (uint32_t i = 0; i < range.layerCount; ++i)
	{
		for (uint32_t j = 0; j < fileMipLevels; ++j)
		{
			uint32_t width{ j ? _width >> j : _width };
			uint32_t height{ j ? _height >> j : _height };

			VkDeviceSize size{ _GetByteSize(width, height) };

			VkImageSubresourceLayers subResource{};
			subResource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
			subResource.baseArrayLayer = i;
			subResource.mipLevel = j;
			subResource.layerCount = 1;

			VKUtil::CopyBufferToImage(stagingBuffer->GetHandle(), _image, width, height, buffOffset, subResource, uploadCmdBuffer);
			buffOffset += size;
		}
	}

	VKUtil::TransitionImageLayout(_image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, range, uploadCmdBuffer);

	VKUtil::ExecuteOneShotCmdBuffer(uploadCmdBuffer);

	delete stagingBuffer;

	if (fileMipLevels == 1)
		GenerateMipmaps();
	
	if (GetResourceInfo()->textureType == TextureResourceType::TEXTURE_CUBEMAP)
		imageViewType = VK_IMAGE_VIEW_TYPE_CUBE;
	else
		imageViewType = VK_IMAGE_VIEW_TYPE_2D;

	uint32_t baseMip{ 2u - Engine::GetConfiguration().Renderer.TextureQuality };

	if (baseMip > _mipLevels - 1)
		baseMip = _mipLevels - 1;

	if (!VKUtil::CreateImageView(_view, _image, imageViewType, _format, VK_IMAGE_ASPECT_COLOR_BIT, baseMip,
		_mipLevels, 0, GetResourceInfo()->textureType == TextureResourceType::TEXTURE_CUBEMAP ? 6 : 1))
		return ENGINE_OUT_OF_RESOURCES;

	VKUtil::CreateSampler(_sampler, VK_FILTER_LINEAR, VK_FILTER_LINEAR,
		VK_SAMPLER_ADDRESS_MODE_REPEAT, VK_SAMPLER_ADDRESS_MODE_REPEAT, VK_SAMPLER_ADDRESS_MODE_REPEAT, 16.f,
		VK_SAMPLER_MIPMAP_MODE_LINEAR, 0.f, 0.f, (float)_mipLevels);

	VK_DBG_SET_OBJECT_NAME((uint64_t)_image, VK_DEBUG_REPORT_OBJECT_TYPE_IMAGE_EXT, _resourceInfo->name.c_str());

	Logger::Log(TEX_MODULE, LOG_DEBUG, "Loaded texture id %d from %s, size %dx%d", _resourceInfo->id, *path, _width, _height);

	return ENGINE_OK;
}

void Texture::SetParameters(SamplerParams &params, float aniso) noexcept
{
	if (_sampler != VK_NULL_HANDLE)
		vkDestroySampler(VKUtil::GetDevice(), _sampler, VKUtil::GetAllocator());

	if (Engine::GetConfiguration().Renderer.Anisotropic)
	{
		if(aniso < 0.f)
			aniso = (float)Engine::GetConfiguration().Renderer.Aniso;
	}

	VKUtil::CreateSampler(_sampler, _SamplerFilter[params.minFilter], _SamplerFilter[params.magFilter],
		_AddressMode[params.addressU], _AddressMode[params.addressV], _AddressMode[params.addressW], aniso,
		_MipmapMode[params.mipmapMode], params.minLodBias, 0.f, (float)_mipLevels, _BorderColor[params.borderColor]);
}

void Texture::GenerateMipmaps()
{
	VkCommandBuffer mipmapCmdBuffer{ VKUtil::CreateOneShotCmdBuffer() };

	VKUtil::TransitionImageLayout(_image, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, VK_IMAGE_ASPECT_COLOR_BIT, mipmapCmdBuffer);

	for (uint32_t i = 1; i < _mipLevels; ++i)
	{
		VkImageBlit blit{};
		blit.srcSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		blit.srcSubresource.layerCount = 1;
		blit.srcSubresource.mipLevel = i - 1;
		blit.srcOffsets[1].x = int32_t(_width >> (i - 1));
		blit.srcOffsets[1].y = int32_t(_height >> (i - 1));
		blit.srcOffsets[1].z = 1;

		blit.dstSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		blit.dstSubresource.layerCount = 1;
		blit.dstSubresource.mipLevel = i;
		blit.dstOffsets[1].x = int32_t(_width >> i);
		blit.dstOffsets[1].y = int32_t(_height >> i);
		blit.dstOffsets[1].z = 1;

		VkImageSubresourceRange range{};
		range.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		range.baseMipLevel = i;
		range.levelCount = 1;
		range.layerCount = 1;

		VKUtil::TransitionImageLayout(_image, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, range, mipmapCmdBuffer);
		vkCmdBlitImage(mipmapCmdBuffer, _image, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, _image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &blit, VK_FILTER_LINEAR);
		VKUtil::TransitionImageLayout(_image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, range, mipmapCmdBuffer);
	}

	VkImageSubresourceRange range{};
	range.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	range.baseMipLevel = 0;
	range.levelCount = _mipLevels;
	range.layerCount = 1;

	VKUtil::TransitionImageLayout(_image, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, range, mipmapCmdBuffer);

	VKUtil::ExecuteOneShotCmdBuffer(mipmapCmdBuffer);
}

bool Texture::CreateView(VkImageAspectFlags aspect, bool forceArray)
{
	if (_view != VK_NULL_HANDLE)
		vkDestroyImageView(VKUtil::GetDevice(), _view, VKUtil::GetAllocator());

	VkImageViewType type{ VK_IMAGE_VIEW_TYPE_2D };

	if (forceArray || _arrayLayers > 1)
		type = VK_IMAGE_VIEW_TYPE_2D_ARRAY;

	if (!VKUtil::CreateImageView(_view, _image, type, _format, aspect, 0, _mipLevels, 0, 1))
		return false;

	return true;
}

VkDeviceSize Texture::_GetByteSize(uint32_t width, uint32_t height)
{
	switch (_format)
	{
		case VK_FORMAT_R8_UNORM:
			return width * height * 1;
		case VK_FORMAT_R8G8_UNORM:
			return width * height * 2;
		case VK_FORMAT_B8G8R8_UNORM:
		case VK_FORMAT_R8G8B8_UNORM:
			return width * height * 3;
		case VK_FORMAT_B8G8R8A8_UNORM:
		case VK_FORMAT_R8G8B8A8_UNORM:
			return width * height * 4;
		case VK_FORMAT_BC1_RGBA_UNORM_BLOCK:
		case VK_FORMAT_BC1_RGBA_SRGB_BLOCK:
		case VK_FORMAT_BC4_UNORM_BLOCK:
		case VK_FORMAT_BC4_SNORM_BLOCK:
			return ((width + 3) / 4) * ((height + 3) / 4) * 8;
		case VK_FORMAT_BC2_UNORM_BLOCK:
		case VK_FORMAT_BC2_SRGB_BLOCK:
		case VK_FORMAT_BC3_UNORM_BLOCK:
		case VK_FORMAT_BC3_SRGB_BLOCK:
		case VK_FORMAT_BC5_UNORM_BLOCK:
		case VK_FORMAT_BC5_SNORM_BLOCK:
		case VK_FORMAT_BC6H_UFLOAT_BLOCK:
		case VK_FORMAT_BC6H_SFLOAT_BLOCK:
		case VK_FORMAT_BC7_UNORM_BLOCK:
		case VK_FORMAT_BC7_SRGB_BLOCK:
			return ((width + 3) / 4) * ((height + 3) / 4) * 16;
		default:
			return 0;
	}

	return 0;
}

Texture *Texture::CreateRenderTarget(VkSampleCountFlagBits samples)
{
	VkFormat fmt{ VK_FORMAT_R16G16B16A16_SFLOAT };
	Texture *t{ new Texture(fmt, VK_IMAGE_TYPE_2D, 1, VK_IMAGE_TILING_OPTIMAL, 1920, 1080, 1, true, VK_NULL_HANDLE, samples) };
	return t;
}

Texture *Texture::CreateDepthStencilTarget()
{
	return nullptr;
}

Texture::~Texture() noexcept
{
	if (_ownMemory)
		vkFreeMemory(VKUtil::GetDevice(), _imageMemory, VKUtil::GetAllocator());

	if (_image != VK_NULL_HANDLE)
		vkDestroyImage(VKUtil::GetDevice(), _image, VKUtil::GetAllocator());

	if (_view != VK_NULL_HANDLE)
		vkDestroyImageView(VKUtil::GetDevice(), _view, VKUtil::GetAllocator());

	if (_sampler != VK_NULL_HANDLE)
		vkDestroySampler(VKUtil::GetDevice(), _sampler, VKUtil::GetAllocator());
}
