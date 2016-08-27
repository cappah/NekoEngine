/* NekoEngine
 *
 * GLESTexture.cpp
 * Author: Alexandru Naiman
 *
 * OpenGL|ES 3 Renderer Implementation
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

#include "GLESTexture.h"

#include <stdlib.h>
#include <string.h>

#include "GLESRenderer.h"

#ifdef NE_PLATFORM_IOS
#define GL_TEXTURE_2D_MULTISAMPLE 0
#define GL_CLAMP_TO_BORDER	0
#define GL_COMPRESSED_RGB_S3TC_DXT1_EXT 0
#define GL_COMPRESSED_RGBA_S3TC_DXT1_EXT 0
#define GL_COMPRESSED_RGBA_S3TC_DXT3_EXT 0
#define GL_COMPRESSED_RGBA_S3TC_DXT5_EXT 0
#endif

GLenum GL_TexTarget[5] =
{
    0,
    GL_TEXTURE_2D,
    GL_TEXTURE_3D,
    GL_TEXTURE_CUBE_MAP,
    GL_TEXTURE_2D_MULTISAMPLE
};

GLenum GL_TexFilter[4] =
{
    GL_NEAREST,
    GL_LINEAR,
    GL_LINEAR_MIPMAP_NEAREST,
    GL_LINEAR_MIPMAP_LINEAR
};

GLenum GL_TexWrap[4] =
{
    GL_REPEAT,
    GL_MIRRORED_REPEAT,
    GL_CLAMP_TO_EDGE,
    0
};

GLenum GL_TexFormatSized[22] =
{
    GL_R8,
    GL_RG8,
    GL_RGB8,
    GL_RGBA8,
    GL_R16F,
    GL_RG16F,
    GL_RGB16F,
    GL_RGBA16F,
    GL_R32F,
    GL_RG32F,
    GL_RGB32F,
    GL_RGBA32F,
    GL_DEPTH_COMPONENT16,
    GL_DEPTH_COMPONENT24,
    0,
    GL_DEPTH_COMPONENT32F,
    GL_DEPTH24_STENCIL8,
    GL_DEPTH32F_STENCIL8,
    GL_R8UI,
    GL_RG8UI,
    GL_RGB8UI,
    GL_RGBA8UI
};

GLenum GL_TexFormat[10] =
{
    GL_RED,
    GL_RG,
    GL_RGB,
    GL_RGBA,
    GL_DEPTH_COMPONENT,
    GL_DEPTH_STENCIL,
    GL_RED_INTEGER,
    GL_RG_INTEGER,
    GL_RGB_INTEGER,
    GL_RGBA_INTEGER
};

GLenum GL_TexType[5] =
{
    GL_UNSIGNED_BYTE,
    GL_UNSIGNED_INT,
    GL_FLOAT,
    GL_HALF_FLOAT,
    GL_UNSIGNED_INT_24_8
};

GLenum GL_CubeFace[6] =
{
    GL_TEXTURE_CUBE_MAP_POSITIVE_X,
    GL_TEXTURE_CUBE_MAP_NEGATIVE_X,
    GL_TEXTURE_CUBE_MAP_POSITIVE_Y,
    GL_TEXTURE_CUBE_MAP_NEGATIVE_Y,
    GL_TEXTURE_CUBE_MAP_POSITIVE_Z,
    GL_TEXTURE_CUBE_MAP_NEGATIVE_Z
};

GLESTexture::GLESTexture(TextureType type)
	: RTexture(type)
{
    _fixedLocations = false;
    _sizedFormat = TextureSizedFormat::RGBA_8U;
	_skipMipLevels = 0;
    GL_CHECK(glGenTextures(1, &_id));
}

void GLESTexture::Bind()
{
	GL_CHECK(glBindTexture(GL_TexTarget[(int)_type], _id));
}

bool GLESTexture::_LoadTGATexture(char *tga, int bpp)
{
	int levelSize = _width;
	int levels = 0;

	if (_type == TextureType::Tex2D)
	{
		while (levelSize > 1)
		{
			levelSize /= 2;
			++levels;
		}

		SetStorage2D(levels, bpp == 24 ? TextureSizedFormat::RGB_8U : TextureSizedFormat::RGBA_8U, _width, _height);
		SetImage2D(0, _width, _height, bpp == 24 ? TextureFormat::RGB : TextureFormat::RGBA, TextureInternalType::UnsignedByte, tga);
	}
	else if (_type == TextureType::TexCubemap)
	{
		int size = _width / 4;
		int pixelSize = bpp / 8;
		int rowSize = pixelSize * size;
		int imageSize = size * size * pixelSize;

		int centerRowOffset = 4 * imageSize;
		int bottomRowOffset = 8 * imageSize;

		int levelSize = size;

		while (levelSize > 1)
		{
			levelSize /= 2;
			levels++;
		}

		char *cubemap = (char *)calloc(imageSize, 6);
		if (!cubemap)
			return false;

		for (int i = 0; i < size; i++)
		{
			int dstOffset = rowSize * i;
			int rowOffset = (4 * rowSize * i);

			memcpy(cubemap + dstOffset, tga + centerRowOffset + rowOffset + (rowSize * 2), rowSize);
			memcpy((cubemap + imageSize) + dstOffset, tga + centerRowOffset + rowOffset, rowSize);
			memcpy((cubemap + imageSize * 2) + dstOffset, tga + rowOffset + rowSize, rowSize);
			memcpy((cubemap + imageSize * 3) + dstOffset, tga + bottomRowOffset + rowOffset + rowSize, rowSize);
			memcpy((cubemap + imageSize * 4) + dstOffset, tga + centerRowOffset + rowOffset + rowSize, rowSize);
			memcpy((cubemap + imageSize * 5) + dstOffset, tga + centerRowOffset + rowOffset + (rowSize * 3), rowSize);
		}

		SetStorageCube(levels, bpp == 24 ? TextureSizedFormat::RGB_8U : TextureSizedFormat::RGBA_8U, size, size);
		SetImageCube(0, size, size, bpp == 24 ? TextureFormat::RGB : TextureFormat::RGBA, TextureInternalType::UnsignedByte,
					 (void *)cubemap, (void *)(cubemap + imageSize), (void *)(cubemap + imageSize * 2), (void *)(cubemap + imageSize * 3), (void *)(cubemap + imageSize * 4), (void *)(cubemap + imageSize * 5));

		free(cubemap);
	}

	return true;
}

bool GLESTexture::LoadFromFile(const char* file)
{
	const char *ext = strrchr(file, '.');

	if (!strncmp(++ext, "tga", 3))
	{
		int bpp;
		char *data = _loadTGA(file, &_width, &_height, &bpp);
		bool ret = _LoadTGATexture(data, bpp);
		free(data);
		return ret;
	}
	else
		return false;

	return false;
}

bool GLESTexture::LoadFromMemory(TextureFileFormat format, const uint8_t* mem, size_t size)
{
	if (format == TextureFileFormat::TGA)
	{
		int bpp;
		char *data = _loadTGAFromMemory(mem, size, &_width, &_height, &bpp);
		bool ret = _LoadTGATexture(data, bpp);
		free(data);
		return ret;
	}

	return false;
}

void GLESTexture::GetImage(int level, TextureFormat format, TextureInternalType type, size_t size, void *buff)
{
	// Unsupported in GL|ES
}

void GLESTexture::SetStorage1D(int levels, TextureSizedFormat format, int width)
{
	// Unsupported in GL|ES
}

void GLESTexture::SetStorage2D(int levels, TextureSizedFormat format, int width, int height)
{
	GL_CHECK(glBindTexture(GL_TexTarget[(int)_type], _id));
	GL_CHECK(glTexStorage2D(GL_TexTarget[(int)_type], levels, GL_TexFormatSized[(int)format], width, height));

	_width = width;
	_height = height;
	_mipLevels = levels;
	_sizedFormat = format;
}

void GLESTexture::SetStorage3D(int levels, TextureSizedFormat format, int width, int height, int depth)
{
	GL_CHECK(glBindTexture(GL_TexTarget[(int)_type], _id));
	GL_CHECK(glTexStorage3D(GL_TexTarget[(int)_type], levels, GL_TexFormatSized[(int)format], width, height, depth));

	_width = width;
	_height = height;
	_depth = depth;
	_mipLevels = levels;
	_sizedFormat = format;
}

void GLESTexture::SetStorage2DMS(int samples, int width, int height, TextureSizedFormat format, bool fixedSampleLocations)
{
	/*GL_CHECK(glBindTexture(GL_TexTarget[(int)_type], _id));
	GL_CHECK(glTexImage2DMultisample(GL_TexTarget[(int)_type], samples, GL_TexFormatSized[(int)format], width, height, fixedSampleLocations));

	_width = width;
	_height = height;
	_samples = samples;
	_sizedFormat = format;
	_fixedLocations = fixedSampleLocations;*/
}

