/* NekoEngine
 *
 * D3D11Texture.cpp
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

#include "D3D11Texture.h"
#include "D3D11Renderer.h"
#include <DirectXTex.h>

using namespace DirectX;

size_t D3D11_FormatSize[26] =
{
	1,
	2,
	3,
	4,
	2,
	2,
	8,
	8,
	4,
	8,
	12,
	16,
	2,
	4,
	4,
	4,
	4,
	4,
	8,
	16,
	16,
	1,
	1,
	1,
	1
};

DXGI_FORMAT D3D11_TexFormatSized[26] =
{
	DXGI_FORMAT_R8_UNORM,
	DXGI_FORMAT_R8G8_UNORM,
	DXGI_FORMAT_R8G8B8A8_UNORM,
	DXGI_FORMAT_R8G8B8A8_UNORM,
	DXGI_FORMAT_R16_FLOAT,
	DXGI_FORMAT_R16G16_FLOAT,
	DXGI_FORMAT_R16G16B16A16_FLOAT,
	DXGI_FORMAT_R16G16B16A16_FLOAT,
	DXGI_FORMAT_R32_FLOAT,
	DXGI_FORMAT_R32G32_FLOAT,
	DXGI_FORMAT_R32G32B32_FLOAT,
	DXGI_FORMAT_R32G32B32A32_FLOAT,
	DXGI_FORMAT_D16_UNORM,
	DXGI_FORMAT_D24_UNORM_S8_UINT,
	DXGI_FORMAT_D32_FLOAT,
	DXGI_FORMAT_D32_FLOAT,
	DXGI_FORMAT_D24_UNORM_S8_UINT,
	DXGI_FORMAT_D32_FLOAT_S8X24_UINT,
	DXGI_FORMAT_R8_UINT,
	DXGI_FORMAT_R8G8_UINT,
	DXGI_FORMAT_R8G8B8A8_UINT,
	DXGI_FORMAT_R8G8B8A8_UINT,
	DXGI_FORMAT_BC1_UNORM,
	DXGI_FORMAT_BC1_UNORM,
	DXGI_FORMAT_BC3_UNORM,
	DXGI_FORMAT_BC5_UNORM
};

D3D11_FILTER D3D11_TexFilter[4] =
{
	D3D11_FILTER_MIN_MAG_MIP_POINT,
	D3D11_FILTER_MIN_MAG_POINT_MIP_LINEAR,
	D3D11_FILTER_MIN_MAG_LINEAR_MIP_POINT,
	D3D11_FILTER_MIN_MAG_MIP_LINEAR
};

D3D11_TEXTURE_ADDRESS_MODE D3D11_TexWrap[4] =
{
	D3D11_TEXTURE_ADDRESS_WRAP,
	D3D11_TEXTURE_ADDRESS_MIRROR,
	D3D11_TEXTURE_ADDRESS_CLAMP,
	D3D11_TEXTURE_ADDRESS_BORDER
};

D3D11Texture::D3D11Texture(D3D11Context *ctx, TextureType type)
	: RTexture(type)
{
	_ctx = ctx;
	_data = nullptr;
	_usable = false;
	_samplerState = nullptr;
	_srv = nullptr;
	_texture = nullptr;

	_samplerDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
	_samplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
	_samplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
	_samplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
	_samplerDesc.MipLODBias = 0.0f;
	_samplerDesc.MaxAnisotropy = 1;
	_samplerDesc.ComparisonFunc = D3D11_COMPARISON_ALWAYS;
	_samplerDesc.BorderColor[0] = 0;
	_samplerDesc.BorderColor[1] = 0;
	_samplerDesc.BorderColor[2] = 0;
	_samplerDesc.BorderColor[3] = 0;
	_samplerDesc.MinLOD = 0;
	_samplerDesc.MaxLOD = D3D11_FLOAT32_MAX;
}

void D3D11Texture::GetImage(int level, TextureFormat format, TextureInternalType type, size_t size, void *buff)
{
	//
}

bool D3D11Texture::LoadFromFile(const char* file)
{
	HRESULT hr;
	TCHAR file_wchar[1024];
	const char *ext = strrchr(file, '.');
	size_t num;

	mbstowcs_s(&num, file_wchar, file, 1024);

	TexMetadata mdata;
	ScratchImage image;

	if (!strncmp(++ext, "tga", 3))
	{
		hr = GetMetadataFromTGAFile(file_wchar, mdata);
		if (FAILED(hr))
			return false;

		hr = LoadFromTGAFile(file_wchar, &mdata, image);
		if (FAILED(hr))
			return false;
	}
	else if (!strncmp(ext, "dds", 3))
	{
		hr = GetMetadataFromDDSFile(file_wchar, DDS_FLAGS_NONE, mdata);
		if (FAILED(hr))
			return false;

		hr = LoadFromDDSFile(file_wchar, DDS_FLAGS_NONE, &mdata, image);
		if (FAILED(hr))
			return false;
	}
	/*else if (!strncmp(ext, "wic", 3))
	{
		hr = GetMetadataFromWICFile(file_wchar, WIC_FLAGS_NONE, mdata);
		if (FAILED(hr))
			return false;

		hr = LoadFromWICFile(file_wchar, WIC_FLAGS_NONE, &mdata, image);
		if (FAILED(hr))
			return false;
	}*/

	if (_srv) _srv->Release();
	hr = CreateShaderResourceView(_ctx->device, image.GetImages(), image.GetImageCount(), mdata, &_srv);
	if (FAILED(hr))
		return false;

	_usable = true;

	return true;
}

