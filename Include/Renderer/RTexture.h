/* Neko Engine
 *
 * RTexture.h
 * Author: Alexandru Naiman
 *
 * Rendering API abstraction
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

#pragma once

#include <stdint.h>
#include <stddef.h>

enum class TextureFileFormat : uint8_t;

enum class TextureType : uint8_t
{
	Tex1D = 0,
	Tex2D = 1,
	Tex3D = 2,
	TexCubemap = 3,
	Tex2DMultisample
};

enum class TextureFilter : uint8_t
{
	Nearest = 0,
	Linear = 1,
	Bilinear = 2,
	Trilinear = 3
};

enum class TextureWrap : uint8_t
{
	Repeat = 0,
	MirroredRepeat = 1,
	ClampToEdge,
	ClampToBorder
};

enum class TextureSizedFormat : uint8_t
{
	R_8U = 0,
	RG_8U = 1,
	RGB_8U,
	RGBA_8U,
	R_16F,
	RG_16F,
	RGB_16F,
	RGBA_16F,
	R_32F,
	RG_32F,
	RGB_32F,
	RGBA_32F,
	DEPTH_16,
	DEPTH_24,
	DEPTH_32,
	DEPTH_32F,
	DEPTH24_STENCIL8,
	DEPTH32F_STENCIL8,
	R_8UI,
	RG_8UI,
	RGB_8UI,
	RGBA_8UI,
	RGB_S3TC_DXT1,
	RGBA_S3TC_DXT1,
	RGBA_S3TC_DXT3,
	RGBA_S3TC_DXT5
};

enum class TextureFormat : uint8_t
{
	RED = 0,
	RG = 1,
	RGB,
	RGBA,
	DEPTH,
	DEPTH_STENCIL,
	RED_INT,
	RG_INT,
	RGB_INT,
	RGBA_INT
};

enum class CompressedTextureFormat : uint8_t
{	
	RGB_S3TC_DXT1 = 0,
	RGBA_S3TC_DXT1 = 1,
	RGBA_S3TC_DXT3,
	RGBA_S3TC_DXT5
};

enum class TextureInternalType : uint8_t
{
	UnsignedByte = 0,
	UnsignedInt = 1,
	Float,
	HalfFloat,
	UnsignedInt_24_8
};

enum class CubeFace : uint8_t
{
	PosX = 0,
	NegX = 1,
	PosY,
	NegY,
	PosZ,
	NegZ
};

class RTexture
{
public:
	
	/**
	 * Create a texture object.
	 * You must call one of the SetStorage* functions to initialize the storage for this texture
	 */
	RTexture(TextureType type) : 
		_type(type),
		_format(TextureFormat::RGB),
		_compressedFormat(CompressedTextureFormat::RGB_S3TC_DXT1),
		_mipLevels(1),
		_samples(1),
		_width(0),
		_height(0),
		_depth(0),
		_compressed(false)
	{ }

	virtual TextureType GetType() { return _type; }
	virtual TextureFormat GetFormat() { return _format; }
	virtual CompressedTextureFormat GetCompressedFormat() { return _compressedFormat; }
	virtual int GetMipLevels() { return _mipLevels; }
	virtual int GetSamples() { return _samples; }
	virtual int GetWidth() { return _width; }
	virtual int GetHeight() { return _height; }
	virtual int GetDepth() { return _depth; }
	virtual bool IsCompressed() { return _compressed; }

	virtual bool LoadFromFile(const char* file) = 0;
	virtual bool LoadFromMemory(TextureFileFormat format, const uint8_t* mem, size_t size) = 0;

	/**
	 * Initialize storage for a one-dimensional texture
	 */
	virtual void SetStorage1D(int levels, TextureSizedFormat format, int width) = 0;
	
	/**
	 * Initialize storage for a two-dimensional texture
	 */
	virtual void SetStorage2D(int levels, TextureSizedFormat format, int width, int height) = 0;
	
	/**
	 * Initialize storage for a multisampled two-dimensional texture
	 */
	virtual void SetStorage2DMS(int samples, int width, int height, TextureSizedFormat format, bool fixedSampleLocations) = 0;
	
	/**
	 * Initialize storage for a three-dimensional texture
	 */
	virtual void SetStorage3D(int levels, TextureSizedFormat format, int width, int height, int depth) = 0;

	/**
	 * Initialize storage for a cubemap texture
	 */
	virtual void SetStorageCube(int levels, TextureSizedFormat format, int width, int height) = 0;

	/**
	 * Set image data for a one-dimensional texture
	 */
	virtual void SetImage1D(int level, int width, TextureFormat format, TextureInternalType type, const void* data) = 0;
	
	/**
	 * Set image data for a two-dimensional texture
	 */
	virtual void SetImage2D(int level, int width, int height, TextureFormat format, TextureInternalType type, const void* data) = 0;
	
	/**
	 * Set image data for a three-dimensional texture
	 */
	virtual void SetImage3D(int level, int width, int height, int depth, TextureFormat format, TextureInternalType type, const void* data) = 0;

	/**
	 * Set image data for a cubemap texture
	 */
	virtual void SetImageCube(int level, int width, int height, TextureFormat format, TextureInternalType type,
		const void *posX, const void *negX, const void *posY, const void *negY, const void *posZ, const void *negZ) = 0;

	/**
 	 * Set image data for a cubemap face
	 */
	virtual void SetImageCubeFace(CubeFace face, int level, int width, int height, TextureFormat format, TextureInternalType type, const void *data) = 0;

	/**
	 * Set image data for a one-dimensional texture in a compressed format
	 */
	virtual void SetCompressedImage1D(int level, int width, CompressedTextureFormat format, int size, const void* data) = 0;
	
	/**
	 * Set image data for a two-dimensional texture in a compressed format
	 */
	virtual void SetCompressedImage2D(int level, int width, int height, CompressedTextureFormat format, int size, const void* data) = 0;
	
	/**
	 * Set image data for a three-dimensional texture in a compressed format
	 */
	virtual void SetCompressedImage3D(int level, int width, int height, int depth, CompressedTextureFormat format, int size, const void* data) = 0;

	/**
	 * Set image data for a cubemap texture in a compressed format
	 */
	virtual void SetCompressedImageCube(int level, int width, int height, CompressedTextureFormat format, int size,
		const void *posX, const void *negX, const void *posY, const void *negY, const void *posZ, const void *negZ) = 0;

	/**
 	 * Set image data for a cubemap face in a compressed format
	 */
	virtual void SetCompressedImageCubeFace(CubeFace face, int level, int width, int height, CompressedTextureFormat format, int size, const void *data) = 0;

	/**
	 * Set the minification filtering
	 */
	virtual void SetMinFilter(TextureFilter filter) = 0;
	
	/**
	 * Set the magnification filtering
	 */
	virtual void SetMagFilter(TextureFilter filter) = 0;

	/**
	 * Set wrapping
	 */
	virtual void SetWrapS(TextureWrap wrap) = 0;

	/**
	* Set wrapping
	*/
	virtual void SetWrapT(TextureWrap wrap) = 0;

	/**
	* Set wrapping
	*/
	virtual void SetWrapR(TextureWrap wrap) = 0;
	
	/**
	 * Enable anisotropic filtering if the hardware supports it
	 */
	virtual void SetAnisotropic(int aniso) = 0;

	/**
	 * Resize texture.
	 * This operation will DESTROY ALL data.
	 */
	virtual void Resize1D(int width) = 0;
	
	/**
	 * Resize texture.
	 * This operation will DESTROY ALL data.
	 */
	virtual void Resize2D(int width, int height) = 0;

	/**
	 * Resize texture.
	 * This operation will DESTROY ALL data.
	 */
	virtual void Resize3D(int width, int height, int depth) = 0;

	/**
	 * Resize texture.
	 * This operation will DESTROY ALL data.
	 */
	virtual void ResizeCubemap(int width, int height) = 0;

	/**
	 * Automatically generate mipmap levels
	 * Must have storage allocated for all levels
	 */
	virtual void GenerateMipmaps() = 0;

	/**
	 * Release resources
	 */
	virtual ~RTexture() { };

protected:
	TextureType _type;
	TextureFormat _format;
	CompressedTextureFormat _compressedFormat;
	bool _compressed;
	int _mipLevels, _samples, _width, _height, _depth;
};

