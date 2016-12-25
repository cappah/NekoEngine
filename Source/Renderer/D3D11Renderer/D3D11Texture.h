/* NekoEngine
 *
 * D3D11Texture.h
 * Author: Alexandru Naiman
 *
 * DirectX 11 Renderer Implementation
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
#include "D3D11Context.h"

#include <Windows.h>
#include <windowsx.h>
#include <d3d11.h>
#include <d3d11_1.h>
#include <d3d11_2.h>

class D3D11Texture :
	public RTexture
{
public:
	D3D11Texture(D3D11Context *ctx, TextureType type);

	int Samples() { return desc_2dms.SampleDesc.Count; }
	bool IsUsable() { return _usable; }
	bool IsMultisampled() { return _multisampled; }
	void CreateTexture();
	DXGI_FORMAT GetDXGIFormat() { return _dxgiFormat; }
	ID3D11Texture2D *GetTexture2D() { return (ID3D11Texture2D *)_texture; }
	ID3D11ShaderResourceView *GetSRV() { return _srv; }
	ID3D11ShaderResourceView * const *GetPPSRV() { return &_srv; }
	ID3D11SamplerState * const *GetPPSS() { return &_samplerState; }

	virtual void GetImage(int level, TextureFormat format, TextureInternalType type, size_t size, void *buff) override;

	bool LoadFromFile(const char* file) override;
	bool LoadFromMemory(TextureFileFormat format, const uint8_t* mem, size_t size) override;

	virtual void SkipMipLevels(int n) override { _skipMipLevels = n; }

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

	virtual void SetMinFilter(TextureFilter filter) override;
	virtual void SetMagFilter(TextureFilter filter) override;
	virtual void SetWrapS(TextureWrap wrap) override;
	virtual void SetWrapT(TextureWrap wrap) override;
	virtual void SetWrapR(TextureWrap wrap) override;
	virtual void SetAnisotropic(int aniso) override;

	virtual void Resize1D(int width) override;
	virtual void Resize2D(int width, int height) override;
	virtual void Resize3D(int width, int height, int depth) override;
	virtual void ResizeCubemap(int width, int height) override;

	virtual void GenerateMipmaps() override;

	virtual ~D3D11Texture();

private:
	D3D11Context *_ctx;
	ID3D11ShaderResourceView *_srv;
	D3D11_SAMPLER_DESC _samplerDesc;
	ID3D11SamplerState *_samplerState;
	D3D11_SUBRESOURCE_DATA *_data;
	D3D11_TEXTURE1D_DESC desc_1d;
	D3D11_TEXTURE2D_DESC desc_2d;
	D3D11_TEXTURE3D_DESC desc_3d;
	D3D11_TEXTURE2D_DESC desc_2dms;
	DXGI_FORMAT _dxgiFormat;
	void *_texture;
	int _levels;
	bool _usable, _depth, _multisampled;
	int _skipMipLevels;

	void _CreateSampler();
	void _DestroyData();
};