bool D3D11Texture::LoadFromMemory(TextureFileFormat format, const uint8_t* mem, size_t size)
{
	HRESULT hr;

	TexMetadata mdata;
	ScratchImage image;

	if (format == TextureFileFormat::TGA)
	{
		hr = GetMetadataFromTGAMemory(mem, size, mdata);
		if (FAILED(hr))
			return false;

		hr = LoadFromTGAMemory(mem, size, &mdata, image);
		if (FAILED(hr))
			return false;
	}
	else if (format == TextureFileFormat::DDS)
	{
		hr = GetMetadataFromDDSMemory(mem, size, DDS_FLAGS_NONE, mdata);
		if (FAILED(hr))
			return false;

		hr = LoadFromDDSMemory(mem, size, DDS_FLAGS_NONE, &mdata, image);
		if (FAILED(hr))
			return false;
	}

	if (_srv) _srv->Release();
	hr = CreateShaderResourceView(_ctx->device, image.GetImages(), image.GetImageCount(), mdata, &_srv);
	if (FAILED(hr))
		return false;

	_usable = true;

	return true;
}

void D3D11Texture::CreateTexture()
{
	HRESULT hr;
	D3D11_SUBRESOURCE_DATA *data = _data ? _data->pSysMem ? _data : nullptr : nullptr;

	if (_texture) ((ID3D11Resource *)_texture)->Release();
	if (_srv) _srv->Release();

	switch (_type)
	{
		case TextureType::Tex1D:
		{
			
			hr = _ctx->device->CreateTexture1D(&desc_1d, data, (ID3D11Texture1D**)&_texture);
			hr = _ctx->device->CreateShaderResourceView((ID3D11Texture1D*)_texture, nullptr, &_srv);
		}
		break;
		case TextureType::Tex2D:
		{
			D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc;
			D3D11_SHADER_RESOURCE_VIEW_DESC *desc = nullptr;

			if (desc_2d.Format == DXGI_FORMAT_D16_UNORM)
			{
				srvDesc.Format = DXGI_FORMAT_R32_FLOAT;
				desc = &srvDesc;
				desc_2d.BindFlags = D3D11_BIND_DEPTH_STENCIL | D3D11_BIND_SHADER_RESOURCE;
			}
			else if (desc_2d.Format == DXGI_FORMAT_D24_UNORM_S8_UINT)
			{
				srvDesc.Format = DXGI_FORMAT_R24_UNORM_X8_TYPELESS;
				desc = &srvDesc;
				desc_2d.Format = DXGI_FORMAT_R24G8_TYPELESS;
				desc_2d.BindFlags = D3D11_BIND_DEPTH_STENCIL | D3D11_BIND_SHADER_RESOURCE;
			}
			else if (desc_2d.Format == DXGI_FORMAT_D32_FLOAT)
			{
				srvDesc.Format = DXGI_FORMAT_R32_FLOAT;
				desc = &srvDesc;
				desc_2d.BindFlags = D3D11_BIND_DEPTH_STENCIL | D3D11_BIND_SHADER_RESOURCE;
			}
			else if (desc_2d.Format == DXGI_FORMAT_D32_FLOAT_S8X24_UINT)
			{
				srvDesc.Format = DXGI_FORMAT_R32_FLOAT;
				desc = &srvDesc;
				desc_2d.BindFlags = D3D11_BIND_DEPTH_STENCIL | D3D11_BIND_SHADER_RESOURCE;
			}

			srvDesc.ViewDimension = _multisampled ? D3D11_SRV_DIMENSION_TEXTURE2DMS : D3D11_SRV_DIMENSION_TEXTURE2D;
			srvDesc.Texture2D.MostDetailedMip = 0;
			srvDesc.Texture2D.MipLevels = 1;

			hr = _ctx->device->CreateTexture2D(&desc_2d, data, (ID3D11Texture2D**)&_texture);
			if (FAILED(hr))
			{
				OutputDebugStringA("fk");
			}

			hr = _ctx->device->CreateShaderResourceView((ID3D11Texture2D*)_texture, desc, &_srv);
			if (FAILED(hr))
			{
				OutputDebugStringA("fk");
			}
		}
		break;
		case TextureType::Tex3D:
		{
			hr = _ctx->device->CreateTexture3D(&desc_3d, data, (ID3D11Texture3D**)&_texture);
			hr = _ctx->device->CreateShaderResourceView((ID3D11Texture3D*)_texture, nullptr, &_srv);
		}
		break;
		case TextureType::TexCubemap:
		{
			hr = _ctx->device->CreateTexture2D(&desc_2d, data, (ID3D11Texture2D**)&_texture);
			hr = _ctx->device->CreateShaderResourceView((ID3D11Texture2D*)_texture, nullptr, &_srv);
		}
		break;
		case TextureType::Tex2DMultisample:
		{
			hr = _ctx->device->CreateTexture2D(&desc_2dms, data, (ID3D11Texture2D**)&_texture);
			if (FAILED(hr))
			{
				OutputDebugStringA("fk");
			}

			hr = _ctx->device->CreateShaderResourceView((ID3D11Texture2D*)_texture, nullptr, &_srv);
			if (FAILED(hr))
			{
				OutputDebugStringA("fk");
			}
		}
	}

	_usable = true;
}