void GLESTexture::SetStorageCube(int levels, TextureSizedFormat format, int width, int height)
{
	GL_CHECK(glBindTexture(GL_TexTarget[(int)_type], _id));
	GL_CHECK(glTexStorage2D(GL_TexTarget[(int)_type], levels, GL_TexFormatSized[(int)format], width, height));

	_width = width;
	_height = height;
	_mipLevels = levels;
	_sizedFormat = format;
}

void GLESTexture::SetImage1D(int level, int width, TextureFormat format, TextureInternalType type, const void* data)
{
	// Unsupported in GL|ES
}

void GLESTexture::SetImage2D(int level, int width, int height, TextureFormat format, TextureInternalType type, const void* data)
{
	GL_CHECK(glBindTexture(GL_TexTarget[(int)_type], _id));
	GL_CHECK(glTexSubImage2D(GL_TexTarget[(int)_type], level, 0, 0, width, height, GL_TexFormat[(int)format], GL_TexType[(int)type], data));

	_format = format;
}

void GLESTexture::SetImage3D(int level, int width, int height, int depth, TextureFormat format, TextureInternalType type, const void* data)
{
	GL_CHECK(glBindTexture(GL_TexTarget[(int)_type], _id));
	GL_CHECK(glTexSubImage3D(GL_TexTarget[(int)_type], level, 0, 0, 0, width, height, depth, GL_TexFormat[(int)format], GL_TexType[(int)type], data));

	_format = format;
}

