/* Neko Engine
 *
 * tga.cpp
 * Author: Alexandru Naiman
 *
 * TARGA image loader
 *
 * ----------------------------------------------------------------------------------
 *
 * Copyright (c) 2015-2016, Alexandru Naiman <alexandru dot naiman at icloud dot com>
 *
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without modification,
 * are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice, this list
 * of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above copyright notice, this
 * list of conditions and the following disclaimer in the documentation and/or other
 * materials provided with the distribution.
 *
 * 3. Neither the name of the copyright holder nor the names of its contributors may be
 * used to endorse or promote products derived from this software without specific prior
 * written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY ALEXANDRU NAIMAN "AS IS" AND ANY EXPRESS OR IMPLIED
 * WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY
 * AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL ALEXANDRU NAIMAN
 * BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE
 * OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */

#define ENGINE_INTERNAL

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <limits.h>

#include "tga.h"

#pragma pack(push,x1)					// Byte alignment (8-bit)
#pragma pack(1)

using namespace std;

typedef struct
{
    unsigned char  identsize;			// size of ID field that follows 18 byte header (0 usually)
    unsigned char  colourmaptype;		// type of colour map 0=none, 1=has palette
    unsigned char  imagetype;			// type of image 2=rgb uncompressed, 10 - rgb rle compressed

    short colourmapstart;				// first colour map entry in palette
    short colourmaplength;				// number of colours in palette
    unsigned char  colourmapbits;		// number of bits per palette entry 15,16,24,32

    short xstart;						// image x origin
    short ystart;						// image y origin
    unsigned short width;						// image width in pixels
    unsigned short height;						// image height in pixels
    unsigned char  bits;				// image bits per pixel 24,32
    unsigned char  descriptor;			// image descriptor bits (vh flip bits)

    // pixel data follows header
} TGA_HEADER;

#pragma pack(pop,x1)

const int IT_COMPRESSED = 10;
const int IT_UNCOMPRESSED = 2;

void LoadCompressedImage(char *pDest, char *pSrc, TGA_HEADER *pHeader)
{
    int w = pHeader->width;
    int h = pHeader->height;
    int rowSize = w * pHeader->bits / 8;
    bool bInverted = ((pHeader->descriptor & (1 << 5)) != 0);
    char * pDestPtr = bInverted ? pDest + (h + 1) * rowSize : pDest;
    int countPixels = 0;
    int nPixels = w * h;

    while(nPixels > countPixels)
    {
        unsigned char chunk = *pSrc++;
        if (chunk < 128)
        {
            int chunkSize = chunk + 1;
            for(int i = 0; i < chunkSize; i++)
            {
                if(bInverted && (countPixels % w) == 0)
                    pDestPtr -= 2 * rowSize;
                *pDestPtr ++ = pSrc[2];
                *pDestPtr ++ = pSrc[1];
                *pDestPtr ++ = pSrc[0];
                pSrc += 3;
                if(pHeader->bits != 24)
                    *pDestPtr ++ = *pSrc++;
                countPixels ++;
            }
        }
        else
        {
            int chunkSize = chunk - 127;
            for(int i = 0; i < chunkSize; i++)
            {
                if(bInverted && (countPixels % w) == 0)
                    pDestPtr -= 2 * rowSize;
                *pDestPtr ++ = pSrc[2];
                *pDestPtr ++ = pSrc[1];
                *pDestPtr ++ = pSrc[0];
                if(pHeader->bits != 24)
                    *pDestPtr ++ = pSrc[3];
                countPixels ++;
            }
            pSrc += (pHeader->bits >> 3);
        }
    }
}