void D3D11Texture::SetStorage1D(int levels, TextureSizedFormat format, int width)
{
	_DestroyData();

	_dxgiFormat = D3D11_TexFormatSized[(int)format];

	ZeroMemory(&desc_1d, sizeof(desc_1d));
	desc_1d.ArraySize = 1;
	desc_1d.Width = width;
	desc_1d.MipLevels = levels;
	desc_1d.BindFlags = D3D11_BIND_SHADER_RESOURCE;
	desc_1d.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	desc_1d.Usage = D3D11_USAGE_IMMUTABLE;
	desc_1d.MiscFlags = 0;
	desc_1d.Format = _dxgiFormat;

	_levels = levels;
	_data = (D3D11_SUBRESOURCE_DATA*)calloc(_levels, sizeof(D3D11_SUBRESOURCE_DATA));

	for (int i = 0; i < levels; ++i)
	{
		int w = i == 0 ? width : width / (2 * i);

		uint64_t size = w * D3D11_FormatSize[(int)format];
		_data[i].SysMemPitch = (UINT)(w * D3D11_FormatSize[(int)format]);
		_data[i].pSysMem = calloc(1, size);
	}

	_CreateSampler();
}

void D3D11Texture::SetStorage2D(int levels, TextureSizedFormat format, int width, int height)
{
	_DestroyData();

	_dxgiFormat = D3D11_TexFormatSized[(int)format];

	ZeroMemory(&desc_2d, sizeof(desc_2d));
	desc_2d.ArraySize = 1;
	desc_2d.Width = width;
	desc_2d.Height = height;
	desc_2d.MipLevels = levels;
	desc_2d.BindFlags = D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_RENDER_TARGET;
	desc_2d.Usage = D3D11_USAGE_DEFAULT;
	desc_2d.MiscFlags = 0;
	desc_2d.Format = _dxgiFormat;
	desc_2d.SampleDesc.Count = 1;

	_levels = levels;
	_data = (D3D11_SUBRESOURCE_DATA*)calloc(_levels, sizeof(D3D11_SUBRESOURCE_DATA));

	for (int i = 0; i < levels; ++i)
	{
		int w = i == 0 ? width : width / (2 * i);
		int h = i == 0 ? height : height / (2 * i);
		
		uint64_t size = w * h * D3D11_FormatSize[(int)format];
		_data[i].SysMemPitch = (UINT)(w * D3D11_FormatSize[(int)format]);
		_data[i].pSysMem = calloc(1, size);
	}

	_CreateSampler();
}