void GLESTexture::SetSubImage1D(int level, int x, int width, TextureFormat format, TextureInternalType type, const void* data)
{
	// Unsupported in GL|ES
}

void GLESTexture::SetSubImage2D(int level, int x, int y, int width, int height, TextureFormat format, TextureInternalType type, const void* data)
{
	GL_CHECK(glBindTexture(GL_TexTarget[(int)_type], _id));
	GL_CHECK(glTexSubImage2D(GL_TexTarget[(int)_type], level, x, y, width, height, GL_TexFormat[(int)format], GL_TexType[(int)type], data));

	_format = format;
}

void GLESTexture::SetSubImage3D(int level, int x, int y, int z, int width, int height, int depth, TextureFormat format, TextureInternalType type, const void* data)
{
	GL_CHECK(glBindTexture(GL_TexTarget[(int)_type], _id));
	GL_CHECK(glTexSubImage3D(GL_TexTarget[(int)_type], level, x, y, z, width, height, depth, GL_TexFormat[(int)format], GL_TexType[(int)type], data));

	_format = format;
}

void GLESTexture::SetImageCube(int level, int width, int height, TextureFormat format, TextureInternalType type,
                              const void *posX, const void *negX, const void *posY, const void *negY, const void *posZ, const void *negZ)
{
	GL_CHECK(glBindTexture(GL_TexTarget[(int)_type], _id));

	GL_CHECK(glTexSubImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X, level, 0, 0, width, height, GL_TexFormat[(int)format], GL_TexType[(int)type], posX));
	GL_CHECK(glTexSubImage2D(GL_TEXTURE_CUBE_MAP_NEGATIVE_X, level, 0, 0, width, height, GL_TexFormat[(int)format], GL_TexType[(int)type], negX));
	GL_CHECK(glTexSubImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_Y, level, 0, 0, width, height, GL_TexFormat[(int)format], GL_TexType[(int)type], posY));
	GL_CHECK(glTexSubImage2D(GL_TEXTURE_CUBE_MAP_NEGATIVE_Y, level, 0, 0, width, height, GL_TexFormat[(int)format], GL_TexType[(int)type], negY));
	GL_CHECK(glTexSubImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_Z, level, 0, 0, width, height, GL_TexFormat[(int)format], GL_TexType[(int)type], posZ));
	GL_CHECK(glTexSubImage2D(GL_TEXTURE_CUBE_MAP_NEGATIVE_Z, level, 0, 0, width, height, GL_TexFormat[(int)format], GL_TexType[(int)type], negZ));

	_format = format;
}

