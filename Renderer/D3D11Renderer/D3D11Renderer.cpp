/* NekoEngine
 *
 * D3D11Renderer.cpp
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

#include "D3D11Renderer.h"
#include "D3D11Fence.h"
#include "D3D11Buffer.h"
#include "D3D11Shader.h"
#include "D3D11Texture.h"
#include "D3D11Framebuffer.h"
#include "D3D11ArrayBuffer.h"

#pragma comment (lib, "d3d11.lib")

D3D11_PRIMITIVE_TOPOLOGY D3D11_DrawModes[2] =
{
	D3D10_PRIMITIVE_TOPOLOGY_TRIANGLELIST,
	D3D10_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP
};

std::vector<ShaderDefine> D3D11Renderer::_shaderDefines;
D3D11Shader *D3D11Renderer::_activeShader = nullptr;
D3D11ArrayBuffer *D3D11Renderer::_activeArrayBuffer = nullptr;
D3D11Framebuffer *D3D11Renderer::_boundFramebuffer = nullptr;

D3D11_BLEND D3D11_BlendFactor[19]
{
	D3D11_BLEND_ZERO,
	D3D11_BLEND_ONE,
	D3D11_BLEND_SRC_COLOR,
	D3D11_BLEND_INV_SRC_COLOR,
	D3D11_BLEND_DEST_COLOR,
	D3D11_BLEND_INV_DEST_COLOR,
	D3D11_BLEND_SRC_ALPHA,
	D3D11_BLEND_INV_SRC_ALPHA,
	D3D11_BLEND_DEST_ALPHA,
	D3D11_BLEND_INV_DEST_ALPHA,
	D3D11_BLEND_ZERO,
	D3D11_BLEND_ZERO,
	D3D11_BLEND_ZERO,
	D3D11_BLEND_ZERO,
	D3D11_BLEND_SRC_ALPHA_SAT,
	D3D11_BLEND_SRC1_COLOR,
	D3D11_BLEND_INV_SRC1_COLOR,
	D3D11_BLEND_SRC1_ALPHA,
	D3D11_BLEND_INV_SRC1_ALPHA
};

D3D11_BLEND_OP D3D11_BlendOp [5]
{
	D3D11_BLEND_OP_ADD,
	D3D11_BLEND_OP_SUBTRACT,
	D3D11_BLEND_OP_REV_SUBTRACT,
	D3D11_BLEND_OP_MIN,
	D3D11_BLEND_OP_MAX
};

D3D11_COMPARISON_FUNC D3D11_TestFunc[8] =
{
	D3D11_COMPARISON_NEVER,
	D3D11_COMPARISON_LESS,
	D3D11_COMPARISON_LESS_EQUAL,
	D3D11_COMPARISON_GREATER,
	D3D11_COMPARISON_GREATER_EQUAL,
	D3D11_COMPARISON_EQUAL,
	D3D11_COMPARISON_NOT_EQUAL,
	D3D11_COMPARISON_ALWAYS
};

D3D11_STENCIL_OP D3D11_StencilOp[8] =
{
	D3D11_STENCIL_OP_KEEP,
	D3D11_STENCIL_OP_ZERO,
	D3D11_STENCIL_OP_REPLACE,
	D3D11_STENCIL_OP_INCR,
	D3D11_STENCIL_OP_INCR_SAT,
	D3D11_STENCIL_OP_DECR,
	D3D11_STENCIL_OP_DECR_SAT,
	D3D11_STENCIL_OP_INVERT
};

D3D11_CULL_MODE D3D11_CullMode[3] =
{
	D3D11_CULL_FRONT,
	D3D11_CULL_BACK,
	D3D11_CULL_FRONT
};

D3D11Renderer::D3D11Renderer()
{
}

bool D3D11Renderer::Initialize(PlatformWindowType hWnd, std::unordered_map<std::string, std::string> *args, bool debug)
{
	HRESULT hr = S_OK;

	ZeroMemory(&_ctx, sizeof(D3D11Context));

	RECT rc;
	GetClientRect(hWnd, &rc);
	UINT width = rc.right - rc.left;
	UINT height = rc.bottom - rc.top;

	UINT createDeviceFlags = 0;

#ifdef _DEBUG
	createDeviceFlags |= D3D11_CREATE_DEVICE_DEBUG;
#endif

	D3D_DRIVER_TYPE driverTypes[] =
	{
		D3D_DRIVER_TYPE_HARDWARE,
		D3D_DRIVER_TYPE_WARP,
		D3D_DRIVER_TYPE_REFERENCE
	};
	UINT numDriverTypes = ARRAYSIZE(driverTypes);

	D3D_FEATURE_LEVEL featureLevels[] =
	{
		D3D_FEATURE_LEVEL_11_1,
		D3D_FEATURE_LEVEL_11_0,
		D3D_FEATURE_LEVEL_10_1,
		D3D_FEATURE_LEVEL_10_0,
	};
	UINT numFeatureLevels = ARRAYSIZE(featureLevels);

	for (UINT driverTypeIndex = 0; driverTypeIndex < numDriverTypes; driverTypeIndex++)
	{
		_driverType = driverTypes[driverTypeIndex];
		hr = D3D11CreateDevice(nullptr, _driverType, nullptr, createDeviceFlags, featureLevels, numFeatureLevels,
			D3D11_SDK_VERSION, &_ctx.device, &_featureLevel, &_ctx.deviceContext);

		if (hr == E_INVALIDARG)
		{
			// DirectX 11.0 platforms will not recognize D3D_FEATURE_LEVEL_11_1 so we need to retry without it
			hr = D3D11CreateDevice(nullptr, _driverType, nullptr, createDeviceFlags, &featureLevels[1], numFeatureLevels - 1,
				D3D11_SDK_VERSION, &_ctx.device, &_featureLevel, &_ctx.deviceContext);
		}

		if (SUCCEEDED(hr))
			break;
	}
	if (FAILED(hr))
		return false;

	// Obtain DXGI factory from device (since we used nullptr for pAdapter above)
	IDXGIFactory1* dxgiFactory = nullptr;
	{
		IDXGIDevice* dxgiDevice = nullptr;
		hr = _ctx.device->QueryInterface(__uuidof(IDXGIDevice), reinterpret_cast<void**>(&dxgiDevice));
		if (SUCCEEDED(hr))
		{
			IDXGIAdapter* adapter = nullptr;
			hr = dxgiDevice->GetAdapter(&adapter);
			if (SUCCEEDED(hr))
			{
				hr = adapter->GetParent(__uuidof(IDXGIFactory1), reinterpret_cast<void**>(&dxgiFactory));
				adapter->Release();
			}
			dxgiDevice->Release();
		}
	}
	if (FAILED(hr))
		return false;

	// Create swap chain
	IDXGIFactory2* dxgiFactory2 = nullptr;
	hr = dxgiFactory->QueryInterface(__uuidof(IDXGIFactory2), reinterpret_cast<void**>(&dxgiFactory2));
	if (dxgiFactory2)
	{
		// DirectX 11.1 or later
		hr = _ctx.device->QueryInterface(__uuidof(ID3D11Device1), reinterpret_cast<void**>(&_ctx.device1));
		if (SUCCEEDED(hr))
		{
			(void)_ctx.deviceContext->QueryInterface(__uuidof(ID3D11DeviceContext1), reinterpret_cast<void**>(&_ctx.deviceContext1));
		}

		DXGI_SWAP_CHAIN_DESC1 sd;
		ZeroMemory(&sd, sizeof(sd));
		sd.Width = width;
		sd.Height = height;
		sd.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
		sd.SampleDesc.Count = 1;
		sd.SampleDesc.Quality = 0;
		sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
		sd.BufferCount = 1;

		hr = dxgiFactory2->CreateSwapChainForHwnd(_ctx.device, hWnd, &sd, nullptr, nullptr, &_swapChain1);
		if (SUCCEEDED(hr))
		{
			hr = _swapChain1->QueryInterface(__uuidof(IDXGISwapChain), reinterpret_cast<void**>(&_swapChain));
		}

		dxgiFactory2->Release();
	}
	else
	{
		// DirectX 11.0 systems
		DXGI_SWAP_CHAIN_DESC sd;
		ZeroMemory(&sd, sizeof(sd));
		sd.BufferCount = 1;
		sd.BufferDesc.Width = width;
		sd.BufferDesc.Height = height;
		sd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
		sd.BufferDesc.RefreshRate.Numerator = 60;
		sd.BufferDesc.RefreshRate.Denominator = 1;
		sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
		sd.OutputWindow = hWnd;
		sd.SampleDesc.Count = 1;
		sd.SampleDesc.Quality = 0;
		sd.Windowed = TRUE;

		hr = dxgiFactory->CreateSwapChain(_ctx.device, &sd, &_swapChain);
	}

	dxgiFactory->MakeWindowAssociation(hWnd, DXGI_MWA_NO_ALT_ENTER);

	dxgiFactory->Release();

	ID3D11Texture2D *backBufferTexture;
	hr = _swapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (LPVOID *)&backBufferTexture);

	if (hr != S_OK)
		return false;

	hr = _ctx.device->CreateRenderTargetView(backBufferTexture, NULL, &_ctx.renderTargetView);
	backBufferTexture->Release();

	if (hr != S_OK)
		return false;

	ID3D11Texture2D* depthStencil = NULL;
	D3D11_TEXTURE2D_DESC depthDesc;
	depthDesc.Width = width;
	depthDesc.Height = height;
	depthDesc.MipLevels = 1;
	depthDesc.ArraySize = 1;
	depthDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
	depthDesc.SampleDesc.Count = 1;
	depthDesc.SampleDesc.Quality = 0;
	depthDesc.Usage = D3D11_USAGE_DEFAULT;
	depthDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL;
	depthDesc.CPUAccessFlags = 0;
	depthDesc.MiscFlags = 0;
	hr = _ctx.device->CreateTexture2D(&depthDesc, NULL, &depthStencil);
	if (hr != S_OK)
		return false;

	D3D11_DEPTH_STENCIL_VIEW_DESC dsvDesc;
	ZeroMemory(&dsvDesc, sizeof(dsvDesc));
	dsvDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
	dsvDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
	dsvDesc.Texture2D.MipSlice = 0;

	hr = _ctx.device->CreateDepthStencilView(depthStencil, &dsvDesc, &_ctx.depthStencilView);
	if (hr != S_OK)
		return false;

	_ctx.deviceContext->OMSetRenderTargets(1, &_ctx.renderTargetView, _ctx.depthStencilView);

	D3D11_VIEWPORT vp;
	vp.Width = (FLOAT)width;
	vp.Height = (FLOAT)height;
	vp.MinDepth = 0.0f;
	vp.MaxDepth = 1.0f;
	vp.TopLeftX = 0;
	vp.TopLeftY = 0;
	_ctx.deviceContext->RSSetViewports(1, &vp);

	memset(&_rasterizerDesc, 0x0, sizeof(_rasterizerDesc));

	_cullMode = D3D11_CULL_BACK;
	_rasterizerDesc.AntialiasedLineEnable = true;
	_rasterizerDesc.CullMode = D3D11_CULL_BACK;
	_rasterizerDesc.FrontCounterClockwise = true;
	_rasterizerDesc.DepthClipEnable = true;
	_rasterizerDesc.FillMode = D3D11_FILL_SOLID;
	_rasterizerDesc.MultisampleEnable = true;
	_rasterizerDesc.ScissorEnable = false;
	
	hr = _ctx.device->CreateRasterizerState(&_rasterizerDesc, &_rasterizerState);

	if (hr != S_OK)
		return false;

	_ctx.deviceContext->RSSetState(_rasterizerState);

	ZeroMemory(&_blendState, sizeof(D3D11_BLEND_DESC));
	
	_blendState.RenderTarget[0].BlendEnable = FALSE;
	_blendState.RenderTarget[0].SrcBlend = D3D11_BLEND_SRC_ALPHA;
	_blendState.RenderTarget[0].DestBlend = D3D11_BLEND_INV_SRC_ALPHA;
	_blendState.RenderTarget[0].BlendOp = D3D11_BLEND_OP_ADD;
	_blendState.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_ONE;
	_blendState.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_ZERO;
	_blendState.RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP_ADD;
	_blendState.RenderTarget[0].RenderTargetWriteMask = 0x0f;

	hr = _ctx.device->CreateBlendState(&_blendState, &_disableAlpha);
	if (FAILED(hr))
		return false;

	_blendState.RenderTarget[0].BlendEnable = TRUE;

	hr = _ctx.device->CreateBlendState(&_blendState, &_enableAlpha);
	if (FAILED(hr))
		return false;

	_blendFactor[0] = 0.f;
	_blendFactor[1] = 0.f;
	_blendFactor[2] = 0.f;
	_blendFactor[3] = 0.f;

	_stencilRef = 0;
	ZeroMemory(&_depthStencilStateDesc, sizeof(D3D11_DEPTH_STENCIL_DESC));
	
	_depthStencilStateDesc.DepthEnable = FALSE;
	_depthStencilStateDesc.DepthFunc = D3D11_COMPARISON_LESS;
	_depthStencilStateDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;

	_depthStencilStateDesc.StencilEnable = FALSE;
	_depthStencilStateDesc.StencilReadMask = 0xFF;
	_depthStencilStateDesc.StencilWriteMask = 0xFF;

	_depthStencilStateDesc.FrontFace.StencilFunc = D3D11_COMPARISON_ALWAYS;
	_depthStencilStateDesc.FrontFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;
	_depthStencilStateDesc.FrontFace.StencilDepthFailOp = D3D11_STENCIL_OP_KEEP;
	_depthStencilStateDesc.FrontFace.StencilPassOp = D3D11_STENCIL_OP_KEEP;

	_depthStencilStateDesc.BackFace.StencilFunc = D3D11_COMPARISON_ALWAYS;
	_depthStencilStateDesc.BackFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;
	_depthStencilStateDesc.BackFace.StencilDepthFailOp = D3D11_STENCIL_OP_KEEP;
	_depthStencilStateDesc.BackFace.StencilPassOp = D3D11_STENCIL_OP_KEEP;

	hr = _ctx.device->CreateDepthStencilState(&_depthStencilStateDesc, &_depthStencilState);
	if (FAILED(hr))
		return false;

	_syncInterval = 0;
	_drawCalls = 0;

	return true;
}

void D3D11Renderer::SetDebugLogFunction(RendererDebugLogProc debugLogFunction)
{
	//
}

const char* D3D11Renderer::GetName()
{
	return "DirectX";
}

int D3D11Renderer::GetMajorVersion()
{
	return 11;
}

int D3D11Renderer::GetMinorVersion()
{
	if (_ctx.deviceContext1)
		return 1;
	return 0;
}

void D3D11Renderer::SetClearColor(float r, float g, float b, float a)
{
	_clearColor[0] = r;
	_clearColor[1] = g;
	_clearColor[2] = b;
	_clearColor[3] = a;
}

void D3D11Renderer::SetViewport(int x, int y, int width, int height)
{
	D3D11_VIEWPORT viewport;
	ZeroMemory(&viewport, sizeof(D3D11_VIEWPORT));

	viewport.TopLeftX = (float)x;
	viewport.TopLeftY = (float)y;
	viewport.Width = (float)width;
	viewport.Height = (float)height;

	_ctx.deviceContext->RSSetViewports(1, &viewport);
}

void D3D11Renderer::EnableDepthTest(bool enable)
{
	_depthStencilStateDesc.DepthEnable = enable;

	_depthStencilState->Release();
	HRESULT hr = _ctx.device->CreateDepthStencilState(&_depthStencilStateDesc, &_depthStencilState);
	if (FAILED(hr))
		return;

	_ctx.deviceContext->OMSetDepthStencilState(_depthStencilState, _stencilRef);
}

void D3D11Renderer::SetDepthFunc(TestFunc func)
{
	_depthStencilStateDesc.DepthFunc = D3D11_TestFunc[(int)func];

	_depthStencilState->Release();
	HRESULT hr = _ctx.device->CreateDepthStencilState(&_depthStencilStateDesc, &_depthStencilState);
	if (FAILED(hr))
		return;

	_ctx.deviceContext->OMSetDepthStencilState(_depthStencilState, _stencilRef);
}

void D3D11Renderer::SetDepthRange(double near, double far)
{
	//
}

void D3D11Renderer::SetDepthRangef(float near, float far)
{
	//
}

void D3D11Renderer::SetDepthMask(bool mask)
{
	_depthStencilStateDesc.DepthWriteMask = mask ? D3D11_DEPTH_WRITE_MASK_ALL : D3D11_DEPTH_WRITE_MASK_ZERO;

	_depthStencilState->Release();
	HRESULT hr = _ctx.device->CreateDepthStencilState(&_depthStencilStateDesc, &_depthStencilState);
	if (FAILED(hr))
		return;

	_ctx.deviceContext->OMSetDepthStencilState(_depthStencilState, _stencilRef);
}

void D3D11Renderer::EnableStencilTest(bool enable)
{
	_depthStencilStateDesc.StencilEnable = enable;

	_depthStencilState->Release();
	HRESULT hr = _ctx.device->CreateDepthStencilState(&_depthStencilStateDesc, &_depthStencilState);
	if (FAILED(hr))
		return;

	_ctx.deviceContext->OMSetDepthStencilState(_depthStencilState, _stencilRef);
}

void D3D11Renderer::SetStencilFunc(TestFunc func, int ref, unsigned int mask)
{
	_stencilRef = ref;

	_depthStencilStateDesc.StencilReadMask = mask;
	_depthStencilStateDesc.StencilWriteMask = mask;
	_depthStencilStateDesc.FrontFace.StencilFunc = D3D11_TestFunc[(int)func];
	_depthStencilStateDesc.BackFace.StencilFunc = D3D11_TestFunc[(int)func];

	_depthStencilState->Release();
	HRESULT hr = _ctx.device->CreateDepthStencilState(&_depthStencilStateDesc, &_depthStencilState);
	if (FAILED(hr))
		return;

	_ctx.deviceContext->OMSetDepthStencilState(_depthStencilState, _stencilRef);
}

void D3D11Renderer::SetStencilFuncSeparate(PolygonFace face, TestFunc func, int ref, unsigned int mask)
{
	_stencilRef = ref;

	_depthStencilStateDesc.StencilReadMask = mask;
	_depthStencilStateDesc.StencilWriteMask = mask;

	if (face == PolygonFace::Front || face == PolygonFace::FrontAndBack)
		_depthStencilStateDesc.FrontFace.StencilFunc = D3D11_TestFunc[(int)func];

	if (face == PolygonFace::Back || face == PolygonFace::FrontAndBack)
		_depthStencilStateDesc.BackFace.StencilFunc = D3D11_TestFunc[(int)func];

	_depthStencilState->Release();
	HRESULT hr = _ctx.device->CreateDepthStencilState(&_depthStencilStateDesc, &_depthStencilState);
	if (FAILED(hr))
		return;

	_ctx.deviceContext->OMSetDepthStencilState(_depthStencilState, _stencilRef);
}

void D3D11Renderer::SetStencilOp(TestOp sfail, TestOp dpfail, TestOp dppass)
{
	_depthStencilStateDesc.FrontFace.StencilFailOp = D3D11_StencilOp[(int)sfail];
	_depthStencilStateDesc.FrontFace.StencilDepthFailOp = D3D11_StencilOp[(int)dpfail];
	_depthStencilStateDesc.FrontFace.StencilPassOp = D3D11_StencilOp[(int)dppass];

	_depthStencilStateDesc.BackFace.StencilFailOp = D3D11_StencilOp[(int)sfail];
	_depthStencilStateDesc.BackFace.StencilDepthFailOp = D3D11_StencilOp[(int)dpfail];
	_depthStencilStateDesc.BackFace.StencilPassOp = D3D11_StencilOp[(int)dppass];

	_depthStencilState->Release();
	HRESULT hr = _ctx.device->CreateDepthStencilState(&_depthStencilStateDesc, &_depthStencilState);
	if (FAILED(hr))
		return;

	_ctx.deviceContext->OMSetDepthStencilState(_depthStencilState, _stencilRef);
}

void D3D11Renderer::SetStencilOpSeparate(PolygonFace face, TestOp sfail, TestOp dpfail, TestOp dppass)
{
	if (face == PolygonFace::Front || face == PolygonFace::FrontAndBack)
	{
		_depthStencilStateDesc.FrontFace.StencilFailOp = D3D11_StencilOp[(int)sfail];
		_depthStencilStateDesc.FrontFace.StencilDepthFailOp = D3D11_StencilOp[(int)dpfail];
		_depthStencilStateDesc.FrontFace.StencilPassOp = D3D11_StencilOp[(int)dppass];
	}

	if (face == PolygonFace::Back || face == PolygonFace::FrontAndBack)
	{
		_depthStencilStateDesc.BackFace.StencilFailOp = D3D11_StencilOp[(int)sfail];
		_depthStencilStateDesc.BackFace.StencilDepthFailOp = D3D11_StencilOp[(int)dpfail];
		_depthStencilStateDesc.BackFace.StencilPassOp = D3D11_StencilOp[(int)dppass];
	}

	_depthStencilState->Release();
	HRESULT hr = _ctx.device->CreateDepthStencilState(&_depthStencilStateDesc, &_depthStencilState);
	if (FAILED(hr))
		return;

	_ctx.deviceContext->OMSetDepthStencilState(_depthStencilState, _stencilRef);
}

void D3D11Renderer::EnableBlend(bool enable)
{
	_ctx.deviceContext->OMSetBlendState(enable ? _enableAlpha : _disableAlpha, _blendFactor, 0xFFFFFFFF);
}

void D3D11Renderer::SetBlendFunc(BlendFactor src, BlendFactor dst)
{
	_blendState.RenderTarget[0].SrcBlend = D3D11_BlendFactor[(int)src];
	_blendState.RenderTarget[0].DestBlend = D3D11_BlendFactor[(int)dst];
	_blendState.RenderTarget[0].SrcBlendAlpha = D3D11_BlendFactor[(int)src];
	_blendState.RenderTarget[0].DestBlendAlpha = D3D11_BlendFactor[(int)dst];

	_enableAlpha->Release();
	HRESULT hr = _ctx.device->CreateBlendState(&_blendState, &_enableAlpha);
	if (FAILED(hr))
	{
		// die
	}
}

void D3D11Renderer::SetBlendFuncSeparate(BlendFactor srcColor, BlendFactor dstColor, BlendFactor srcAlpa, BlendFactor dstAlpha)
{
	_blendState.RenderTarget[0].SrcBlend = D3D11_BlendFactor[(int)srcColor];
	_blendState.RenderTarget[0].DestBlend = D3D11_BlendFactor[(int)dstColor];
	_blendState.RenderTarget[0].SrcBlendAlpha = D3D11_BlendFactor[(int)srcAlpa];
	_blendState.RenderTarget[0].DestBlendAlpha = D3D11_BlendFactor[(int)dstAlpha];

	_enableAlpha->Release();
	HRESULT hr = _ctx.device->CreateBlendState(&_blendState, &_enableAlpha);
	if (FAILED(hr))
	{
		// die
	}
}

void D3D11Renderer::SetBlendColor(float r, float g, float b, float a)
{
	_blendFactor[0] = r;
	_blendFactor[1] = g;
	_blendFactor[2] = b;
	_blendFactor[3] = a;
}

void D3D11Renderer::SetBlendEquation(BlendEquation eq)
{
	_blendState.RenderTarget[0].BlendOp = D3D11_BlendOp[(int)eq];
	_blendState.RenderTarget[0].BlendOpAlpha = D3D11_BlendOp[(int)eq];

	_enableAlpha->Release();
	HRESULT hr = _ctx.device->CreateBlendState(&_blendState, &_enableAlpha);
	if (FAILED(hr))
	{
		// die
	}
}

void D3D11Renderer::SetBlendEquationSeparate(BlendEquation color, BlendEquation alpha)
{
	_blendState.RenderTarget[0].BlendOp = D3D11_BlendOp[(int)color];
	_blendState.RenderTarget[0].BlendOpAlpha = D3D11_BlendOp[(int)alpha];

	_enableAlpha->Release();
	HRESULT hr = _ctx.device->CreateBlendState(&_blendState, &_enableAlpha);
	if (FAILED(hr))
	{
		// die
	}
}

void D3D11Renderer::SetStencilMask(unsigned int mask)
{
	_depthStencilStateDesc.StencilReadMask = mask;
	_depthStencilStateDesc.StencilWriteMask = mask;

	_depthStencilState->Release();
	HRESULT hr = _ctx.device->CreateDepthStencilState(&_depthStencilStateDesc, &_depthStencilState);
	if (FAILED(hr))
		return;

	_ctx.deviceContext->OMSetDepthStencilState(_depthStencilState, _stencilRef);
}

void D3D11Renderer::SetStencilMaskSeparate(PolygonFace face, unsigned int mask)
{
	_depthStencilStateDesc.StencilReadMask = mask;
	_depthStencilStateDesc.StencilWriteMask = mask;

	_depthStencilState->Release();
	HRESULT hr = _ctx.device->CreateDepthStencilState(&_depthStencilStateDesc, &_depthStencilState);
	if (FAILED(hr))
		return;

	_ctx.deviceContext->OMSetDepthStencilState(_depthStencilState, _stencilRef);
}

void D3D11Renderer::EnableFaceCulling(bool enable)
{
	_rasterizerDesc.CullMode = enable ? _cullMode : D3D11_CULL_NONE;

	_rasterizerState->Release();
	HRESULT hr = _ctx.device->CreateRasterizerState(&_rasterizerDesc, &_rasterizerState);

	if (hr != S_OK)
		return;

	_ctx.deviceContext->RSSetState(_rasterizerState);
}

void D3D11Renderer::SetFaceCulling(PolygonFace face)
{
	_cullMode = D3D11_CullMode[(int)face];
	_rasterizerDesc.CullMode = _cullMode;

	_rasterizerState->Release();
	HRESULT hr = _ctx.device->CreateRasterizerState(&_rasterizerDesc, &_rasterizerState);

	if (hr != S_OK)
		return;

	_ctx.deviceContext->RSSetState(_rasterizerState);
}

void D3D11Renderer::SetFrontFace(FrontFace face)
{
	_rasterizerDesc.FrontCounterClockwise = (face == FrontFace::CounterClockwise);

	_rasterizerState->Release();
	HRESULT hr = _ctx.device->CreateRasterizerState(&_rasterizerDesc, &_rasterizerState);

	if (hr != S_OK)
		return;

	_ctx.deviceContext->RSSetState(_rasterizerState);
}

void D3D11Renderer::SetColorMask(bool r, bool g, bool b, bool a) { }

void D3D11Renderer::DrawArrays(PolygonMode mode, int32_t first, int count)
{
	if (_activeShader)
	{
		_activeShader->EnableTextures();
		_activeShader->SetInputLayout();
	}

	_ctx.deviceContext->IASetPrimitiveTopology(D3D11_DrawModes[(int)mode]);
	_ctx.deviceContext->Draw(count, first);
	++_drawCalls;
}

void D3D11Renderer::DrawElements(PolygonMode mode, int count, ElementType type, const void *indices)
{
	if (_activeShader)
	{
		_activeShader->EnableTextures();
		_activeShader->SetInputLayout();
	}

	_ctx.deviceContext->IASetPrimitiveTopology(D3D11_DrawModes[(int)mode]);
	_ctx.deviceContext->DrawIndexed(count, (int)indices, 0);
	++_drawCalls;
}

void D3D11Renderer::DrawElementsBaseVertex(PolygonMode mode, int32_t count, ElementType type, const void * indices, int32_t baseVertex)
{
	if (_activeShader)
	{
		_activeShader->EnableTextures();
		_activeShader->SetInputLayout();
	}

	_ctx.deviceContext->IASetPrimitiveTopology(D3D11_DrawModes[(int)mode]);
	_ctx.deviceContext->DrawIndexed(count, (int)indices, baseVertex);
	++_drawCalls;
}

void D3D11Renderer::Clear(uint32_t mask)
{
	if(mask & R_CLEAR_COLOR)
		_ctx.deviceContext->ClearRenderTargetView(_boundFramebuffer ? _boundFramebuffer->GetRTV() : _ctx.renderTargetView, _clearColor);

	UINT flags = 0;

	if (mask & R_CLEAR_DEPTH)
		flags |= D3D11_CLEAR_DEPTH;

	if (mask & R_CLEAR_STENCIL)
		flags |= D3D11_CLEAR_STENCIL;

	if (flags)
		_ctx.deviceContext->ClearDepthStencilView(_boundFramebuffer ? _boundFramebuffer->GetDSV() ? _boundFramebuffer->GetDSV() : _ctx.depthStencilView : _ctx.depthStencilView, flags, 1.f, 0);
}

void D3D11Renderer::ReadPixels(int x, int y, int width, int height, TextureFormat format, TextureInternalType type, void * data)
{
}

void D3D11Renderer::SwapBuffers()
{
	_swapChain->Present(_syncInterval, 0);
}

bool D3D11Renderer::HasCapability(RendererCapability cap)
{
	switch (cap)
	{
		case RendererCapability::MemoryInformation: return false;
		case RendererCapability::AnisotropicFiltering: return true;
		case RendererCapability::MultisampledFramebuffer: return true;
		case RendererCapability::PerSampleShading: return false;
		case RendererCapability::DrawBaseVertex: return true;
		default: return false;
	}
}

RBuffer *D3D11Renderer::CreateBuffer(BufferType type, bool dynamic, bool persistent)
{
	return (RBuffer *)new D3D11Buffer(&_ctx, type, dynamic, persistent);
}

RShader *D3D11Renderer::CreateShader()
{
	return (RShader *)new D3D11Shader(&_ctx);
}

RTexture *D3D11Renderer::CreateTexture(TextureType type)
{
	return (RTexture *)new D3D11Texture(&_ctx, type);
}

RFramebuffer *D3D11Renderer::CreateFramebuffer(int width, int height)
{
	return (RFramebuffer *)new D3D11Framebuffer(&_ctx, width, height);
}

RArrayBuffer *D3D11Renderer::CreateArrayBuffer()
{
	return (RArrayBuffer *)new D3D11ArrayBuffer();
}

RFence *D3D11Renderer::CreateFence()
{
	return (RFence *)new D3D11Fence();
}

void D3D11Renderer::AddShaderDefine(std::string name, std::string value)
{
	ShaderDefine d{name, value};
	_shaderDefines.push_back(d);
}

bool D3D11Renderer::IsTextureFormatSupported(TextureFileFormat format)
{
	switch (format)
	{
	case TextureFileFormat::DDS:
	case TextureFileFormat::TGA:
		return true;
	default:
		return false;
	}
}

void D3D11Renderer::SetPixelStore(PixelStoreParameter param, int value)
{
	//
}

void D3D11Renderer::ScreenResized()
{
	//
}

int D3D11Renderer::GetMaxSamples()
{
	return 31;
}

int D3D11Renderer::GetMaxAnisotropy()
{
	return 16;
}

uint64_t D3D11Renderer::GetVideoMemorySize()
{
	return uint64_t();
}

uint64_t D3D11Renderer::GetUsedVideoMemorySize()
{
	return uint64_t();
}

void D3D11Renderer::BindDefaultFramebuffer()
{
	_ctx.deviceContext->OMSetRenderTargets(1, &_ctx.renderTargetView, _ctx.depthStencilView);
	_boundFramebuffer = nullptr;
}

RFramebuffer * D3D11Renderer::GetBoundFramebuffer()
{
	return _boundFramebuffer;
}

void D3D11Renderer::SetMinSampleShading(int32_t samples)
{
}

void D3D11Renderer::SetSwapInterval(int swapInterval)
{
	_syncInterval = swapInterval;
}

D3D11Renderer::~D3D11Renderer()
{
	_swapChain->Release();
	_swapChain1->Release();
	_ctx.device->Release();
	if(_ctx.device1) _ctx.device1->Release();
	_ctx.deviceContext->Release();
	if(_ctx.deviceContext1) _ctx.deviceContext1->Release();	
	_ctx.renderTargetView->Release();
}