/* Neko Engine
 *
 * GLFramebuffer.h
 * Author: Alexandru Naiman
 *
 * OpenGL Renderer Implementation
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

#include <Renderer/RFramebuffer.h>

#include <vector>

#ifdef __APPLE__
	#include <OpenGL/gl3.h>
	#include <OpenGL/gl3ext.h>
#else
	#include "glad.h"
#endif

#define RBO_DEPTH			0
#define RBO_STENCIL			1
#define RBO_DEPTH_STENCIL	2

class GLFramebuffer :
	public RFramebuffer
{
public:
	GLFramebuffer(int width, int height, bool create = true);

	GLuint GetId() { return _id; }

	virtual void Bind(int location) override;
	virtual void Unbind() override;

	virtual void Resize(int width, int height) override;

	virtual void AttachTexture(DrawAttachment attachment, class RTexture* texture) override;
	virtual void AttachDepthTexture(class RTexture* texture) override;
	virtual void AttachDepthStencilTexture(class RTexture* texture) override;

	virtual void CreateDepthBuffer() override;
	virtual void CreateMultisampledDepthBuffer(int samples) override;
	virtual void CreateStencilBuffer() override;
	virtual void CreateMultisampledStencilBuffer(int samples) override;
	virtual void CreateDepthStencilBuffer() override;
	virtual void CreateMultisampledDepthStencilBuffer(int samples) override;

	virtual FramebufferStatus CheckStatus() override;

	virtual void Blit(RFramebuffer* dest, int srcX0, int srcY0, int srcX1, int srcY1, int dstX0, int dstY0, int dstX1, int dstY1) override;
	virtual void CopyColor(RFramebuffer* dest, TextureFilter filter) override;
	virtual void CopyDepth(RFramebuffer* dest) override;
	virtual void CopyStencil(RFramebuffer* dest) override;

	virtual void SetDrawBuffer(DrawAttachment attachment) override;
	virtual void SetDrawBuffers(int32_t n, DrawAttachment* buffers) override;

	virtual ~GLFramebuffer();

protected:
	GLuint _id;
	GLenum _lastTarget;
	std::vector<class GLTexture *> _colorTextures;
	GLuint _rbos[3];
};

class GLFramebuffer_NoDSA :
	public GLFramebuffer
{
public:
	GLFramebuffer_NoDSA(int width, int height);
	
	virtual void AttachTexture(DrawAttachment attachment, class RTexture* texture) override;
	virtual void AttachDepthTexture(class RTexture* texture) override;
	virtual void AttachDepthStencilTexture(class RTexture* texture) override;
	
	virtual void CreateDepthBuffer() override;
	virtual void CreateMultisampledDepthBuffer(int samples) override;
	virtual void CreateStencilBuffer() override;
	virtual void CreateMultisampledStencilBuffer(int samples) override;
	virtual void CreateDepthStencilBuffer() override;
	virtual void CreateMultisampledDepthStencilBuffer(int samples) override;
	
	virtual FramebufferStatus CheckStatus() override;
	
	virtual void Blit(RFramebuffer* dest, int srcX0, int srcY0, int srcX1, int srcY1, int dstX0, int dstY0, int dstX1, int dstY1) override;
	virtual void CopyColor(RFramebuffer* dest, TextureFilter filter) override;
	virtual void CopyDepth(RFramebuffer* dest) override;
	virtual void CopyStencil(RFramebuffer* dest) override;
	
	virtual void SetDrawBuffer(DrawAttachment attachment) override;
	virtual void SetDrawBuffers(int32_t n, DrawAttachment* buffers) override;
	
	virtual ~GLFramebuffer_NoDSA();
};
