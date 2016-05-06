/* Neko Engine
 *
 * IGLTexture.mm
 * Author: Alexandru Naiman
 *
 * iOS OpenGL|ES Renderer Implementation
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

#include "IGLTexture.h"

#include <stdlib.h>
#include <string.h>

#include <OpenGLES/ES3/glext.h>

#include "IGLRenderer.h"

#ifdef NE_PLATFORM_IOS
#define GL_TEXTURE_1D 0
#define GL_TEXTURE_2D_MULTISAMPLE 0
#define GL_CLAMP_TO_BORDER	0
#define GL_DEPTH_COMPONENT32 0
#define GL_COMPRESSED_RGB_S3TC_DXT1_EXT 0
#define GL_COMPRESSED_RGBA_S3TC_DXT1_EXT 0
#define GL_COMPRESSED_RGBA_S3TC_DXT3_EXT 0
#define GL_COMPRESSED_RGBA_S3TC_DXT5_EXT 0
#endif

GLenum GL_TexTarget[5] =
{
    GL_TEXTURE_1D,
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
    GL_CLAMP_TO_BORDER
};

GLenum GL_TexFormatSized[26] =
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
    GL_DEPTH_COMPONENT32,
    GL_DEPTH_COMPONENT32F,
    GL_DEPTH24_STENCIL8,
    GL_DEPTH32F_STENCIL8,
    GL_R8UI,
    GL_RG8UI,
    GL_RGB8UI,
    GL_RGBA8UI,
    GL_COMPRESSED_RGB_S3TC_DXT1_EXT,
    GL_COMPRESSED_RGBA_S3TC_DXT1_EXT,
    GL_COMPRESSED_RGBA_S3TC_DXT3_EXT,
    GL_COMPRESSED_RGBA_S3TC_DXT5_EXT
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

GLenum GL_CompressedTexFormat[4] =
{
    GL_COMPRESSED_RGB_S3TC_DXT1_EXT,
    GL_COMPRESSED_RGBA_S3TC_DXT1_EXT,
    GL_COMPRESSED_RGBA_S3TC_DXT3_EXT,
    GL_COMPRESSED_RGBA_S3TC_DXT5_EXT
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

IGLTexture::IGLTexture(TextureType type)
: RTexture(type)
{
    GL_CHECK(glGenTextures(1, &_id));
}

void IGLTexture::Bind()
{
	GL_CHECK(glBindTexture(GL_TexTarget[(int)_type], _id));
}

bool IGLTexture::_LoadTGATexture(char *tga, int bpp)
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
		
#pragma omp parallel for
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

#ifndef NE_PLATFORM_IOS
bool IGLTexture::_LoadDDSTexture(class nv_dds::CDDSImage& image)
{
	bool ret = false;
	
	if (!image.is_valid())
		return false;
	
	if (_type == TextureType::Tex1D)
	{
		GL_CHECK(glBindTexture(GL_TEXTURE_1D, _id));
		ret = image.upload_texture1D();
		GL_CHECK(glBindTexture(GL_TEXTURE_1D, 0));
	}
	else if (_type == TextureType::Tex2D)
	{
		GL_CHECK(glBindTexture(GL_TEXTURE_2D, _id));
		ret = image.upload_texture2D();
		GL_CHECK(glBindTexture(GL_TEXTURE_2D, 0));
	}
	else if (_type == TextureType::Tex3D)
	{
		GL_CHECK(glBindTexture(GL_TEXTURE_3D, _id));
		ret = image.upload_texture3D();
		GL_CHECK(glBindTexture(GL_TEXTURE_3D, 0));
	}
	else if (_type == TextureType::TexCubemap)
	{
		GL_CHECK(glBindTexture(GL_TEXTURE_CUBE_MAP, _id));
		ret = image.upload_texture2D();
		GL_CHECK(glBindTexture(GL_TEXTURE_CUBE_MAP, 0));
	}
	
	_width = image.get_width();
	_height = image.get_height();
	
	return ret;
}
#endif

bool IGLTexture::LoadFromFile(const char* file)
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
	#ifndef NE_PLATFORM_IOS
	else if (!strncmp(ext, "dds", 3))
	{
		nv_dds::CDDSImage image;
		
		image.load(file);
		bool ret = _LoadDDSTexture(image);
		
		image.clear();
		
		return ret;

	}
	#endif
	else
		return false;
	
	return false;
}

bool IGLTexture::LoadFromMemory(TextureFileFormat format, const uint8_t* mem, size_t size)
{
	if (format == TextureFileFormat::TGA)
	{
		int bpp;
		char *data = _loadTGAFromMemory(mem, size, &_width, &_height, &bpp);
		bool ret = _LoadTGATexture(data, bpp);
		free(data);
		return ret;
	}
	#ifndef NE_PLATFORM_IOS
	else if (format == TextureFileFormat::DDS)
	{
		nv_dds::CDDSImage image;
		
		image.loadFromMemory(mem, size);
		bool ret = _LoadDDSTexture(image);
		
		image.clear();
		
		return ret;
	}
	#endif
	
	return false;
}

void IGLTexture::SetStorage1D(int levels, TextureSizedFormat format, int width)
{
	/*GL_CHECK(glBindTexture(GL_TexTarget[(int)_type], _id));
	GL_CHECK(glTexStorage1D(GL_TexTarget[(int)_type], levels, GL_TexFormatSized[(int)format], width));
	
	_width = width;
	_mipLevels = levels;
	_sizedFormat = format;*/
}

