/* NekoEngine
 *
 * TGA.cpp
 * Author: Alexandru Naiman
 *
 * TARGA image loader
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

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdint.h>
#include <limits.h>

#include <System/AssetLoader/AssetLoader.h>

#pragma pack(push,x1)					// Byte alignment (8-bit)
#pragma pack(1)

using namespace std;

typedef struct TGA_HEADER
{
	uint8_t identSize;
	uint8_t colorMapType;
	uint8_t imageType;
	int16_t colorMapStart;
	int16_t colorMapLength;
	uint8_t colorMapBits;
	int16_t xStart;
	int16_t yStart;
	uint16_t width;
	uint16_t height;
	uint8_t bits;
	uint8_t descriptor;
} TGAHeader;

#pragma pack(pop,x1)

const int IT_COMPRESSED = 10;
const int IT_UNCOMPRESSED = 2;
const int IT_UNCOMPRESSED_BW = 3;

inline int16_t swap_int16(int16_t val)
{
	return (val << 8) | ((val >> 8) & 0xFF);
}

inline uint16_t swap_uint16(uint16_t val)
{
	return (val << 8) | (val >> 8);
}

static inline void _swapEndianness(TGAHeader *header)
{
	union
	{
		uint32_t i;
		char c[4];
	} bint{ 0x01020304 };

	if (bint.c[0] != 1) return;

	header->colorMapStart = swap_int16(header->colorMapStart);
	header->colorMapLength = swap_int16(header->colorMapLength);

	header->xStart = swap_int16(header->xStart);
	header->yStart = swap_int16(header->yStart);

	header->width = swap_uint16(header->width);
	header->height = swap_uint16(header->height);
}

static inline void _loadCompressedImage(uint8_t *dst, const uint8_t *src, TGAHeader *header)
{
	int w = header->width;
	int h = header->height;
	int rowSize = w * header->bits / 8;
	bool inverted = ((header->descriptor & (1 << 5)) != 0);
	uint8_t *dstPtr = inverted ? dst + (h + 1) * rowSize : dst;
	int count = 0;
	int pixels = w * h;

	while (pixels > count)
	{
		unsigned char chunk = *src++;
		if (chunk < 128)
		{
			int chunkSize = chunk + 1;
			for (int i = 0; i < chunkSize; i++)
			{
				if (inverted && (count % w) == 0)
					dstPtr -= 2 * rowSize;
				*dstPtr++ = src[2];
				*dstPtr++ = src[1];
				*dstPtr++ = src[0];
				src += 3;
				if (header->bits != 24)
					*dstPtr++ = *src++;
				else
					*dstPtr++ = 0;
				count++;
			}
		}
		else
		{
			int chunkSize = chunk - 127;
			for (int i = 0; i < chunkSize; i++)
			{
				if (inverted && (count % w) == 0)
					dstPtr -= 2 * rowSize;
				*dstPtr++ = src[2];
				*dstPtr++ = src[1];
				*dstPtr++ = src[0];
				if (header->bits != 24)
					*dstPtr++ = src[3];
				else
					*dstPtr++ = 0;
				count++;
			}
			src += (header->bits >> 3);
		}
	}
}

static inline void _loadUncompressedImage(uint8_t *dst, const uint8_t *src, TGAHeader *header)
{
	int w = header->width;
	int h = header->height;
	int rowSize = w * header->bits / 8;
	bool inverted = ((header->descriptor & (1 << 5)) != 0);
	for (int i = 0; i < h; i++)
	{
		const uint8_t *srcRow = src +
			(inverted ? (h - i - 1) * rowSize : i * rowSize);
		if (header->bits == 8)
		{
			for (int j = 0; j < w; ++j)
				*dst++ = *srcRow++;
		}
		else if (header->bits == 24)
		{
			for (int j = 0; j < w; ++j)
			{
				*dst++ = srcRow[2];
				*dst++ = srcRow[1];
				*dst++ = srcRow[0];
				*dst++ = 0;
				srcRow += 3;
			}
		}
		else
		{
			for (int j = 0; j < w; ++j)
			{
				*dst++ = srcRow[2];
				*dst++ = srcRow[1];
				*dst++ = srcRow[0];
				*dst++ = srcRow[3];
				srcRow += 4;
			}
		}
	}
}

int AssetLoader::LoadTGA(const uint8_t *data, uint64_t dataSize, uint32_t &width, uint32_t &height, uint8_t &bpp, uint8_t **imgData, uint64_t &imgDataSize)
{
	const uint8_t *ptr = data;
	TGAHeader header;
	memcpy(&header, ptr, sizeof(header));
	ptr += sizeof(header) + header.identSize;

	_swapEndianness(&header);

	if (header.imageType != IT_COMPRESSED && header.imageType != IT_UNCOMPRESSED && header.imageType != IT_UNCOMPRESSED_BW)
		return ENGINE_INVALID_HEADER;

	if (header.bits != 8 && header.bits != 24 && header.bits != 32)
		return ENGINE_INVALID_HEADER;

	uint64_t imgSize = (unsigned long)header.width * (unsigned long)header.height;
	if (imgSize < header.width || imgSize < header.height)
		return ENGINE_INVALID_HEADER;

	uint64_t size = imgSize * (header.bits == 8 ? 8 : 32);
	if (size < imgSize || size < header.bits)
		return ENGINE_INVALID_HEADER;

	imgDataSize = (size + 7) / 8;
	*imgData = (uint8_t *)calloc(sizeof(uint8_t), imgDataSize);

	if (header.imageType == IT_UNCOMPRESSED || header.imageType == IT_UNCOMPRESSED_BW)
		_loadUncompressedImage(*imgData, ptr, &header);
	else
		_loadCompressedImage(*imgData, ptr, &header);

	width = header.width;
	height = header.height;
	bpp = header.bits == 8 ? 8 : 32;

	return ENGINE_OK;
}