void GLESTexture::SetImageCubeFace(CubeFace face, int level, int width, int height, TextureFormat format, TextureInternalType type, const void *data)
{
	GL_CHECK(glBindTexture(GL_TexTarget[(int)_type], _id));
	GL_CHECK(glTexSubImage2D(GL_CubeFace[(int)face], level, 0, 0, width, height, GL_TexFormat[(int)format], GL_TexType[(int)type], data));

	_format = format;
}

void GLESTexture::SetMinFilter(TextureFilter filter)
{
	GL_CHECK(glBindTexture(GL_TexTarget[(int)_type], _id));
    GL_CHECK(glTexParameteri(GL_TexTarget[(int)_type], GL_TEXTURE_MIN_FILTER, GL_TexFilter[(int)filter]));
}

void GLESTexture::SetMagFilter(TextureFilter filter)
{
	GL_CHECK(glBindTexture(GL_TexTarget[(int)_type], _id));
    GL_CHECK(glTexParameteri(GL_TexTarget[(int)_type], GL_TEXTURE_MAG_FILTER, GL_TexFilter[(int)filter]));
}

void GLESTexture::SetAnisotropic(int aniso)
{
	GL_CHECK(glBindTexture(GL_TexTarget[(int)_type], _id));
    GL_CHECK(glTexParameteri(GL_TexTarget[(int)_type], GL_TEXTURE_MAX_ANISOTROPY_EXT, aniso));
}

void GLESTexture::SetWrapS(TextureWrap wrap)
{
	GL_CHECK(glBindTexture(GL_TexTarget[(int)_type], _id));
    GL_CHECK(glTexParameteri(GL_TexTarget[(int)_type], GL_TEXTURE_WRAP_S, GL_TexWrap[(int)wrap]));
}

void GLESTexture::SetWrapT(TextureWrap wrap)
{
	GL_CHECK(glBindTexture(GL_TexTarget[(int)_type], _id));
    GL_CHECK(glTexParameteri(GL_TexTarget[(int)_type], GL_TEXTURE_WRAP_T, GL_TexWrap[(int)wrap]));
}

void GLESTexture::SetWrapR(TextureWrap wrap)
{
	GL_CHECK(glBindTexture(GL_TexTarget[(int)_type], _id));
	GL_CHECK(glTexParameteri(GL_TexTarget[(int)_type], GL_TEXTURE_WRAP_R, GL_TexWrap[(int)wrap]));
}

void GLESTexture::Resize1D(int width)
{
    _Destroy();
	GL_CHECK(glGenTextures(1, &_id));
    SetStorage1D(_mipLevels, _sizedFormat, width);
}

void GLESTexture::Resize2D(int width, int height)
{
    _Destroy();
	GL_CHECK(glGenTextures(1, &_id));

    if (_type == TextureType::Tex2D)
        SetStorage2D(_mipLevels, _sizedFormat, width, height);
    else
        SetStorage2DMS(_samples, width, height, _sizedFormat, _fixedLocations);
}

void GLESTexture::Resize3D(int width, int height, int depth)
{
    _Destroy();
	GL_CHECK(glGenTextures(1, &_id));
    SetStorage3D(_mipLevels, _sizedFormat, width, height, depth);
}

void GLESTexture::ResizeCubemap(int width, int height)
{
    _Destroy();
	GL_CHECK(glGenTextures(1, &_id));
    SetStorageCube(_mipLevels, _sizedFormat, width, height);
}

void GLESTexture::GenerateMipmaps()
{
    GL_CHECK(glBindTexture(GL_TexTarget[(int)_type], _id));
    GL_CHECK(glGenerateMipmap(GL_TexTarget[(int)_type]));
}

void GLESTexture::_Destroy()
{
    GL_CHECK(glDeleteTextures(1, &_id));
}

GLESTexture::~GLESTexture()
{
    _Destroy();
}