void IGLTexture::SetStorage2D(int levels, TextureSizedFormat format, int width, int height)
{
	GL_CHECK(glBindTexture(GL_TexTarget[(int)_type], _id));
	GL_CHECK(glTexStorage2D(GL_TexTarget[(int)_type], levels, GL_TexFormatSized[(int)format], width, height));
	
	_width = width;
	_height = height;
	_mipLevels = levels;
	_sizedFormat = format;
}

void IGLTexture::SetStorage3D(int levels, TextureSizedFormat format, int width, int height, int depth)
{
	GL_CHECK(glBindTexture(GL_TexTarget[(int)_type], _id));
	GL_CHECK(glTexStorage3D(GL_TexTarget[(int)_type], levels, GL_TexFormatSized[(int)format], width, height, depth));
	
	_width = width;
	_height = height;
	_depth = depth;
	_mipLevels = levels;
	_sizedFormat = format;
}

void IGLTexture::SetStorage2DMS(int samples, int width, int height, TextureSizedFormat format, bool fixedSampleLocations)
{
	/*GL_CHECK(glBindTexture(GL_TexTarget[(int)_type], _id));
	GL_CHECK(glTexImage2DMultisample(GL_TexTarget[(int)_type], samples, GL_TexFormatSized[(int)format], width, height, fixedSampleLocations));
	
	_width = width;
	_height = height;
	_samples = samples;
	_sizedFormat = format;
	_fixedLocations = fixedSampleLocations;*/
}

void IGLTexture::SetStorageCube(int levels, TextureSizedFormat format, int width, int height)
{
	GL_CHECK(glBindTexture(GL_TexTarget[(int)_type], _id));
	GL_CHECK(glTexStorage2D(GL_TexTarget[(int)_type], levels, GL_TexFormatSized[(int)format], width, height));
	
	_width = width;
	_height = height;
	_mipLevels = levels;
	_sizedFormat = format;
}

void IGLTexture::SetImage1D(int level, int width, TextureFormat format, TextureInternalType type, const void* data)
{
	/*GL_CHECK(glBindTexture(GL_TexTarget[(int)_type], _id));
	GL_CHECK(glTexSubImage1D(GL_TexTarget[(int)_type], level, 0, width, GL_TexFormat[(int)format], GL_TexType[(int)type], data));
	
	_format = format;*/
}

