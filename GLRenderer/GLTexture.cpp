/* Neko Engine
 *
 * GLTexture.cpp
 * Author: Alexandru Naiman
 *
 * OpenGL Renderer Implementation
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

#include "GLTexture.h"
#include "GLRenderer.h"

#include <string.h>

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

GLTexture::GLTexture(TextureType type, bool create)
	: RTexture(type)
{
	_id = 0;
	_resident = false;
	_handle = 0;
	_fixedLocations = false;
	_sizedFormat = TextureSizedFormat::RGBA_8U;

	if (!create)
		return;
	
	GL_CHECK(glCreateTextures(GL_TexTarget[(int)type], 1, &_id));
}

void GLTexture::Bind()
{
	GL_CHECK(glBindTexture(GL_TexTarget[(int)_type], _id));
}

void GLTexture::MakeResident()
{
	GL_CHECK(_handle = glGetTextureHandleARB(_id));
	GL_CHECK(glMakeTextureHandleResidentARB(_handle));

	_resident = true;
}

bool GLTexture::_LoadTGATexture(char *tga, int bpp)
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

bool GLTexture::_LoadDDSTexture(class nv_dds::CDDSImage& image)
{
	bool ret = false;

	if (!image.is_valid())
		return false;
	
	if (_type == TextureType::Tex1D)
	{
		glBindTexture(GL_TEXTURE_1D, _id);
		ret = image.upload_texture1D();
		glBindTexture(GL_TEXTURE_1D, 0);
	}
	else if (_type == TextureType::Tex2D)
	{
		glBindTexture(GL_TEXTURE_2D, _id);
		ret = image.upload_texture2D();
		glBindTexture(GL_TEXTURE_2D, 0);
	}
	else if (_type == TextureType::Tex3D)
	{
		glBindTexture(GL_TEXTURE_3D, _id);
		ret = image.upload_texture3D();
		glBindTexture(GL_TEXTURE_3D, 0);
	}
	else if (_type == TextureType::TexCubemap)
	{
		glBindTexture(GL_TEXTURE_CUBE_MAP, _id);
		ret = image.upload_texture2D();
		glBindTexture(GL_TEXTURE_CUBE_MAP, 0);
	}

	_width = image.get_width();
	_height = image.get_height();

	return ret;
}

bool GLTexture::LoadFromFile(const char* file)
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
	else if (!strncmp(ext, "dds", 3))
	{
		nv_dds::CDDSImage image;

		image.load(file);
		bool ret = _LoadDDSTexture(image);

		image.clear();

		return ret;
	}
	else
		return false;

	return false;
}

bool GLTexture::LoadFromMemory(TextureFileFormat format, const uint8_t* mem, size_t size)
{
	if (format == TextureFileFormat::TGA)
	{
		int bpp;
		char *data = _loadTGAFromMemory(mem, size, &_width, &_height, &bpp);
		bool ret = _LoadTGATexture(data, bpp);
		free(data);
		return ret;
	}
	else if (format == TextureFileFormat::DDS)
	{
		nv_dds::CDDSImage image;

		image.loadFromMemory(mem, size);
		bool ret = _LoadDDSTexture(image);

		image.clear();

		return ret;
	}

	return false;
}

void GLTexture::SetStorage1D(int levels, TextureSizedFormat format, int width)
{
	GL_CHECK(glTextureStorage1D(_id, levels, GL_TexFormatSized[(int)format], width));
	
	_width = width;
	_mipLevels = levels;
	_sizedFormat = format;
}

void GLTexture::SetStorage2D(int levels, TextureSizedFormat format, int width, int height)
{
	GL_CHECK(glTextureStorage2D(_id, levels, GL_TexFormatSized[(int)format], width, height));

	_width = width;
	_height = height;
	_mipLevels = levels;
	_sizedFormat = format;
}

void GLTexture::SetStorage3D(int levels, TextureSizedFormat format, int width, int height, int depth)
{
	GL_CHECK(glTextureStorage3D(_id, levels, GL_TexFormatSized[(int)format], width, height, depth));

	_width = width;
	_height = height;
	_depth = depth;
	_mipLevels = levels;
	_sizedFormat = format;
}

void GLTexture::SetStorage2DMS(int samples, int width, int height, TextureSizedFormat format, bool fixedSampleLocations)
{
	GL_CHECK(glTextureStorage2DMultisample(_id, samples, GL_TexFormatSized[(int)format], width, height, fixedSampleLocations));

	_width = width;
	_height = height;
	_samples = samples;
	_sizedFormat = format;
	_fixedLocations = fixedSampleLocations;
}

void GLTexture::SetStorageCube(int levels, TextureSizedFormat format, int width, int height)
{
	GL_CHECK(glTextureStorage2D(_id, levels, GL_TexFormatSized[(int)format], width, height));

	_width = width;
	_height = height;
	_mipLevels = levels;
	_sizedFormat = format;
}

void GLTexture::SetImage1D(int level, int width, TextureFormat format, TextureInternalType type, const void* data)
{
	_format = format;
	GL_CHECK(glTextureSubImage1D(_id, level, 0, width, GL_TexFormat[(int)format], GL_TexType[(int)type], data));
}

void GLTexture::SetImage2D(int level, int width, int height, TextureFormat format, TextureInternalType type, const void* data)
{
	_format = format;
	GL_CHECK(glTextureSubImage2D(_id, level, 0, 0, width, height, GL_TexFormat[(int)format], GL_TexType[(int)type], data));
}

void GLTexture::SetImage3D(int level, int width, int height, int depth, TextureFormat format, TextureInternalType type, const void* data)
{
	_format = format;
	GL_CHECK(glTextureSubImage3D(_id, level, 0, 0, 0, width, height, depth, GL_TexFormat[(int)format], GL_TexType[(int)type], data));
}

void GLTexture::SetImageCube(int level, int width, int height, TextureFormat format, TextureInternalType type,
	const void *posX, const void *negX, const void *posY, const void *negY, const void *posZ, const void *negZ)
{
	_format = format;
	GL_CHECK(glBindTexture(GL_TEXTURE_CUBE_MAP, _id));
	GL_CHECK(glTexSubImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X, level, 0, 0, width, height, GL_TexFormat[(int)format], GL_TexType[(int)type], posX));
	GL_CHECK(glTexSubImage2D(GL_TEXTURE_CUBE_MAP_NEGATIVE_X, level, 0, 0, width, height, GL_TexFormat[(int)format], GL_TexType[(int)type], negX));
	GL_CHECK(glTexSubImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_Y, level, 0, 0, width, height, GL_TexFormat[(int)format], GL_TexType[(int)type], posY));
	GL_CHECK(glTexSubImage2D(GL_TEXTURE_CUBE_MAP_NEGATIVE_Y, level, 0, 0, width, height, GL_TexFormat[(int)format], GL_TexType[(int)type], negY));
	GL_CHECK(glTexSubImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_Z, level, 0, 0, width, height, GL_TexFormat[(int)format], GL_TexType[(int)type], posZ));
	GL_CHECK(glTexSubImage2D(GL_TEXTURE_CUBE_MAP_NEGATIVE_Z, level, 0, 0, width, height, GL_TexFormat[(int)format], GL_TexType[(int)type], negZ));
	GL_CHECK(glBindTexture(GL_TEXTURE_CUBE_MAP, 0));
}

void GLTexture::SetImageCubeFace(CubeFace face, int level, int width, int height, TextureFormat format, TextureInternalType type, const void *data)
{
	_format = format;
	GL_CHECK(glBindTexture(GL_TEXTURE_CUBE_MAP, _id));
	GL_CHECK(glTexSubImage2D(GL_CubeFace[(int)face], level, 0, 0, width, height, GL_TexFormat[(int)format], GL_TexType[(int)type], data));
	GL_CHECK(glBindTexture(GL_TEXTURE_CUBE_MAP, 0));
}

void GLTexture::SetCompressedImage1D(int level, int width, CompressedTextureFormat format, int size, const void* data)
{
	GL_CHECK(glBindTexture(GL_TexTarget[(int)_type], _id));
	GL_CHECK(glCompressedTexSubImage1D(GL_TexTarget[(int)_type], level, 0, width, GL_CompressedTexFormat[(int)format], size, data));
	
	_compressed = true;
	_compressedFormat = format;
}

void GLTexture::SetCompressedImage2D(int level, int width, int height, CompressedTextureFormat format, int size, const void* data)
{
	GL_CHECK(glBindTexture(GL_TexTarget[(int)_type], _id));
	GL_CHECK(glCompressedTexImage2D(GL_TEXTURE_2D, level, GL_CompressedTexFormat[(int)format], width, height, 0, size, data));
	
	_compressed = true;
	_compressedFormat = format;
}

void GLTexture::SetCompressedImage3D(int level, int width, int height, int depth, CompressedTextureFormat format, int size, const void* data)
{
	GL_CHECK(glBindTexture(GL_TexTarget[(int)_type], _id));
	GL_CHECK(glCompressedTexImage3D(GL_TexTarget[(int)_type], level, GL_CompressedTexFormat[(int)format], width, height, depth, 0, size, data));
	
	_compressed = true;
	_compressedFormat = format;
}

void GLTexture::SetCompressedImageCube(int level, int width, int height, CompressedTextureFormat format, int size,
	const void *posX, const void *negX, const void *posY, const void *negY, const void *posZ, const void *negZ)
{
	_compressed = true;
	_compressedFormat = format;
	GL_CHECK(glBindTexture(GL_TEXTURE_CUBE_MAP, _id));
	GL_CHECK(glCompressedTexSubImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X, level, 0, 0, width, height, GL_CompressedTexFormat[(int)format], size, posX));
	GL_CHECK(glCompressedTexSubImage2D(GL_TEXTURE_CUBE_MAP_NEGATIVE_X, level, 0, 0, width, height, GL_CompressedTexFormat[(int)format], size, negX));
	GL_CHECK(glCompressedTexSubImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_Y, level, 0, 0, width, height, GL_CompressedTexFormat[(int)format], size, posY));
	GL_CHECK(glCompressedTexSubImage2D(GL_TEXTURE_CUBE_MAP_NEGATIVE_Y, level, 0, 0, width, height, GL_CompressedTexFormat[(int)format], size, negY));
	GL_CHECK(glCompressedTexSubImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_Z, level, 0, 0, width, height, GL_CompressedTexFormat[(int)format], size, posZ));
	GL_CHECK(glCompressedTexSubImage2D(GL_TEXTURE_CUBE_MAP_NEGATIVE_Z, level, 0, 0, width, height, GL_CompressedTexFormat[(int)format], size, negZ));
	GL_CHECK(glBindTexture(GL_TEXTURE_CUBE_MAP, 0));
}

void GLTexture::SetCompressedImageCubeFace(CubeFace face, int level, int width, int height, CompressedTextureFormat format, int size, const void *data)
{
	_compressed = true;
	_compressedFormat = format;
	GL_CHECK(glBindTexture(GL_TEXTURE_CUBE_MAP, _id));
	GL_CHECK(glCompressedTexSubImage2D(GL_CubeFace[(int)face], level, 0, 0, width, height, GL_CompressedTexFormat[(int)format], size, data));
	GL_CHECK(glBindTexture(GL_TEXTURE_CUBE_MAP, 0));
}

void GLTexture::SetMinFilter(TextureFilter filter)
{
	GL_CHECK(glTextureParameteri(_id, GL_TEXTURE_MIN_FILTER, GL_TexFilter[(int)filter]));
}

void GLTexture::SetMagFilter(TextureFilter filter)
{
	GL_CHECK(glTextureParameteri(_id, GL_TEXTURE_MAG_FILTER, GL_TexFilter[(int)filter]));
}

void GLTexture::SetAnisotropic(int aniso)
{
	GL_CHECK(glTextureParameteri(_id, GL_TEXTURE_MAX_ANISOTROPY_EXT, aniso));
}

void GLTexture::SetWrapS(TextureWrap wrap)
{
	GL_CHECK(glTextureParameteri(_id, GL_TEXTURE_WRAP_S, GL_TexWrap[(int)wrap]));
}

void GLTexture::SetWrapT(TextureWrap wrap)
{
	GL_CHECK(glTextureParameteri(_id, GL_TEXTURE_WRAP_T, GL_TexWrap[(int)wrap]));
}

void GLTexture::SetWrapR(TextureWrap wrap)
{
	GL_CHECK(glTextureParameteri(_id, GL_TEXTURE_WRAP_R, GL_TexWrap[(int)wrap]));
}

void GLTexture::Resize1D(int width)
{
	_Destroy();
	GL_CHECK(glCreateTextures(GL_TexTarget[(int)_type], 1, &_id));
	SetStorage1D(_mipLevels, _sizedFormat, width);
	MakeResident();
}

void GLTexture::Resize2D(int width, int height)
{
	_Destroy();
	GL_CHECK(glCreateTextures(GL_TexTarget[(int)_type], 1, &_id));

	if (_type == TextureType::Tex2D)
		SetStorage2D(_mipLevels, _sizedFormat, width, height);
	else
		SetStorage2DMS(_samples, width, height, _sizedFormat, _fixedLocations);

	MakeResident();
}

void GLTexture::Resize3D(int width, int height, int depth)
{
	_Destroy();
	GL_CHECK(glCreateTextures(GL_TexTarget[(int)_type], 1, &_id));
	SetStorage3D(_mipLevels, _sizedFormat, width, height, depth);
	MakeResident();
}

void GLTexture::ResizeCubemap(int width, int height)
{
	_Destroy();
	GL_CHECK(glCreateTextures(GL_TexTarget[(int)_type], 1, &_id));
	SetStorageCube(_mipLevels, _sizedFormat, width, height);
	MakeResident();
}

void GLTexture::GenerateMipmaps()
{
	GL_CHECK(glGenerateTextureMipmap(_id));
}

void GLTexture::_Destroy()
{
	if (_resident)
	{ GL_CHECK(glMakeTextureHandleNonResidentARB(_handle)); }

	GL_CHECK(glDeleteTextures(1, &_id));
}

GLTexture::~GLTexture()
{
	_Destroy();
}

// Non-DSA variants

GLTexture_NoDSA::GLTexture_NoDSA(TextureType type)
	: GLTexture(type, false)
{
	GL_CHECK(glGenTextures(1, &_id));
}

void GLTexture_NoDSA::SetStorage1D(int levels, TextureSizedFormat format, int width)
{
	GL_CHECK(glBindTexture(GL_TexTarget[(int)_type], _id));
	GL_CHECK(glTexStorage1D(GL_TexTarget[(int)_type], levels, GL_TexFormatSized[(int)format], width));
	
	_width = width;
	_mipLevels = levels;
	_sizedFormat = format;
}

void GLTexture_NoDSA::SetStorage2D(int levels, TextureSizedFormat format, int width, int height)
{
	GL_CHECK(glBindTexture(GL_TexTarget[(int)_type], _id));
	GL_CHECK(glTexStorage2D(GL_TexTarget[(int)_type], levels, GL_TexFormatSized[(int)format], width, height));
	
	_width = width;
	_height = height;
	_mipLevels = levels;
	_sizedFormat = format;
}

void GLTexture_NoDSA::SetStorage3D(int levels, TextureSizedFormat format, int width, int height, int depth)
{
	GL_CHECK(glBindTexture(GL_TexTarget[(int)_type], _id));
	GL_CHECK(glTexStorage3D(GL_TexTarget[(int)_type], levels, GL_TexFormatSized[(int)format], width, height, depth));
	
	_width = width;
	_height = height;
	_depth = depth;
	_mipLevels = levels;
	_sizedFormat = format;
}

void GLTexture_NoDSA::SetStorage2DMS(int samples, int width, int height, TextureSizedFormat format, bool fixedSampleLocations)
{
	GL_CHECK(glBindTexture(GL_TexTarget[(int)_type], _id));
	GL_CHECK(glTexImage2DMultisample(GL_TexTarget[(int)_type], samples, GL_TexFormatSized[(int)format], width, height, fixedSampleLocations));
	
	_width = width;
	_height = height;
	_samples = samples;
	_sizedFormat = format;
	_fixedLocations = fixedSampleLocations;
}

void GLTexture_NoDSA::SetStorageCube(int levels, TextureSizedFormat format, int width, int height)
{
	GL_CHECK(glBindTexture(GL_TexTarget[(int)_type], _id));
	GL_CHECK(glTexStorage2D(GL_TexTarget[(int)_type], levels, GL_TexFormatSized[(int)format], width, height));
	
	_width = width;
	_height = height;
	_mipLevels = levels;
	_sizedFormat = format;
}

void GLTexture_NoDSA::SetImage1D(int level, int width, TextureFormat format, TextureInternalType type, const void* data)
{
	GL_CHECK(glBindTexture(GL_TexTarget[(int)_type], _id));
	GL_CHECK(glTexSubImage1D(GL_TexTarget[(int)_type], level, 0, width, GL_TexFormat[(int)format], GL_TexType[(int)type], data));
	
	_format = format;
}

void GLTexture_NoDSA::SetImage2D(int level, int width, int height, TextureFormat format, TextureInternalType type, const void* data)
{
	GL_CHECK(glBindTexture(GL_TexTarget[(int)_type], _id));
	GL_CHECK(glTexSubImage2D(GL_TexTarget[(int)_type], level, 0, 0, width, height, GL_TexFormat[(int)format], GL_TexType[(int)type], data));
	
	_format = format;
}

void GLTexture_NoDSA::SetImage3D(int level, int width, int height, int depth, TextureFormat format, TextureInternalType type, const void* data)
{
	GL_CHECK(glBindTexture(GL_TexTarget[(int)_type], _id));
	GL_CHECK(glTexSubImage3D(GL_TexTarget[(int)_type], level, 0, 0, 0, width, height, depth, GL_TexFormat[(int)format], GL_TexType[(int)type], data));
	
	_format = format;
}

void GLTexture_NoDSA::SetMinFilter(TextureFilter filter)
{
	GL_CHECK(glBindTexture(GL_TexTarget[(int)_type], _id));
	GL_CHECK(glTexParameteri(GL_TexTarget[(int)_type], GL_TEXTURE_MIN_FILTER, GL_TexFilter[(int)filter]));
}

void GLTexture_NoDSA::SetMagFilter(TextureFilter filter)
{
	GL_CHECK(glBindTexture(GL_TexTarget[(int)_type], _id));
	GL_CHECK(glTexParameteri(GL_TexTarget[(int)_type], GL_TEXTURE_MAG_FILTER, GL_TexFilter[(int)filter]));
}

void GLTexture_NoDSA::SetAnisotropic(int aniso)
{
	GL_CHECK(glBindTexture(GL_TexTarget[(int)_type], _id));
	GL_CHECK(glTexParameteri(GL_TexTarget[(int)_type], GL_TEXTURE_MAX_ANISOTROPY_EXT, aniso));
}

void GLTexture_NoDSA::SetWrapS(TextureWrap wrap)
{
	GL_CHECK(glBindTexture(GL_TexTarget[(int)_type], _id));
	GL_CHECK(glTexParameteri(GL_TexTarget[(int)_type], GL_TEXTURE_WRAP_S, GL_TexWrap[(int)wrap]));
}

void GLTexture_NoDSA::SetWrapT(TextureWrap wrap)
{
	GL_CHECK(glBindTexture(GL_TexTarget[(int)_type], _id));
	GL_CHECK(glTexParameteri(GL_TexTarget[(int)_type], GL_TEXTURE_WRAP_T, GL_TexWrap[(int)wrap]));
}

void GLTexture_NoDSA::SetWrapR(TextureWrap wrap)
{
	GL_CHECK(glBindTexture(GL_TexTarget[(int)_type], _id));
	GL_CHECK(glTexParameteri(GL_TexTarget[(int)_type], GL_TEXTURE_WRAP_R, GL_TexWrap[(int)wrap]));
}

void GLTexture_NoDSA::Resize1D(int width)
{
	_Destroy();
	GL_CHECK(glGenTextures(1, &_id));
	SetStorage1D(_mipLevels, _sizedFormat, width);
}

void GLTexture_NoDSA::Resize2D(int width, int height)
{
	_Destroy();
	GL_CHECK(glGenTextures(1, &_id));

	if (_type == TextureType::Tex2D)
		SetStorage2D(_mipLevels, _sizedFormat, width, height);
	else
		SetStorage2DMS(_samples, width, height, _sizedFormat, _fixedLocations);
}

void GLTexture_NoDSA::Resize3D(int width, int height, int depth)
{
	_Destroy();
	GL_CHECK(glGenTextures(1, &_id));
	SetStorage3D(_mipLevels, _sizedFormat, width, height, depth);
}

void GLTexture_NoDSA::ResizeCubemap(int width, int height)
{
	_Destroy();
	GL_CHECK(glGenTextures(1, &_id));
	SetStorageCube(_mipLevels, _sizedFormat, width, height);
}

void GLTexture_NoDSA::GenerateMipmaps()
{
	GL_CHECK(glBindTexture(GL_TexTarget[(int)_type], _id));
	GL_CHECK(glGenerateMipmap(GL_TexTarget[(int)_type]));
}

GLTexture_NoDSA::~GLTexture_NoDSA()
{
	_Destroy();
}
