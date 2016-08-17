/* NekoEngine
 *
 * SSAO.cpp
 * Author: Alexandru Naiman
 *
 * HBAO+
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

#include <stdlib.h>

#include <Engine/HBAO.h>
#include <Engine/DeferredBuffer.h>

#define HBAO_MODULE	"HBAO"

HBAO::HBAO(int width, int height)
{
	_fbo = nullptr;
	_texture = nullptr;
	_fboWidth = width;
	_fboHeight = height;
}

bool HBAO::Initialize() noexcept
{
	if (!Engine::GetRenderer()->IsHBAOSupported())
	{
		Logger::Log(HBAO_MODULE, LOG_CRITICAL, "HBAO+ not supported by the current renderer");
		return false;
	}

	if (!Engine::GetRenderer()->InitializeHBAO())
	{
		Logger::Log(HBAO_MODULE, LOG_CRITICAL, "Failed to initialize HBAO");
		return false;
	}

	if ((_fbo = Engine::GetRenderer()->CreateFramebuffer(_fboWidth, _fboHeight)) == nullptr)
	{
		Logger::Log(HBAO_MODULE, LOG_CRITICAL, "Failed to create framebuffer");
		return false;
	}

	if ((_texture = Engine::GetRenderer()->CreateTexture(TextureType::Tex2D)) == nullptr)
	{
		Logger::Log(HBAO_MODULE, LOG_CRITICAL, "Failed to create texture");
		return false;
	}

	_texture->SetStorage2D(1, TextureSizedFormat::RGBA_16F, _fboWidth, _fboHeight);
	_texture->SetMinFilter(TextureFilter::Linear);
	_texture->SetMagFilter(TextureFilter::Linear);
	_texture->SetWrapS(TextureWrap::ClampToEdge);
	_texture->SetWrapT(TextureWrap::ClampToEdge);

	_fbo->AttachTexture(DrawAttachment::Color0, _texture);
	if (_fbo->CheckStatus() != FramebufferStatus::Complete)
	{
		Logger::Log(HBAO_MODULE, LOG_CRITICAL, "Framebuffer incomplete");
		return false;
	}

	memset(&_args, 0x0, sizeof(RHBAOArgs));

	return true;
}

void HBAO::Render() noexcept
{
	RFramebuffer* fbo = Engine::GetRenderer()->GetBoundFramebuffer();

	_args.depthTexture = DeferredBuffer::GetDepthTexture();
	_args.normalTexture = DeferredBuffer::GetNormalTexture();

	Engine::GetRenderer()->RenderHBAO(&_args, _fbo);

	fbo->Bind(FB_DRAW);
}

void HBAO::Resize(int width, int height) noexcept
{
	_fboWidth = width;
	_fboHeight = height;

	_fbo->Resize(_fboWidth, _fboHeight);
}

HBAO::~HBAO()
{
	delete _texture;
	_texture = nullptr;
}