void IGLTexture::SetImage2D(int level, int width, int height, TextureFormat format, TextureInternalType type, const void* data)
{
	GL_CHECK(glBindTexture(GL_TexTarget[(int)_type], _id));
	GL_CHECK(glTexSubImage2D(GL_TexTarget[(int)_type], level, 0, 0, width, height, GL_TexFormat[(int)format], GL_TexType[(int)type], data));
	
	_format = format;
}

void IGLTexture::SetImage3D(int level, int width, int height, int depth, TextureFormat format, TextureInternalType type, const void* data)
{
	GL_CHECK(glBindTexture(GL_TexTarget[(int)_type], _id));
	GL_CHECK(glTexSubImage3D(GL_TexTarget[(int)_type], level, 0, 0, 0, width, height, depth, GL_TexFormat[(int)format], GL_TexType[(int)type], data));
	
	_format = format;
}

void IGLTexture::SetImageCube(int level, int width, int height, TextureFormat format, TextureInternalType type,
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

void IGLTexture::SetImageCubeFace(CubeFace face, int level, int width, int height, TextureFormat format, TextureInternalType type, const void *data)
{
	GL_CHECK(glBindTexture(GL_TexTarget[(int)_type], _id));
	GL_CHECK(glTexSubImage2D(GL_CubeFace[(int)face], level, 0, 0, width, height, GL_TexFormat[(int)format], GL_TexType[(int)type], data));
	
	_format = format;
}

void IGLTexture::SetCompressedImage1D(int level, int width, CompressedTextureFormat format, int size, const void* data)
{
	/*GL_CHECK(glBindTexture(GL_TexTarget[(int)_type], _id));
	GL_CHECK(glCompressedTexSubImage1D(GL_TexTarget[(int)_type], level, 0, width, GL_CompressedTexFormat[(int)format], size, data));
	
	_compressed = true;
	_compressedFormat = format;*/
}

void IGLTexture::SetCompressedImage2D(int level, int width, int height, CompressedTextureFormat format, int size, const void* data)
{
	GL_CHECK(glBindTexture(GL_TexTarget[(int)_type], _id));
	GL_CHECK(glCompressedTexImage2D(GL_TEXTURE_2D, level, GL_CompressedTexFormat[(int)format], width, height, 0, size, data));
	
	_compressed = true;
	_compressedFormat = format;
}

void IGLTexture::SetCompressedImage3D(int level, int width, int height, int depth, CompressedTextureFormat format, int size, const void* data)
{
	GL_CHECK(glBindTexture(GL_TexTarget[(int)_type], _id));
    GL_CHECK(glCompressedTexImage3D(GL_TexTarget[(int)_type], level, GL_CompressedTexFormat[(int)format], width, height, depth, 0, size, data));
	
	_compressed = true;
	_compressedFormat = format;
}

void IGLTexture::SetCompressedImageCube(int level, int width, int height, CompressedTextureFormat format, int size,
                                        const void *posX, const void *negX, const void *posY, const void *negY, const void *posZ, const void *negZ)
{
	GL_CHECK(glBindTexture(GL_TexTarget[(int)_type], _id));
	
    GL_CHECK(glCompressedTexSubImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X, level, 0, 0, width, height, GL_CompressedTexFormat[(int)format], size, posX));
	GL_CHECK(glCompressedTexSubImage2D(GL_TEXTURE_CUBE_MAP_NEGATIVE_X, level, 0, 0, width, height, GL_CompressedTexFormat[(int)format], size, negX));
    GL_CHECK(glCompressedTexSubImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_Y, level, 0, 0, width, height, GL_CompressedTexFormat[(int)format], size, posY));
    GL_CHECK(glCompressedTexSubImage2D(GL_TEXTURE_CUBE_MAP_NEGATIVE_Y, level, 0, 0, width, height, GL_CompressedTexFormat[(int)format], size, negY));
    GL_CHECK(glCompressedTexSubImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_Z, level, 0, 0, width, height, GL_CompressedTexFormat[(int)format], size, posZ));
    GL_CHECK(glCompressedTexSubImage2D(GL_TEXTURE_CUBE_MAP_NEGATIVE_Z, level, 0, 0, width, height, GL_CompressedTexFormat[(int)format], size, negZ));
	
	_compressed = true;
	_compressedFormat = format;
}

