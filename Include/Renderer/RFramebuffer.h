/* Neko Engine
 *
 * RFramebuffer.h
 * Author: Alexandru Naiman
 *
 * Rendering API abstraction
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

#include <stdint.h>
#include <stdlib.h>

#include <Renderer/RTexture.h>

// Conflicts with X11
#ifdef None
#undef None
#endif

#define FB_DRAW		0
#define FB_READ		1

enum class FramebufferStatus : int
{
	Complete = 0,
	IncompleteAttachment = 1,
	MissingAttachment,
	Unsupported,
	IncompleteMultisample
};

enum class DrawAttachment : int
{
	None = 0,
	Color0 = 1,
	Color1,
	Color2,
	Color3,
	Color4,
	Color5,
	Color6,
	Color7,
	Color8,
	Color9
};

#define R_DEFAULT_FRAMEBUFFER		(RFramebuffer *)0

class RFramebuffer
{
public:
	
	/**
	 * Create a framebuffer object.
	 * You must attach at least one color buffer.
	 */
	RFramebuffer(int width, int height) : _width(width), _height(height), _drawBuffers(nullptr) { };

	/**
	 * Bind the framebuffer.
	 * location must be either FB_DRAW or FB_READ
	 */
	virtual void Bind(int location) = 0;
	
	/**
	 * Bind the default framebuffer
	 */
	virtual void Unbind() = 0;

	/**
	 * Resize the framebuffer's attachments
	 * The application should check the framebuffer's status after resize.
	 */
	virtual void Resize(int width, int height) = 0;

	/**
	 * Attach color texture
	 */
	virtual void AttachTexture(DrawAttachment attachment, class RTexture* texture) = 0;
	
	/**
	 * Attach depth texture. Texture format must be DEPTH & type must be one of: DEPTH_16, DEPTH_24, DEPTH_32 or DEPTH_32F
	 */
	virtual void AttachDepthTexture(class RTexture* texture) = 0;
	
	/**
	 * Attach depth & stencil texture. Texture type format be DEPTH_STENCIL & type must be either DEPTH24_STENCIL8 or DEPTH32F_STENCIL8
	 */
	virtual void AttachDepthStencilTexture(class RTexture* texture) = 0;

	/**
	 * Attach a 24-bit depth renderbuffer
	 */
	virtual void CreateDepthBuffer() = 0;

	/**
	* Attach a multisampled 24-bit depth renderbuffer
	*/
	virtual void CreateMultisampledDepthBuffer(int samples) = 0;
	
	/**
	 * Attach a 8-bit stencil renderbuffer
	 */
	virtual void CreateStencilBuffer() = 0;

	/**
	* Attach a multisampled 8-bit stencil renderbuffer
	*/
	virtual void CreateMultisampledStencilBuffer(int samples) = 0;
	
	/**
	 * Attach a 24-bit depth & 8-bit stencil renderbuffer
	 */
	virtual void CreateDepthStencilBuffer() = 0;

	/**
	* Attach a multisampled 24-bit depth & 8-bit stencil renderbuffer
	*/
	virtual void CreateMultisampledDepthStencilBuffer(int samples) = 0;
	
	/**
	 * Check the framebuffer's completeness
	 */
	virtual FramebufferStatus CheckStatus() = 0;

	/**
	 * Copy the first color, depth & stencil attachments
	 */
	virtual void Blit(RFramebuffer* dest, int srcX0, int srcY0, int srcX1, int srcY1, int dstX0, int dstY0, int dstX1, int dstY1) = 0;
	
	/**
	 * Copy the contents of the first color attachment
	 */
	virtual void CopyColor(RFramebuffer* dest, TextureFilter filter) = 0;
	
	/**
	 * Copy the contents of the depth buffer
	 */
	virtual void CopyDepth(RFramebuffer* dest) = 0;
	
	/**
	 * Copy the contents of the stencil buffer
	 */
	virtual void CopyStencil(RFramebuffer* dest) = 0;

	virtual void SetDrawBuffer(DrawAttachment attachment) = 0;
	virtual void SetDrawBuffers(int32_t n, DrawAttachment* buffers) = 0;
	
	/**
	 * Release resources
	 */
	virtual ~RFramebuffer() { free(_drawBuffers); };

protected:
	int _width, _height;
	int *_drawBuffers;
};

