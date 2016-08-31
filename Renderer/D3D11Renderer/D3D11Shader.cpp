/* NekoEngine
 *
 * D3D11Shader.cpp
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

#include "D3D11Shader.h"
#include "D3D11Renderer.h"
#include "D3D11Buffer.h"
#include "D3D11ArrayBuffer.h"

#include <d3dcompiler.h>

#include <string>
#include <fstream>

#define D3D_COMPILE_STANDARD_FILE_INCLUDE ((ID3DInclude*)(UINT_PTR)1)

#pragma comment (lib, "d3dcompiler.lib")

using namespace std;

DXGI_FORMAT D3D11_BufferDataType[40] =
{
	DXGI_FORMAT_R8_SINT,
	DXGI_FORMAT_R8G8_SINT,
	DXGI_FORMAT_R8G8B8A8_SINT,
	DXGI_FORMAT_R8G8B8A8_SINT,
	DXGI_FORMAT_R8_UINT,
	DXGI_FORMAT_R8G8_UINT,
	DXGI_FORMAT_R8G8B8A8_UINT,
	DXGI_FORMAT_R8G8B8A8_UINT,
	DXGI_FORMAT_R16_SINT,
	DXGI_FORMAT_R16G16_SINT,
	DXGI_FORMAT_R16G16B16A16_SINT,
	DXGI_FORMAT_R16G16B16A16_SINT,
	DXGI_FORMAT_R16_UINT,
	DXGI_FORMAT_R16G16_UINT,
	DXGI_FORMAT_R16G16B16A16_UINT,
	DXGI_FORMAT_R16G16B16A16_UINT,
	DXGI_FORMAT_R32_SINT,
	DXGI_FORMAT_R32G32_SINT,
	DXGI_FORMAT_R32G32B32_SINT,
	DXGI_FORMAT_R32G32B32A32_SINT,
	DXGI_FORMAT_R32_UINT,
	DXGI_FORMAT_R32G32_UINT,
	DXGI_FORMAT_R32G32B32_UINT,
	DXGI_FORMAT_R32G32B32A32_UINT,
	DXGI_FORMAT_R16_FLOAT,
	DXGI_FORMAT_R16G16_FLOAT,
	DXGI_FORMAT_R16G16B16A16_FLOAT,
	DXGI_FORMAT_R16G16B16A16_FLOAT,
	DXGI_FORMAT_R32_FLOAT,
	DXGI_FORMAT_R32G32_FLOAT,
	DXGI_FORMAT_R32G32B32_FLOAT,
	DXGI_FORMAT_R32G32B32A32_FLOAT,
	DXGI_FORMAT_R32_FLOAT,
	DXGI_FORMAT_R32G32_FLOAT,
	DXGI_FORMAT_R32G32B32_FLOAT,
	DXGI_FORMAT_R32G32B32A32_FLOAT,
	DXGI_FORMAT_R32_FLOAT,
	DXGI_FORMAT_R32G32_FLOAT,
	DXGI_FORMAT_R32G32B32_FLOAT,
	DXGI_FORMAT_R32G32B32A32_FLOAT
};

D3D11Shader::D3D11Shader(D3D11Context *ctx)
	: RShader()
{
	_ctx = ctx;

	_vs = nullptr;
	_ps = nullptr;
	_gs = nullptr;
	_hs = nullptr;
	_ds = nullptr;
	_cs = nullptr;

	_d3dVsBlob = nullptr;
	_d3dPsBlob = nullptr;
	_d3dGsBlob = nullptr;
	_d3dCsBlob = nullptr;
	_d3dHsBlob = nullptr;
	_d3dDsBlob = nullptr;

	ZeroMemory(&_vsBlob, sizeof(ShaderBlob));
	ZeroMemory(&_psBlob, sizeof(ShaderBlob));
	ZeroMemory(&_gsBlob, sizeof(ShaderBlob));
	ZeroMemory(&_hsBlob, sizeof(ShaderBlob));
	ZeroMemory(&_dsBlob, sizeof(ShaderBlob));
	ZeroMemory(&_csBlob, sizeof(ShaderBlob));

	for (int i = 0; i < 10; ++i)
		_vsBuffers[i].index = -1;

	for (int i = 0; i < 10; ++i)
		_fsBuffers[i].index = -1;

	_nextBinding = 0;
	_vsNumBuffers = 0;
	_fsNumBuffers = 0;
	_layout = nullptr;
}

void D3D11Shader::Enable()
{
	if (_vs)
		_ctx->deviceContext->VSSetShader(_vs, 0, 0);

	if (_ps)
		_ctx->deviceContext->PSSetShader(_ps, 0, 0);

	if (_gs)
		_ctx->deviceContext->GSSetShader(_gs, 0, 0);

	if (_hs)
		_ctx->deviceContext->HSSetShader(_hs, 0, 0);

	if (_ds)
		_ctx->deviceContext->DSSetShader(_ds, 0, 0);

	if (_cs)
		_ctx->deviceContext->CSSetShader(_cs, 0, 0);

	D3D11Renderer::SetActiveShader(this);
}

void D3D11Shader::Disable()
{ 
	D3D11Renderer::SetActiveShader(nullptr);

	for (pair<unsigned int, D3D11Texture*> kvp : _textures)
	{
		if (!kvp.second->IsUsable())
			kvp.second->CreateTexture();

		_ctx->deviceContext->VSSetSamplers(kvp.first, 0, nullptr);
		_ctx->deviceContext->PSSetSamplers(kvp.first, 0, nullptr);

		_ctx->deviceContext->VSSetShaderResources(kvp.first, 0, nullptr);
		_ctx->deviceContext->PSSetShaderResources(kvp.first, 0, nullptr);
	}
}

void D3D11Shader::SetTexture(unsigned int location, RTexture *tex)
{
	_textures[location] = (D3D11Texture*)tex;
}

void D3D11Shader::BindUniformBuffers()
{
	for (int i = 0; i < 10; ++i)
	{
		if (_vsBuffers[i].index != -1)
		{
			ID3D11Buffer *buf = _vsBuffers[i].ubo->GetD3DBuffer();
			//if(_ctx->deviceContext1)			
			//	_ctx->deviceContext1->VSSetConstantBuffers1(_vsBuffers[i].index, 1, &buf, (const UINT *)_vsBuffers[i].offset, (const UINT *)_vsBuffers[i].size);
			//else
				_ctx->deviceContext->VSSetConstantBuffers(_vsBuffers[i].index, 1, &buf);
		}
	}
	
	for (int i = 0; i < 10; ++i)
	{
		if (_fsBuffers[i].index != -1)
		{
			ID3D11Buffer *buf = _fsBuffers[i].ubo->GetD3DBuffer();
			//if (_ctx->deviceContext1)
			//	_ctx->deviceContext1->PSSetConstantBuffers1(_fsBuffers[i].index, 1, &buf, (const UINT *)_fsBuffers[i].offset, (const UINT *)_fsBuffers[i].size);
			//else
				_ctx->deviceContext->PSSetConstantBuffers(_fsBuffers[i].index, 1, &buf);
		}
	}
}

void D3D11Shader::VSUniformBlockBinding(int location, const char *name)
{
	//
}

void D3D11Shader::FSUniformBlockBinding(int location, const char *name)
{
	//
}

void D3D11Shader::VSSetUniformBuffer(int location, uint64_t offset, uint64_t size, RBuffer *buf)
{
	_vsBuffers[_vsNumBuffers].index = location;
	_vsBuffers[_vsNumBuffers].offset = offset;
	_vsBuffers[_vsNumBuffers].size = size;
	_vsBuffers[_vsNumBuffers++].ubo = (D3D11Buffer *)buf;
}

void D3D11Shader::FSSetUniformBuffer(int location, uint64_t offset, uint64_t size, RBuffer *buf)
{
	_fsBuffers[_fsNumBuffers].index = location;
	_fsBuffers[_fsNumBuffers].offset = offset;
	_fsBuffers[_fsNumBuffers].size = size;
	_fsBuffers[_fsNumBuffers++].ubo = (D3D11Buffer *)buf;
}

void D3D11Shader::SetSubroutines(ShaderType type, int count, const uint32_t *indices) { }

bool D3D11Shader::LoadFromSource(ShaderType type, int count, const char *source, int length)
{
	HRESULT hr;
	char *target = NULL;
	ID3DBlob *shaderBlob = nullptr;
	ID3DBlob *errorBlob = nullptr;

	char *wrapper = "#define LANG_HLSL 1\n\
	#define BEGIN_ATTRIBS struct VSInput { \n\
	#define ATTRIB(a, b, c, d) a b : d \n\
	#define END_ATTRIBS }; \n\
	#define GET_ATTRIB(a) input. ## a \n\
	#define BEGIN_VARS_IN struct PSInput { float4 position : SV_POSITION; \n\
	#define BEGIN_VARS_OUT struct PSInput { float4 position : SV_POSITION; \n\
	#define VAR(a, b, c) a b : c \n\
	#define END_VARS }; \n\
	#define GET_VAR_IN(a) input. ## a \n\
	#define GET_VAR_OUT(a) output. ## a \n\
	#define BEGIN_UNIFORM(a) cbuffer a { \n\
	#define END_UNIFORM }; \n\
	#define BEGIN_PSOUT struct PSOutput { \n\
	#define END_PSOUT }; \n\
	#define PSOUT(a, b, c) a b : SV_Target ## c \n\
	#define GET_PSOUT(a) output. ## a \n\
	#define SH_POSITION	output.position \n\
	#define BEGIN_VS PSInput main(VSInput input) { PSInput output; \n\
	#define END_VS return output; } \n\
	#define BEGIN_PS PSOutput main(PSInput input) { PSOutput output; \n\
	#define END_PS return output; } \n\
	#define TEX_SAMPLE(a, b) a ## .Sample(sam ## a, b) \n\
	#define TEX2D_SAMPLE(a, b) TEX_SAMPLE(a, b) \n\
	#define TEXCUBE_SAMPLE(a, b) TEX_SAMPLE(a, b) \n\
	#define TEX2D(a, b) Texture2D a : register(t ## b); SamplerState sam ## a : register(s ## b);\n\
	#define TEXCUBE(a, b) TextureCube a : register(t ## b); SamplerState sam ## a : register(s ## b);\n";

	size_t totalLen = strlen(wrapper) + length + 1;
	char *src = (char *)calloc(totalLen, 1);
	snprintf(src, totalLen, "%s%s", wrapper, source);

	switch (type)
	{
	case ShaderType::Vertex:
		hr = D3DCompile(src, totalLen, NULL, NULL, D3D_COMPILE_STANDARD_FILE_INCLUDE, "main", "vs_5_0",
			D3DCOMPILE_AVOID_FLOW_CONTROL | D3DCOMPILE_OPTIMIZATION_LEVEL3, 0, &_d3dVsBlob, &errorBlob);
	break;
	case ShaderType::Fragment:
		hr = D3DCompile(src, totalLen, NULL, NULL, D3D_COMPILE_STANDARD_FILE_INCLUDE, "main", "ps_5_0",
			D3DCOMPILE_AVOID_FLOW_CONTROL | D3DCOMPILE_OPTIMIZATION_LEVEL3, 0, &_d3dPsBlob, &errorBlob);
	break;
	case ShaderType::Geometry:
		hr = D3DCompile(src, totalLen, NULL, NULL, D3D_COMPILE_STANDARD_FILE_INCLUDE, "main", "gs_5_0",
			D3DCOMPILE_AVOID_FLOW_CONTROL | D3DCOMPILE_OPTIMIZATION_LEVEL3, 0, &_d3dGsBlob, &errorBlob);
	break;
	case ShaderType::TesselationControl:
		hr = D3DCompile(src, totalLen, NULL, NULL, D3D_COMPILE_STANDARD_FILE_INCLUDE, "main", "hs_5_0",
			D3DCOMPILE_AVOID_FLOW_CONTROL | D3DCOMPILE_OPTIMIZATION_LEVEL3, 0, &_d3dHsBlob, &errorBlob);
	break;
	case ShaderType::TesselationEval:
		hr = D3DCompile(src, totalLen, NULL, NULL, D3D_COMPILE_STANDARD_FILE_INCLUDE, "main", "ds_5_0",
			D3DCOMPILE_AVOID_FLOW_CONTROL | D3DCOMPILE_OPTIMIZATION_LEVEL3, 0, &_d3dDsBlob, &errorBlob);
	break;
	case ShaderType::Compute:
		hr = D3DCompile(src, totalLen, NULL, NULL, D3D_COMPILE_STANDARD_FILE_INCLUDE, "main", "cs_5_0",
			D3DCOMPILE_AVOID_FLOW_CONTROL | D3DCOMPILE_OPTIMIZATION_LEVEL3, 0, &_d3dCsBlob, &errorBlob);
	break;
	}

	free(src);

	if (FAILED(hr))
	{
		char *str = (char*)errorBlob->GetBufferPointer();
		OutputDebugStringA(str);
		errorBlob->Release();
		return false;
	}

	return true;
}

bool D3D11Shader::LoadFromStageBinary(ShaderType type, const char *file)
{
	ifstream stm;
	size_t size;
	char *data;

	stm.open(file, ifstream::in | ifstream::binary);

	if (!stm.good())
		return false;

	stm.seekg(0, ios::end);
	size = (size_t)stm.tellg();
	data = new char[size];
	stm.seekg(0, ios::beg);
	stm.read(data, size);
	stm.close();

	HRESULT hr;
	
	switch (type)
	{
		case ShaderType::Vertex:
		{
			free(_vsBlob.data);
			_vsBlob.data = data;
			_vsBlob.size = size;
			hr = _ctx->device->CreateVertexShader(data, size, 0, &_vs);
		}
		break;
		case ShaderType::Fragment:
		{
			free(_psBlob.data);
			_psBlob.data = data;
			_psBlob.size = size;
			hr = _ctx->device->CreatePixelShader(data, size, 0, &_ps);
		}
		break;
		case ShaderType::Geometry:
		{
			free(_vsBlob.data);
			_gsBlob.data = data;
			_gsBlob.size = size;
			hr = _ctx->device->CreateGeometryShader(data, size, 0, &_gs);
		}
		break;
		case ShaderType::Compute:
		{
			free(_csBlob.data);
			_csBlob.data = data;
			_csBlob.size = size;
			hr = _ctx->device->CreateComputeShader(data, size, 0, &_cs);
		}
		break;
	}

	if (FAILED(hr))
		return false;

	return true;
}

bool D3D11Shader::LoadFromBinary(int count, const void *binary, size_t length)
{
	return false;
}

bool D3D11Shader::Link()
{
	if (_d3dVsBlob)
	{
		if (FAILED(_ctx->device->CreateVertexShader(_d3dVsBlob->GetBufferPointer(), _d3dVsBlob->GetBufferSize(), 0, &_vs)))
			return false;
	}

	if (_d3dPsBlob)
	{
		if (FAILED(_ctx->device->CreatePixelShader(_d3dPsBlob->GetBufferPointer(), _d3dPsBlob->GetBufferSize(), 0, &_ps)))
			return false;
	}

	if (_d3dGsBlob)
	{
		if (FAILED(_ctx->device->CreateGeometryShader(_d3dGsBlob->GetBufferPointer(), _d3dGsBlob->GetBufferSize(), 0, &_gs)))
			return false;
	}

	if (_d3dHsBlob)
	{
		if (FAILED(_ctx->device->CreateHullShader(_d3dHsBlob->GetBufferPointer(), _d3dHsBlob->GetBufferSize(), 0, &_hs)))
			return false;
	}

	if (_d3dDsBlob)
	{
		if (FAILED(_ctx->device->CreateDomainShader(_d3dDsBlob->GetBufferPointer(), _d3dDsBlob->GetBufferSize(), 0, &_ds)))
			return false;
	}

	if (_d3dCsBlob)
	{
		if (FAILED(_ctx->device->CreateComputeShader(_d3dCsBlob->GetBufferPointer(), _d3dCsBlob->GetBufferSize(), 0, &_cs)))
			return false;
	}

	return true;
}

D3D11Shader::~D3D11Shader()
{
	if (_vs)
		_vs->Release();

	if (_ps)
		_ps->Release();

	if (_gs)
		_gs->Release();

	if (_hs)
		_hs->Release();

	if (_ds)
		_ds->Release();

	if (_cs)
		_cs->Release();

	free(_vsBlob.data);
	free(_psBlob.data);
	free(_gsBlob.data);
	free(_hsBlob.data);
	free(_dsBlob.data);
	free(_csBlob.data);

	if(_d3dVsBlob)
		_d3dVsBlob->Release();

	if (_d3dPsBlob)
		_d3dPsBlob->Release();

	if (_d3dGsBlob)
		_d3dGsBlob->Release();

	if (_d3dCsBlob)
		_d3dCsBlob->Release();

	if (_d3dHsBlob)
		_d3dHsBlob->Release();

	if (_d3dDsBlob)
		_d3dDsBlob->Release();
}

void D3D11Shader::EnableTextures()
{
	for (pair<unsigned int, D3D11Texture*> kvp : _textures)
	{
		if (!kvp.second->IsUsable())
			kvp.second->CreateTexture();

		_ctx->deviceContext->VSSetSamplers(kvp.first, 1, kvp.second->GetPPSS());
		_ctx->deviceContext->PSSetSamplers(kvp.first, 1, kvp.second->GetPPSS());

		_ctx->deviceContext->VSSetShaderResources(kvp.first, 1, kvp.second->GetPPSRV());
		_ctx->deviceContext->PSSetShaderResources(kvp.first, 1, kvp.second->GetPPSRV());
	}
}

void D3D11Shader::SetInputLayout()
{
	if (!_layout)
	{
		D3D11Buffer *vertexBuffer = D3D11Renderer::GetActiveArrayBuffer()->GetVertexBuffer();
		D3D11_INPUT_ELEMENT_DESC descriptors[10];
		int numDescriptors = 0;


		for (BufferAttribute &attrib : vertexBuffer->GetAttributes())
			descriptors[numDescriptors++] = { attrib.name.c_str(), 0, D3D11_BufferDataType[((int)attrib.type * 4) + attrib.size - 1], 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 };

		if(_d3dVsBlob)
			_ctx->device->CreateInputLayout(descriptors, numDescriptors, _d3dVsBlob->GetBufferPointer(), _d3dVsBlob->GetBufferSize(), &_layout);
		else
			_ctx->device->CreateInputLayout(descriptors, numDescriptors, _vsBlob.data, _vsBlob.size, &_layout);
	}

	_ctx->deviceContext->IASetInputLayout(_layout);
}