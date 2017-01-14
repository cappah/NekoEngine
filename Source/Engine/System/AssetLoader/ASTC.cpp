/* NekoEngine
 *
 * ASTC.cpp
 * Author: Alexandru Naiman
 *
 * ASTC loader
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

//*************
//* NOT TESTED
//*************

#include <vulkan/vulkan.h>

#include <System/AssetLoader/AssetLoader.h>

#define MIN_ASTC_BLOCK_SIZE	4
#define MAX_ASTC_BLOCK_SIZE	12

typedef struct ASTC_HEADER
{
	unsigned char magic[4];
	unsigned char blockdim_x;
	unsigned char blockdim_y;
	unsigned char blockdim_z;
	unsigned char xsize[3];
	unsigned char ysize[3];
	unsigned char zsize[3];
} ASTCHeader;

static const unsigned char _ASTCMagic[4] = { 0x13, 0xAB, 0xA1, 0x5C };

VkFormat LinearBlockSizeToPixelFormat[][(MAX_ASTC_BLOCK_SIZE - MIN_ASTC_BLOCK_SIZE) + 1] =
{
	{ VK_FORMAT_ASTC_4x4_UNORM_BLOCK, VK_FORMAT_ASTC_5x4_UNORM_BLOCK, VK_FORMAT_UNDEFINED, VK_FORMAT_UNDEFINED, VK_FORMAT_UNDEFINED, VK_FORMAT_UNDEFINED, VK_FORMAT_UNDEFINED, VK_FORMAT_UNDEFINED, VK_FORMAT_UNDEFINED },
	{ VK_FORMAT_UNDEFINED, VK_FORMAT_ASTC_5x5_UNORM_BLOCK, VK_FORMAT_ASTC_6x5_UNORM_BLOCK, VK_FORMAT_UNDEFINED, VK_FORMAT_ASTC_8x5_UNORM_BLOCK, VK_FORMAT_UNDEFINED, VK_FORMAT_ASTC_10x5_UNORM_BLOCK, VK_FORMAT_UNDEFINED, VK_FORMAT_UNDEFINED },
	{ VK_FORMAT_UNDEFINED, VK_FORMAT_UNDEFINED, VK_FORMAT_ASTC_6x6_UNORM_BLOCK, VK_FORMAT_UNDEFINED, VK_FORMAT_ASTC_8x6_UNORM_BLOCK, VK_FORMAT_UNDEFINED, VK_FORMAT_ASTC_10x6_UNORM_BLOCK, VK_FORMAT_UNDEFINED, VK_FORMAT_UNDEFINED },
	{ VK_FORMAT_UNDEFINED, VK_FORMAT_UNDEFINED, VK_FORMAT_UNDEFINED, VK_FORMAT_UNDEFINED, VK_FORMAT_UNDEFINED, VK_FORMAT_UNDEFINED, VK_FORMAT_UNDEFINED, VK_FORMAT_UNDEFINED, VK_FORMAT_UNDEFINED },
	{ VK_FORMAT_UNDEFINED, VK_FORMAT_UNDEFINED, VK_FORMAT_UNDEFINED, VK_FORMAT_UNDEFINED, VK_FORMAT_ASTC_8x8_UNORM_BLOCK, VK_FORMAT_UNDEFINED, VK_FORMAT_ASTC_10x8_UNORM_BLOCK, VK_FORMAT_UNDEFINED, VK_FORMAT_UNDEFINED },
	{ VK_FORMAT_UNDEFINED, VK_FORMAT_UNDEFINED, VK_FORMAT_UNDEFINED, VK_FORMAT_UNDEFINED, VK_FORMAT_UNDEFINED, VK_FORMAT_UNDEFINED, VK_FORMAT_UNDEFINED, VK_FORMAT_UNDEFINED, VK_FORMAT_UNDEFINED },
	{ VK_FORMAT_UNDEFINED, VK_FORMAT_UNDEFINED, VK_FORMAT_UNDEFINED, VK_FORMAT_UNDEFINED, VK_FORMAT_UNDEFINED, VK_FORMAT_UNDEFINED, VK_FORMAT_ASTC_10x10_UNORM_BLOCK, VK_FORMAT_UNDEFINED, VK_FORMAT_ASTC_12x10_UNORM_BLOCK },
	{ VK_FORMAT_UNDEFINED, VK_FORMAT_UNDEFINED, VK_FORMAT_UNDEFINED, VK_FORMAT_UNDEFINED, VK_FORMAT_UNDEFINED, VK_FORMAT_UNDEFINED, VK_FORMAT_UNDEFINED, VK_FORMAT_UNDEFINED, VK_FORMAT_UNDEFINED },
	{ VK_FORMAT_UNDEFINED, VK_FORMAT_UNDEFINED, VK_FORMAT_UNDEFINED, VK_FORMAT_UNDEFINED, VK_FORMAT_UNDEFINED, VK_FORMAT_UNDEFINED, VK_FORMAT_UNDEFINED, VK_FORMAT_UNDEFINED, VK_FORMAT_ASTC_12x12_UNORM_BLOCK }
};

static inline VkFormat _ASTC_GetPixelFormat(ASTCHeader *header)
{
	if ((header->blockdim_x < MIN_ASTC_BLOCK_SIZE) || (header->blockdim_x > MAX_ASTC_BLOCK_SIZE) ||
		(header->blockdim_y < MIN_ASTC_BLOCK_SIZE) || (header->blockdim_y > MAX_ASTC_BLOCK_SIZE))
		return VK_FORMAT_UNDEFINED;

	return LinearBlockSizeToPixelFormat[header->blockdim_y - MIN_ASTC_BLOCK_SIZE][header->blockdim_x - MIN_ASTC_BLOCK_SIZE];
}

static inline bool _ASTC_VerifyHeader(ASTCHeader *hdr, uint32_t &width, uint32_t &height, uint32_t &depth)
{
	if (memcmp(hdr->magic, _ASTCMagic, sizeof(_ASTCMagic)))
		return false;

	width = hdr->xsize[0] | (hdr->xsize[1] << 8) | (hdr->xsize[2] << 16);
	height = hdr->ysize[0] | (hdr->ysize[1] << 8) | (hdr->ysize[2] << 16);
	depth = hdr->zsize[0] | (hdr->zsize[1] << 8) | (hdr->zsize[2] << 16);

	if (depth != 1)
	{
		// TODO: 3D texture support
		return false;
	}

	return true;
}

int AssetLoader::LoadASTC(const uint8_t *data, uint64_t dataSize, uint32_t &width, uint32_t &height, uint32_t &depth, uint32_t &format, uint32_t &mipLevels, uint8_t **imgData, uint64_t &imgDataSize)
{
	ASTCHeader hdr{};
	size_t offset{ 0 };
	imgDataSize = (uint32_t)(dataSize - sizeof(ASTCHeader));

	memcpy(&hdr, data + offset, sizeof(ASTCHeader));
	offset += sizeof(ASTCHeader);

	if (!_ASTC_VerifyHeader(&hdr, width, height, depth))
		return ENGINE_INVALID_HEADER;

	format = _ASTC_GetPixelFormat(&hdr);
	if (format == VK_FORMAT_UNDEFINED)
		return ENGINE_INVALID_HEADER;

	*imgData = (uint8_t *)data + offset;
	mipLevels = 1;

	return true;
}