void D3D11Texture::SetStorage3D(int levels, TextureSizedFormat format, int width, int height, int depth)
{
	_DestroyData();

	_dxgiFormat = D3D11_TexFormatSized[(int)format];

	ZeroMemory(&desc_3d, sizeof(desc_3d));
	desc_3d.Width = width;
	desc_3d.Height = height;
	desc_3d.Depth = depth;
	desc_3d.MipLevels = levels;
	desc_3d.BindFlags = D3D11_BIND_SHADER_RESOURCE;
	desc_3d.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	desc_3d.Usage = D3D11_USAGE_IMMUTABLE;
	desc_3d.MiscFlags = 0;
	desc_3d.Format = _dxgiFormat;

	_levels = levels;
	_data = (D3D11_SUBRESOURCE_DATA*)calloc(_levels, sizeof(D3D11_SUBRESOURCE_DATA));

	for (int i = 0; i < levels; ++i)
	{
		int w = i == 0 ? width : width / (2 * i);
		int h = i == 0 ? height : height / (2 * i);
		int d = i == 0 ? depth : depth / (2 * i);

		uint64_t size = w * h * d * D3D11_FormatSize[(int)format];
		_data[i].SysMemPitch = (UINT)(w * D3D11_FormatSize[(int)format]);
		_data[i].pSysMem = calloc(1, size);
	}

	_CreateSampler();
}

void D3D11Texture::SetStorage2DMS(int samples, int width, int height, TextureSizedFormat format, bool fixedSampleLocations)
{
	_DestroyData();
	_dxgiFormat = D3D11_TexFormatSized[(int)format];

	ZeroMemory(&desc_2dms, sizeof(desc_2dms));
	desc_2dms.ArraySize = 1;
	desc_2dms.Width = width;
	desc_2dms.Height = height;
	desc_2dms.MipLevels = 1;
	desc_2dms.BindFlags = D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_RENDER_TARGET;
	//desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	desc_2dms.Usage = D3D11_USAGE_DEFAULT;
	desc_2dms.MiscFlags = 0;
	desc_2dms.Format = _dxgiFormat;
	desc_2dms.SampleDesc.Count = samples;
	//desc_2dms.SampleDesc.Quality

	_multisampled = true;

	_CreateSampler();
}

void D3D11Texture::SetStorageCube(int levels, TextureSizedFormat format, int width, int height)
{
	_DestroyData();

	_dxgiFormat = D3D11_TexFormatSized[(int)format];

	ZeroMemory(&desc_2d, sizeof(desc_2d));
	desc_2d.ArraySize = 6;
	desc_2d.Width = width;
	desc_2d.Height = height;
	desc_2d.MipLevels = levels;
	desc_2d.BindFlags = D3D11_BIND_SHADER_RESOURCE;
	desc_2d.CPUAccessFlags = 0;
	desc_2d.Usage = D3D11_USAGE_IMMUTABLE;
	desc_2d.MiscFlags = D3D11_RESOURCE_MISC_TEXTURECUBE;
	desc_2d.Format = _dxgiFormat;
	desc_2d.SampleDesc.Count = 1;

	_levels = levels * 6;
	_data = (D3D11_SUBRESOURCE_DATA*)calloc(_levels, sizeof(D3D11_SUBRESOURCE_DATA));

	for (int i = 0; i < levels; ++i)
	{
		int w = i == 0 ? width : width / (2 * i);
		int h = i == 0 ? height : height / (2 * i);
		uint64_t size = w * h * D3D11_FormatSize[(int)format];
		UINT pitch = (UINT)(w * D3D11_FormatSize[(int)format]);

		for (int j = 0; j < 6; ++j)
		{
			_data[i * 6 + j].SysMemPitch = pitch;
			_data[i * 6 + j].pSysMem = calloc(1, size);
		}
	}

	_CreateSampler();
}

void D3D11Texture::SetImage1D(int level, int width, TextureFormat format, TextureInternalType type, const void* data)
{
	if (level > _levels)
		return;

	uint64_t size = width * D3D11_FormatSize[(int)format];
	memcpy((void *)_data[level].pSysMem, data, size);
}

void D3D11Texture::SetImage2D(int level, int width, int height, TextureFormat format, TextureInternalType type, const void* data)
{
	if (level > _levels)
		return;

	uint64_t size = width * height * D3D11_FormatSize[(int)format];
	memcpy((void *)_data[level].pSysMem, data, size);
}

void D3D11Texture::SetImage3D(int level, int width, int height, int depth, TextureFormat format, TextureInternalType type, const void* data)
{
	if (level > _levels)
		return;

	uint64_t size = width * height * depth * D3D11_FormatSize[(int)format];
	memcpy((void *)_data[level].pSysMem, data, size);
}

void D3D11Texture::SetSubImage1D(int level, int x, int width, TextureFormat format, TextureInternalType type, const void* data)
{
	if (level > _levels)
		return;

	uint64_t size = width * D3D11_FormatSize[(int)format];
	memcpy((uint8_t *)_data[level].pSysMem + (x * D3D11_FormatSize[(int)format]), (uint8_t *)data, size);
}

