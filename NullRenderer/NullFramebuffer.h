/* NekoEngine
 *
 * NullFramebuffer.h
 * Author: Alexandru Naiman
 *
 * Null Renderer Implementation
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

#include "NullRenderer.h"
#include <Renderer/RFramebuffer.h>

class NullFramebuffer :
	public RFramebuffer
{
public:
	NullFramebuffer(int width, int height, bool create = true) : RFramebuffer(width, height) { }

	virtual void Bind(int location) override { NullRenderer::SetBoundFramebuffer(this); }
	virtual void Unbind() override { NullRenderer::SetBoundFramebuffer(R_DEFAULT_FRAMEBUFFER); }

	virtual void Resize(int width, int height) override { }

	virtual void AttachTexture(DrawAttachment attachment, class RTexture* texture) override { }
	virtual void AttachDepthTexture(class RTexture* texture) override { }
	virtual void AttachDepthStencilTexture(class RTexture* texture) override { }

	virtual void CreateDepthBuffer() override { }
	virtual void CreateMultisampledDepthBuffer(int samples) override { }
	virtual void CreateStencilBuffer() override { }
	virtual void CreateMultisampledStencilBuffer(int samples) override { }
	virtual void CreateDepthStencilBuffer() override { }
	virtual void CreateMultisampledDepthStencilBuffer(int samples) override { }

	virtual FramebufferStatus CheckStatus() override { return FramebufferStatus::Complete; }

	virtual void Blit(RFramebuffer* dest, int srcX0, int srcY0, int srcX1, int srcY1, int dstX0, int dstY0, int dstX1, int dstY1) override { }
	virtual void CopyColor(RFramebuffer* dest, TextureFilter filter) override { }
	virtual void CopyDepth(RFramebuffer* dest) override { }
	virtual void CopyStencil(RFramebuffer* dest) override { }

	virtual void SetDrawBuffer(DrawAttachment attachment) override { }
	virtual void SetDrawBuffers(int32_t n, DrawAttachment* buffers) override { }

	virtual ~NullFramebuffer() { }
};