/* NekoEngine
 *
 * D3D11Framebuffer.cpp
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

#include "D3D11Framebuffer.h"
#include "D3D11Texture.h"
#include "D3D11Renderer.h"

D3D11Framebuffer::D3D11Framebuffer(D3D11Context *ctx, int width, int height) : RFramebuffer(width, height)
{
	_ctx = ctx;
	_rtvCount = 0;
	_multisampled = false;
	_dsv = nullptr;
	_depth = nullptr;
	memset(_rtv, 0x0, sizeof(ID3D11RenderTargetView *) * 9);
}

void D3D11Framebuffer::Bind(int location)
{
	_ctx->deviceContext->OMSetRenderTargets(_rtvCount, _rtv, _dsv);
	D3D11Renderer::SetBoundFramebuffer(this);
}

void D3D11Framebuffer::Unbind()
{
	_ctx->deviceContext->OMSetRenderTargets(1, &_ctx->renderTargetView, _ctx->depthStencilView);
	D3D11Renderer::SetBoundFramebuffer(nullptr);
}

void D3D11Framebuffer::Resize(int width, int height) { }

void D3D11Framebuffer::AttachTexture(DrawAttachment attachment, class RTexture* texture)
{
	D3D11Texture *tex = (D3D11Texture *)texture;

	if (!tex->IsUsable())
		tex->CreateTexture();

	if (attachment == DrawAttachment::Color0)
	{
		_format = tex->GetDXGIFormat();
		_color = tex->GetTexture2D();

		if (tex->GetType() == TextureType::Tex2DMultisample)
			_multisampled = true;
	}
	
	if (_rtv[_rtvCount]) _rtv[_rtvCount]->Release();
	_ctx->device->CreateRenderTargetView(tex->GetTexture2D(), NULL, &_rtv[_rtvCount++]);
}

void D3D11Framebuffer::AttachDepthTexture(class RTexture* texture)
{
	D3D11Texture *tex = (D3D11Texture *)texture;
	if (!tex->IsUsable()) tex->CreateTexture();
	if (_dsv) _dsv->Release();
	_ctx->device->CreateDepthStencilView(tex->GetTexture2D(), NULL, &_dsv);
}

void D3D11Framebuffer::AttachDepthStencilTexture(class RTexture* texture)
{
	D3D11Texture *tex = (D3D11Texture *)texture;
	if (!tex->IsUsable()) tex->CreateTexture();
	if (_dsv) _dsv->Release();

	D3D11_DEPTH_STENCIL_VIEW_DESC dsvDesc;
	ZeroMemory(&dsvDesc, sizeof(D3D11_DEPTH_STENCIL_VIEW_DESC));

	dsvDesc.Format = tex->GetDXGIFormat();
	dsvDesc.ViewDimension = tex->IsMultisampled() ? D3D11_DSV_DIMENSION_TEXTURE2DMS : D3D11_DSV_DIMENSION_TEXTURE2D;
	dsvDesc.Texture2D.MipSlice = 0;

	_ctx->device->CreateDepthStencilView(tex->GetTexture2D(), &dsvDesc, &_dsv);
}

void D3D11Framebuffer::CreateDepthBuffer()
{
	D3D11_TEXTURE2D_DESC desc;
	ZeroMemory(&desc, sizeof(desc));

	desc.Format = DXGI_FORMAT_D32_FLOAT;
	desc.Width = _width;
	desc.Height = _height;
	desc.ArraySize = 1;
	desc.MipLevels = 1;
	desc.SampleDesc.Count = 1;
	desc.BindFlags = D3D11_BIND_DEPTH_STENCIL;

	if (_depth) _depth->Release();
	_ctx->device->CreateTexture2D(&desc, nullptr, &_depth);

	if (_dsv) _dsv->Release();
	_ctx->device->CreateDepthStencilView(_depth, NULL, &_dsv);
}

void D3D11Framebuffer::CreateMultisampledDepthBuffer(int samples)
{
	D3D11_TEXTURE2D_DESC desc;
	ZeroMemory(&desc, sizeof(desc));

	desc.Format = DXGI_FORMAT_D32_FLOAT;
	desc.Width = _width;
	desc.Height = _height;
	desc.ArraySize = 1;
	desc.MipLevels = 1;
	desc.SampleDesc.Count = samples;
	desc.BindFlags = D3D11_BIND_DEPTH_STENCIL;

	if (_depth) _depth->Release();
	_ctx->device->CreateTexture2D(&desc, nullptr, &_depth);

	if (_dsv) _dsv->Release();
	_ctx->device->CreateDepthStencilView(_depth, NULL, &_dsv);
}

void D3D11Framebuffer::CreateStencilBuffer()
{
	D3D11_TEXTURE2D_DESC desc;
	ZeroMemory(&desc, sizeof(desc));

	desc.Format = DXGI_FORMAT_D32_FLOAT;
	desc.Width = _width;
	desc.Height = _height;
	desc.ArraySize = 1;
	desc.MipLevels = 1;
	desc.SampleDesc.Count = 1;
	desc.BindFlags = D3D11_BIND_DEPTH_STENCIL;

	if (_depth) _depth->Release();
	_ctx->device->CreateTexture2D(&desc, nullptr, &_depth);
	
	if (_dsv) _dsv->Release();
	_ctx->device->CreateDepthStencilView(_depth, NULL, &_dsv);
}

void D3D11Framebuffer::CreateMultisampledStencilBuffer(int samples)
{
	D3D11_TEXTURE2D_DESC desc;
	ZeroMemory(&desc, sizeof(desc));

	desc.Format = DXGI_FORMAT_D32_FLOAT;
	desc.Width = _width;
	desc.Height = _height;
	desc.ArraySize = 1;
	desc.MipLevels = 1;
	desc.SampleDesc.Count = samples;
	desc.BindFlags = D3D11_BIND_DEPTH_STENCIL;

	if (_depth) _depth->Release();
	_ctx->device->CreateTexture2D(&desc, nullptr, &_depth);

	if (_dsv) _dsv->Release();
	_ctx->device->CreateDepthStencilView(_depth, NULL, &_dsv);
}

void D3D11Framebuffer::CreateDepthStencilBuffer()
{
	D3D11_TEXTURE2D_DESC desc;
	ZeroMemory(&desc, sizeof(desc));

	desc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
	desc.Width = _width;
	desc.Height = _height;
	desc.ArraySize = 1;
	desc.MipLevels = 1;
	desc.SampleDesc.Count = 1;
	desc.BindFlags = D3D11_BIND_DEPTH_STENCIL;

	if (_depth) _depth->Release();
	_ctx->device->CreateTexture2D(&desc, nullptr, &_depth);

	if (_dsv) _dsv->Release();
	_ctx->device->CreateDepthStencilView(_depth, NULL, &_dsv);
}

void D3D11Framebuffer::CreateMultisampledDepthStencilBuffer(int samples)
{
	D3D11_TEXTURE2D_DESC desc;
	ZeroMemory(&desc, sizeof(desc));

	desc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
	desc.Width = _width;
	desc.Height = _height;
	desc.ArraySize = 1;
	desc.MipLevels = 1;
	desc.SampleDesc.Count = samples;
	desc.BindFlags = D3D11_BIND_DEPTH_STENCIL;

	if (_depth) _depth->Release();
	_ctx->device->CreateTexture2D(&desc, nullptr, &_depth);

	if (_dsv) _dsv->Release();
	_ctx->device->CreateDepthStencilView(_depth, NULL, &_dsv);
}

FramebufferStatus D3D11Framebuffer::CheckStatus() { return FramebufferStatus::Complete; }

void D3D11Framebuffer::Blit(RFramebuffer* dest, int srcX0, int srcY0, int srcX1, int srcY1, int dstX0, int dstY0, int dstX1, int dstY1)
{
	D3D11Framebuffer *d = (D3D11Framebuffer *)dest;
	if (_multisampled)
		_ctx->deviceContext->ResolveSubresource(d->GetColorTexture(), 0, _color, 0, _format);
	else
		_ctx->deviceContext->CopyResource(d->GetColorTexture(), _color);
}

void D3D11Framebuffer::CopyColor(RFramebuffer* dest, TextureFilter filter)
{
	D3D11Framebuffer *d = (D3D11Framebuffer *)dest;
	if (_multisampled)
		_ctx->deviceContext->ResolveSubresource(d->GetColorTexture(), 0, _color, 0, _format);
	else
		_ctx->deviceContext->CopyResource(d->GetColorTexture(), _color);
}

void D3D11Framebuffer::CopyDepth(RFramebuffer* dest)
{
	D3D11Framebuffer *d = (D3D11Framebuffer *)dest;
	if (_multisampled)
		_ctx->deviceContext->ResolveSubresource(d->GetColorTexture(), 0, _color, 0, _format);
	else
		_ctx->deviceContext->CopyResource(d->GetColorTexture(), _color);
}

void D3D11Framebuffer::CopyStencil(RFramebuffer* dest)
{
	D3D11Framebuffer *d = (D3D11Framebuffer *)dest;
	if (_multisampled)
		_ctx->deviceContext->ResolveSubresource(d->GetColorTexture(), 0, _color, 0, _format);
	else
		_ctx->deviceContext->CopyResource(d->GetColorTexture(), _color);
}

void D3D11Framebuffer::SetDrawBuffer(DrawAttachment attachment)
{
	if(attachment == DrawAttachment::None)
		_ctx->deviceContext->OMSetRenderTargets(0, nullptr, NULL);
	else
		_ctx->deviceContext->OMSetRenderTargets(1, &_rtv[(int)attachment - 1], NULL);
}

void D3D11Framebuffer::SetReadBuffer(DrawAttachment attachment)
{
	/*if (attachment == DrawAttachment::None)
		_ctx->deviceContext->OMSetRenderTargets(0, nullptr, NULL);
	else
		_ctx->deviceContext->OMSetRenderTargets(1, &_rtv[(int)attachment - 1], NULL);*/
}

void D3D11Framebuffer::SetDrawBuffers(int32_t n, DrawAttachment* buffers)
{
	_ctx->deviceContext->OMSetRenderTargets(n, _rtv, NULL);
}

D3D11Framebuffer::~D3D11Framebuffer()
{
	if (_depth)
		_depth->Release();

	if (_dsv)
		_dsv->Release();

	for (int i = 0; i < _rtvCount; ++i)
		_rtv[i]->Release();
}