void D3D11Texture::SetSubImage2D(int level, int x, int y, int width, int height, TextureFormat format, TextureInternalType type, const void* data)
{
	if (level > _levels)
		return;

	for (int i = 0; i < height; ++i)
	{
		int yOffset = _data[level].SysMemPitch * (i + y);
		int xOffset = (int)(x * D3D11_FormatSize[(int)format]);
		int offset = xOffset + yOffset;

		int size = (int)(width * D3D11_FormatSize[(int)format]);
		int dataOffset = size * i;

		memcpy((uint8_t *)_data[level].pSysMem + offset, (uint8_t *)data + dataOffset, size);
	}
}

void D3D11Texture::SetSubImage3D(int level, int x, int y, int z, int width, int height, int depth, TextureFormat format, TextureInternalType type, const void* data)
{
	/*if (level > _levels)
		return;

	for (int i = 0; i < height; ++i)
	{
		int yOffset = _data[level].SysMemPitch * (i + y);
		int xOffset = x * D3D11_FormatSize[(int)format];
		int offset = xOffset + yOffset;

		uint16_t size = width * D3D11_FormatSize[(int)format];
		int dataOffset = size * i;

		memcpy((uint8_t *)_data[level].pSysMem + offset, (uint8_t *)data + dataOffset, size);
	}*/
}

void D3D11Texture::SetImageCube(int level, int width, int height, TextureFormat format, TextureInternalType type,
	const void *posX, const void *negX, const void *posY, const void *negY, const void *posZ, const void *negZ)
{
	int cubelvl = level * 6;

	SetImage2D(cubelvl, width, height, format, type, posX);
	SetImage2D(cubelvl + 1, width, height, format, type, negX);
	SetImage2D(cubelvl + 2, width, height, format, type, posY);
	SetImage2D(cubelvl + 3, width, height, format, type, negY);
	SetImage2D(cubelvl + 4, width, height, format, type, posZ);
	SetImage2D(cubelvl + 5, width, height, format, type, negZ);

	CreateTexture();
}

void D3D11Texture::SetImageCubeFace(CubeFace face, int level, int width, int height, TextureFormat format, TextureInternalType type, const void *data)
{
	SetImage2D(level + (int)face, width, height, format, type, data);
}

void D3D11Texture::SetMinFilter(TextureFilter filter) { _samplerDesc.Filter = D3D11_TexFilter[(int)filter]; _CreateSampler(); }
void D3D11Texture::SetMagFilter(TextureFilter filter) { _samplerDesc.Filter = D3D11_TexFilter[(int)filter]; _CreateSampler(); }
void D3D11Texture::SetWrapS(TextureWrap wrap) { _samplerDesc.AddressU = D3D11_TexWrap[(int)wrap]; _CreateSampler(); }
void D3D11Texture::SetWrapT(TextureWrap wrap) { _samplerDesc.AddressV = D3D11_TexWrap[(int)wrap]; _CreateSampler(); }
void D3D11Texture::SetWrapR(TextureWrap wrap) { _samplerDesc.AddressW = D3D11_TexWrap[(int)wrap]; _CreateSampler(); }
void D3D11Texture::SetAnisotropic(int aniso) { _samplerDesc.Filter = D3D11_FILTER_ANISOTROPIC; _samplerDesc.MaxAnisotropy = aniso; _CreateSampler(); }

void D3D11Texture::Resize1D(int width) { }
void D3D11Texture::Resize2D(int width, int height) { }
void D3D11Texture::Resize3D(int width, int height, int depth) { }
void D3D11Texture::ResizeCubemap(int width, int height) { }

void D3D11Texture::GenerateMipmaps()
{
	//_ctx->deviceContext->GenerateMips(_srv);
}

void D3D11Texture::_CreateSampler()
{
	if (_samplerState) _samplerState->Release();
	_ctx->device->CreateSamplerState(&_samplerDesc, &_samplerState);
}

void D3D11Texture::_DestroyData()
{
	if (!_data)
		return;

	int levels = _levels;
	if (_type == TextureType::TexCubemap)
		levels *= 6;

	for (int n = 0; n < _levels; ++n)
		free((void *)_data[n].pSysMem);
	free(_data);

	_data = nullptr;
}

D3D11Texture::~D3D11Texture()
{
	if (_texture) ((ID3D11Resource *)_texture)->Release();
	if (_srv) _srv->Release();
	if (_samplerState) _samplerState->Release();
	_DestroyData();
}