void IGLTexture::SetCompressedImageCubeFace(CubeFace face, int level, int width, int height, CompressedTextureFormat format, int size, const void *data)
{
	/*GL_CHECK(glBindTexture(GL_TexTarget[(int)_type], _id));
	GL_CHECK(glCompressedTexSubImage1D(GL_CubeFace[(int)face], level, 0, width, GL_CompressedTexFormat[(int)format], size, data));

	_compressed = true;
	_compressedFormat = format;*/
}

void IGLTexture::SetMinFilter(TextureFilter filter)
{
	GL_CHECK(glBindTexture(GL_TexTarget[(int)_type], _id));
    GL_CHECK(glTexParameteri(GL_TexTarget[(int)_type], GL_TEXTURE_MIN_FILTER, GL_TexFilter[(int)filter]));
}

void IGLTexture::SetMagFilter(TextureFilter filter)
{
	GL_CHECK(glBindTexture(GL_TexTarget[(int)_type], _id));
    GL_CHECK(glTexParameteri(GL_TexTarget[(int)_type], GL_TEXTURE_MAG_FILTER, GL_TexFilter[(int)filter]));
}

void IGLTexture::SetAnisotropic(int aniso)
{
	GL_CHECK(glBindTexture(GL_TexTarget[(int)_type], _id));
    GL_CHECK(glTexParameteri(GL_TexTarget[(int)_type], GL_TEXTURE_MAX_ANISOTROPY_EXT, aniso));
}

void IGLTexture::SetWrapS(TextureWrap wrap)
{
	GL_CHECK(glBindTexture(GL_TexTarget[(int)_type], _id));
    GL_CHECK(glTexParameteri(GL_TexTarget[(int)_type], GL_TEXTURE_WRAP_S, GL_TexWrap[(int)wrap]));
}

void IGLTexture::SetWrapT(TextureWrap wrap)
{
	GL_CHECK(glBindTexture(GL_TexTarget[(int)_type], _id));
    GL_CHECK(glTexParameteri(GL_TexTarget[(int)_type], GL_TEXTURE_WRAP_T, GL_TexWrap[(int)wrap]));
}

void IGLTexture::SetWrapR(TextureWrap wrap)
{
	GL_CHECK(glBindTexture(GL_TexTarget[(int)_type], _id));
	GL_CHECK(glTexParameteri(GL_TexTarget[(int)_type], GL_TEXTURE_WRAP_R, GL_TexWrap[(int)wrap]));
}

void IGLTexture::Resize1D(int width)
{
    _Destroy();
    SetStorage1D(_mipLevels, _sizedFormat, width);
}

void IGLTexture::Resize2D(int width, int height)
{
    _Destroy();
    
    if (_type == TextureType::Tex2D)
        SetStorage2D(_mipLevels, _sizedFormat, width, height);
    else
        SetStorage2DMS(_samples, width, height, _sizedFormat, _fixedLocations);
}

void IGLTexture::Resize3D(int width, int height, int depth)
{
    _Destroy();
    SetStorage3D(_mipLevels, _sizedFormat, width, height, depth);
}

void IGLTexture::ResizeCubemap(int width, int height)
{
    _Destroy();
    SetStorageCube(_mipLevels, _sizedFormat, width, height);
}

void IGLTexture::GenerateMipmaps()
{
    GL_CHECK(glBindTexture(GL_TexTarget[(int)_type], _id));
    GL_CHECK(glGenerateMipmap(GL_TexTarget[(int)_type]));
}

void IGLTexture::_Destroy()
{
    GL_CHECK(glDeleteTextures(1, &_id));
}

IGLTexture::~IGLTexture()
{
    _Destroy();
}