void LoadUncompressedImage(char *pDest, char *pSrc, TGA_HEADER *pHeader)
{
    int w = pHeader->width;
    int h = pHeader->height;
    int rowSize = w * pHeader->bits / 8;
    bool bInverted = ((pHeader->descriptor & (1 << 5)) != 0);
    for(int i = 0; i < h; i++)
    {
        char *pSrcRow = pSrc + 
            (bInverted ? (h - i - 1) * rowSize : i * rowSize);
        if(pHeader->bits == 24)
        {
            for(int j = 0; j < w; j++)
            {
                *pDest ++ = pSrcRow[2];
                *pDest ++ = pSrcRow[1];
                *pDest ++ = pSrcRow[0];
                pSrcRow += 3;
            }
        }
        else
        {
            for(int j = 0; j < w; j++)
            {
                *pDest ++ = pSrcRow[2];
                *pDest ++ = pSrcRow[1];
                *pDest ++ = pSrcRow[0];
                *pDest ++ = pSrcRow[3];
                pSrcRow += 4;
            }
        }
    }
}

char *_loadTGA(string path, int *width, int *height, int *bpp)
{
	FILE *fp = fopen(path.c_str(), "rb");
	if (!fp)
		return NULL;

	TGA_HEADER header;
	if(fread(&header, sizeof(header), 1, fp) != 1)
		return NULL;

	fseek(fp, 0, SEEK_END);
	size_t fileLen = ftell(fp);
	fseek(fp, sizeof(header) + header.identsize, SEEK_SET);

	if (header.imagetype != IT_COMPRESSED && header.imagetype != IT_UNCOMPRESSED)
	{
		fclose(fp);
		return NULL;
	}

	if (header.bits != 24 && header.bits != 32)
	{
		fclose(fp);
		return NULL;
	}

	size_t bufferSize = fileLen - sizeof(header) - header.identsize;
	char *pBuffer = (char *)calloc(sizeof(char), bufferSize);

	if (!pBuffer)
	{
		fclose(fp);
		return NULL;
	}

	if(fread(pBuffer, 1, bufferSize, fp) != bufferSize)
	{
		fclose(fp);
		free(pBuffer);
		return NULL;
	}
	fclose(fp);

	*width = header.width;
	*height = header.height;
	*bpp = header.bits;

	unsigned long imgSize = header.width * header.height;

	if (imgSize < header.width || imgSize < header.height)
	{
		free(pBuffer);
		return NULL;
	}

	unsigned long size = imgSize * header.bits;

	if(size < imgSize || size < header.bits)
	{
		free(pBuffer);
		return NULL;
	}

	char *pOutBuffer = (char *)calloc(sizeof(char), size / 8);

	switch(header.imagetype)
	{
		case IT_UNCOMPRESSED:
			LoadUncompressedImage(pOutBuffer, pBuffer, &header);
		break;
		case IT_COMPRESSED:
			LoadCompressedImage(pOutBuffer, pBuffer, &header);
		break;
	}

	free(pBuffer);
	return pOutBuffer;
}

char* _loadTGAFromMemory(const uint8_t *mem, size_t memSize, int* width, int* height, int* bpp)
{
	const uint8_t *ptr = mem;
	TGA_HEADER header;
	memcpy(&header, ptr, sizeof(header));
	ptr += sizeof(header) + header.identsize;

	if (header.imagetype != IT_COMPRESSED && header.imagetype != IT_UNCOMPRESSED)
		return nullptr;

	if (header.bits != 24 && header.bits != 32)
		return nullptr;

	size_t bufferSize = memSize - sizeof(header) - header.identsize;
	char *pBuffer = (char *)calloc(sizeof(char), bufferSize);
	if (!pBuffer)
		return nullptr;

	memcpy(pBuffer, ptr, bufferSize);

	*width = header.width;
	*height = header.height;
	*bpp = header.bits;

	unsigned long imgSize = header.width * header.height;

	if (imgSize < header.width || imgSize < header.height)
	{
		free(pBuffer);
		return nullptr;
	}

	unsigned long size = imgSize * header.bits;

	if (size < imgSize || size < header.bits)
	{
		free(pBuffer);
		return nullptr;
	}

	char *pOutBuffer = (char *)calloc(sizeof(char), size / 8);

	switch (header.imagetype)
	{
	case IT_UNCOMPRESSED:
		LoadUncompressedImage(pOutBuffer, pBuffer, &header);
		break;
	case IT_COMPRESSED:
		LoadCompressedImage(pOutBuffer, pBuffer, &header);
		break;
	}

	free(pBuffer);
	return pOutBuffer;
}
