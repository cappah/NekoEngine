/* NekoEngine
 *
 * GLTexture.h
 * Author: Alexandru Naiman
 *
 * OpenGL 4 Renderer Implementation
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

#pragma once

#include <Renderer/RTexture.h>

#ifdef __APPLE__
	#include <OpenGL/gl3.h>
	#include <OpenGL/gl3ext.h>
#else
	#include "glad.h"
#endif

#include "Loaders/tga.h"
#include "Loaders/nv_dds.h"

class GLTexture :
	public RTexture	
{
public:
	GLTexture(TextureType type, bool create = true);

	void Bind();
	
	GLuint GetId() { return _id; }
	GLuint64 GetHandle() { return _handle; }
	bool IsResident() { return _resident; }
	void MakeResident();

	virtual void GetImage(int level, TextureFormat format, TextureInternalType type, size_t size, void *buff) override;

	virtual bool LoadFromFile(const char* file) override;
	virtual bool LoadFromMemory(TextureFileFormat format, const uint8_t* mem, size_t size) override;

	virtual void SetStorage1D(int levels, TextureSizedFormat format, int width) override;
	virtual void SetStorage2D(int levels, TextureSizedFormat format, int width, int height) override;
	virtual void SetStorage3D(int levels, TextureSizedFormat format, int width, int height, int depth) override;
	virtual void SetStorage2DMS(int samples, int width, int height, TextureSizedFormat format, bool fixedSampleLocations) override;
	virtual void SetStorageCube(int levels, TextureSizedFormat format, int width, int height) override;

	virtual void SetImage1D(int level, int width, TextureFormat format, TextureInternalType type, const void* data) override;
	virtual void SetImage2D(int level, int width, int height, TextureFormat format, TextureInternalType type, const void* data) override;
	virtual void SetImage3D(int level, int width, int height, int depth, TextureFormat format, TextureInternalType type, const void* data) override;
	virtual void SetSubImage1D(int level, int x, int width, TextureFormat format, TextureInternalType type, const void* data) override;
	virtual void SetSubImage2D(int level, int x, int y, int width, int height, TextureFormat format, TextureInternalType type, const void* data) override;
	virtual void SetSubImage3D(int level, int x, int y, int z, int width, int height, int depth, TextureFormat format, TextureInternalType type, const void* data) override;
	virtual void SetImageCube(int level, int width, int height, TextureFormat format, TextureInternalType type,
		const void *posX, const void *negX, const void *posY, const void *negY, const void *posZ, const void *negZ) override;
	virtual void SetImageCubeFace(CubeFace face, int level, int width, int height, TextureFormat format, TextureInternalType type, const void *data) override;

	virtual void SetCompressedImage1D(int level, int width, CompressedTextureFormat format, int size, const void* data) override;
	virtual void SetCompressedImage2D(int level, int width, int height, CompressedTextureFormat format, int size, const void* data) override;
	virtual void SetCompressedImage3D(int level, int width, int height, int depth, CompressedTextureFormat format, int size, const void* data) override;
	virtual void SetCompressedImageCube(int level, int width, int height, CompressedTextureFormat format, int size,
		const void *posX, const void *negX, const void *posY, const void *negY, const void *posZ, const void *negZ) override;
	virtual void SetCompressedImageCubeFace(CubeFace face, int level, int width, int height, CompressedTextureFormat format, int size, const void *data) override;

	virtual void SetMinFilter(TextureFilter filter) override;
	virtual void SetMagFilter(TextureFilter filter) override;
	virtual void SetAnisotropic(int aniso) override;
	virtual void SetWrapS(TextureWrap wrap) override;
	virtual void SetWrapT(TextureWrap wrap) override;
	virtual void SetWrapR(TextureWrap wrap) override;

	virtual void Resize1D(int width) override;
	virtual void Resize2D(int width, int height) override;
	virtual void Resize3D(int width, int height, int depth) override;
	virtual void ResizeCubemap(int width, int height) override;

	virtual void GenerateMipmaps() override;

	virtual ~GLTexture();

protected:
	GLuint _id;
	GLuint64 _handle;
	bool _resident, _fixedLocations;
	TextureSizedFormat _sizedFormat;

	void _Destroy();
	bool _LoadTGATexture(char *tga, int bpp);
	bool _LoadDDSTexture(class nv_dds::CDDSImage& image);
};

class GLTexture_NoDSA :
	public GLTexture
{
public:
	GLTexture_NoDSA(TextureType type);
	
	virtual void GetImage(int level, TextureFormat format, TextureInternalType type, size_t size, void *buff) override;

	virtual void SetStorage1D(int levels, TextureSizedFormat format, int width) override;
	virtual void SetStorage2D(int levels, TextureSizedFormat format, int width, int height) override;
	virtual void SetStorage3D(int levels, TextureSizedFormat format, int width, int height, int depth) override;
	virtual void SetStorage2DMS(int samples, int width, int height, TextureSizedFormat format, bool fixedSampleLocations) override;
	virtual void SetStorageCube(int levels, TextureSizedFormat format, int width, int height) override;
	
	virtual void SetImage1D(int level, int width, TextureFormat format, TextureInternalType type, const void* data) override;
	virtual void SetImage2D(int level, int width, int height, TextureFormat format, TextureInternalType type, const void* data) override;
	virtual void SetImage3D(int level, int width, int height, int depth, TextureFormat format, TextureInternalType type, const void* data) override;
	virtual void SetSubImage1D(int level, int x, int width, TextureFormat format, TextureInternalType type, const void* data) override;
	virtual void SetSubImage2D(int level, int x, int y, int width, int height, TextureFormat format, TextureInternalType type, const void* data) override;
	virtual void SetSubImage3D(int level, int x, int y, int z, int width, int height, int depth, TextureFormat format, TextureInternalType type, const void* data) override;
	
	virtual void SetMinFilter(TextureFilter filter) override;
	virtual void SetMagFilter(TextureFilter filter) override;
	virtual void SetAnisotropic(int aniso) override;
	virtual void SetWrapS(TextureWrap wrap) override;
	virtual void SetWrapT(TextureWrap wrap) override;
	virtual void SetWrapR(TextureWrap wrap) override;
	
	virtual void Resize1D(int width) override;
	virtual void Resize2D(int width, int height) override;
	virtual void Resize3D(int width, int height, int depth) override;
	virtual void ResizeCubemap(int width, int height) override;
	
	virtual void GenerateMipmaps() override;
	
	virtual ~GLTexture_NoDSA();
};
