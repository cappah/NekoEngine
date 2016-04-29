/* Neko Engine
 *
 * GLFramebuffer.cpp
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

#include "GLFramebuffer.h"
#include "GLRenderer.h"
#include "GLTexture.h"

#include <string.h>

GLenum GL_FramebufferTarget[2]
{
	GL_DRAW_FRAMEBUFFER,
	GL_READ_FRAMEBUFFER
};

GLenum GL_Attachments[11]
{
	GL_NONE,
	GL_COLOR_ATTACHMENT0,
	GL_COLOR_ATTACHMENT1,
	GL_COLOR_ATTACHMENT2,
	GL_COLOR_ATTACHMENT3,
	GL_COLOR_ATTACHMENT4,
	GL_COLOR_ATTACHMENT5,
	GL_COLOR_ATTACHMENT6,
	GL_COLOR_ATTACHMENT7,
	GL_COLOR_ATTACHMENT8,
	GL_COLOR_ATTACHMENT9
};

extern GLenum GL_TexFilter[];

GLFramebuffer::GLFramebuffer(int width, int height, bool create)
	: RFramebuffer(width, height)
{
	_lastTarget = GL_DRAW_FRAMEBUFFER;
	memset(_rbos, 0x0, sizeof(GLuint) * 3);

	if (!create)
		return;

	GL_CHECK(glCreateFramebuffers(1, &_id));
}

void GLFramebuffer::Bind(int location)
{
	_lastTarget = GL_FramebufferTarget[location];
	GL_CHECK(glBindFramebuffer(_lastTarget, _id));
	GL_CHECK(glViewport(0, 0, _width, _height));
	GLRenderer::SetBoundFramebuffer(this);
}

void GLFramebuffer::Unbind()
{
	GL_CHECK(glBindFramebuffer(_lastTarget, 0));
	GLRenderer::SetBoundFramebuffer(R_DEFAULT_FRAMEBUFFER);
}

void GLFramebuffer::Resize(int width, int height)
{
	_width = width;
	_height = height;

	for (int i = 0; i < _colorTextures.size(); i++)
	{
		GLTexture *tex = _colorTextures[i];
		tex->Resize2D(width, height);
		AttachTexture((DrawAttachment)(i + 1), tex);
	}

	if (_rbos[RBO_DEPTH] != 0)
	{
		GL_CHECK(glDeleteBuffers(1, &_rbos[RBO_DEPTH]));
		CreateDepthBuffer();
	}

	if (_rbos[RBO_STENCIL] != 0)
	{
		GL_CHECK(glDeleteBuffers(1, &_rbos[RBO_STENCIL]));
		CreateStencilBuffer();
	}

	if (_rbos[RBO_DEPTH_STENCIL] != 0)
	{
		GL_CHECK(glDeleteBuffers(1, &_rbos[RBO_DEPTH_STENCIL]));
		CreateDepthStencilBuffer();
	}
}

void GLFramebuffer::AttachTexture(DrawAttachment attachment, class RTexture* texture)
{
	GL_CHECK(glNamedFramebufferTexture(_id, GL_Attachments[(int)attachment], ((GLTexture *)texture)->GetId(), 0));
}

void GLFramebuffer::AttachDepthTexture(class RTexture* texture)
{
	GL_CHECK(glNamedFramebufferTexture(_id, GL_DEPTH_ATTACHMENT, ((GLTexture *)texture)->GetId(), 0));
}

void GLFramebuffer::AttachDepthStencilTexture(class RTexture* texture)
{
	GL_CHECK(glNamedFramebufferTexture(_id, GL_DEPTH_STENCIL_ATTACHMENT, ((GLTexture *)texture)->GetId(), 0));
}

void GLFramebuffer::CreateDepthBuffer()
{
	GL_CHECK(glCreateRenderbuffers(1, &_rbos[RBO_DEPTH]));
	GL_CHECK(glNamedRenderbufferStorage(_rbos[RBO_DEPTH], GL_DEPTH_COMPONENT24, _width, _height));
	GL_CHECK(glNamedFramebufferRenderbuffer(_id, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, _rbos[RBO_DEPTH]));
}

void GLFramebuffer::CreateMultisampledDepthBuffer(int samples)
{
	GL_CHECK(glCreateRenderbuffers(1, &_rbos[RBO_DEPTH]));
	GL_CHECK(glNamedRenderbufferStorageMultisample(_rbos[RBO_DEPTH], samples, GL_DEPTH_COMPONENT24, _width, _height));
	GL_CHECK(glNamedFramebufferRenderbuffer(_id, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, _rbos[RBO_DEPTH]));
}

void GLFramebuffer::CreateStencilBuffer()
{
	GL_CHECK(glCreateRenderbuffers(1, &_rbos[RBO_STENCIL]));
	GL_CHECK(glNamedRenderbufferStorage(_rbos[RBO_STENCIL], GL_STENCIL_INDEX8, _width, _height));
	GL_CHECK(glNamedFramebufferRenderbuffer(_id, GL_STENCIL_ATTACHMENT, GL_RENDERBUFFER, _rbos[RBO_STENCIL]));
}

void GLFramebuffer::CreateMultisampledStencilBuffer(int samples)
{
	GL_CHECK(glCreateRenderbuffers(1, &_rbos[RBO_STENCIL]));
	GL_CHECK(glNamedRenderbufferStorageMultisample(_rbos[RBO_STENCIL], samples, GL_STENCIL_INDEX8, _width, _height));
	GL_CHECK(glNamedFramebufferRenderbuffer(_id, GL_STENCIL_ATTACHMENT, GL_RENDERBUFFER, _rbos[RBO_STENCIL]));
}

void GLFramebuffer::CreateDepthStencilBuffer()
{
	GL_CHECK(glCreateRenderbuffers(1, &_rbos[RBO_DEPTH_STENCIL]));
	GL_CHECK(glNamedRenderbufferStorage(_rbos[RBO_DEPTH_STENCIL], GL_DEPTH24_STENCIL8, _width, _height));
	GL_CHECK(glNamedFramebufferRenderbuffer(_id, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, _rbos[RBO_DEPTH_STENCIL]));
}

void GLFramebuffer::CreateMultisampledDepthStencilBuffer(int samples)
{
	GL_CHECK(glCreateRenderbuffers(1, &_rbos[RBO_DEPTH_STENCIL]));
	GL_CHECK(glNamedRenderbufferStorageMultisample(_rbos[RBO_DEPTH_STENCIL], samples, GL_DEPTH24_STENCIL8, _width, _height));
	GL_CHECK(glNamedFramebufferRenderbuffer(_id, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, _rbos[RBO_DEPTH_STENCIL]));
}

FramebufferStatus GLFramebuffer::CheckStatus()
{
	GLenum status;
	GL_CHECK(status = glCheckNamedFramebufferStatus(_id, GL_FRAMEBUFFER));

	if (status == GL_FRAMEBUFFER_COMPLETE)
		return FramebufferStatus::Complete;
	
	if (status == GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT)
		return FramebufferStatus::IncompleteAttachment;

	if (status == GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT)
		return FramebufferStatus::MissingAttachment;

	if (status == GL_FRAMEBUFFER_INCOMPLETE_MULTISAMPLE)
		return FramebufferStatus::IncompleteMultisample;

	return FramebufferStatus::Unsupported;
}

void GLFramebuffer::Blit(RFramebuffer* dest, int srcX0, int srcY0, int srcX1, int srcY1, int dstX0, int dstY0, int dstX1, int dstY1)
{
	GLuint destFbo = 0;

	if (dest != R_DEFAULT_FRAMEBUFFER)
	{
		GLFramebuffer *d = (GLFramebuffer *)dest;
		destFbo = d->GetId();
	}

	GL_CHECK(glBlitNamedFramebuffer(_id, destFbo, srcX0, srcY0, srcX1, srcY1, dstX0, dstY0, dstX1, dstY1, GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT, GL_NEAREST));
}

void GLFramebuffer::CopyColor(RFramebuffer* dest, TextureFilter filter)
{
	GLuint destFbo = 0;

	if (dest != R_DEFAULT_FRAMEBUFFER)
	{
		GLFramebuffer *d = (GLFramebuffer *)dest;
		destFbo = d->GetId();
	}

	GL_CHECK(glBlitNamedFramebuffer(_id, destFbo, 0, 0, _width, _height, 0, 0, _width, _height, GL_COLOR_BUFFER_BIT, GL_TexFilter[(int)filter]));
}

void GLFramebuffer::CopyDepth(RFramebuffer* dest)
{
	GLuint destFbo = 0;

	if (dest != R_DEFAULT_FRAMEBUFFER)
	{
		GLFramebuffer *d = (GLFramebuffer *)dest;
		destFbo = d->GetId();
	}

	GL_CHECK(glBlitNamedFramebuffer(_id, destFbo, 0, 0, _width, _height, 0, 0, _width, _height, GL_DEPTH_BUFFER_BIT, GL_NEAREST));
}

void GLFramebuffer::CopyStencil(RFramebuffer* dest)
{
	GLuint destFbo = 0;

	if (dest != R_DEFAULT_FRAMEBUFFER)
	{
		GLFramebuffer *d = (GLFramebuffer *)dest;
		destFbo = d->GetId();
	}

	GL_CHECK(glBlitNamedFramebuffer(_id, destFbo, 0, 0, _width, _height, 0, 0, _width, _height, GL_STENCIL_BUFFER_BIT, GL_NEAREST));
}

void GLFramebuffer::SetDrawBuffer(DrawAttachment attachment)
{
	GL_CHECK(glNamedFramebufferDrawBuffer(_id, GL_Attachments[(int)attachment]));
}

void GLFramebuffer::SetDrawBuffers(int32_t n, DrawAttachment* buffers)
{
	GLenum drawBuffers[11];

	for (int i = 0; i < n; i++)
		drawBuffers[i] = GL_Attachments[(int)buffers[i]];

	GL_CHECK(glNamedFramebufferDrawBuffers(_id, n, drawBuffers));
}

GLFramebuffer::~GLFramebuffer()
{
	GL_CHECK(glDeleteFramebuffers(1, &_id));
}

// Non-DSA variant

GLFramebuffer_NoDSA::GLFramebuffer_NoDSA(int width, int height)
	: GLFramebuffer(width, height, false)
{
	GL_CHECK(glGenFramebuffers(1, &_id));
}

void GLFramebuffer_NoDSA::AttachTexture(DrawAttachment attachment, class RTexture* texture)
{
	GL_CHECK(glNamedFramebufferTexture(_id, GL_Attachments[(int)attachment], ((GLTexture *)texture)->GetId(), 0));
}

void GLFramebuffer_NoDSA::AttachDepthTexture(class RTexture* texture)
{
	GL_CHECK(glNamedFramebufferTexture(_id, GL_DEPTH_ATTACHMENT, ((GLTexture *)texture)->GetId(), 0));
}

void GLFramebuffer_NoDSA::AttachDepthStencilTexture(class RTexture* texture)
{
	GL_CHECK(glNamedFramebufferTexture(_id, GL_DEPTH_STENCIL_ATTACHMENT, ((GLTexture *)texture)->GetId(), 0));
}

void GLFramebuffer_NoDSA::CreateDepthBuffer()
{
	GL_CHECK(glCreateRenderbuffers(1, &_rbos[RBO_DEPTH]));
	GL_CHECK(glNamedRenderbufferStorage(_rbos[RBO_DEPTH], GL_DEPTH_COMPONENT24, _width, _height));
	GL_CHECK(glNamedFramebufferRenderbuffer(_id, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, _rbos[RBO_DEPTH]));
}

void GLFramebuffer_NoDSA::CreateMultisampledDepthBuffer(int samples)
{
	GL_CHECK(glCreateRenderbuffers(1, &_rbos[RBO_DEPTH]));
	GL_CHECK(glNamedRenderbufferStorageMultisample(_rbos[RBO_DEPTH], samples, GL_DEPTH_COMPONENT24, _width, _height));
	GL_CHECK(glNamedFramebufferRenderbuffer(_id, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, _rbos[RBO_DEPTH]));
}

void GLFramebuffer_NoDSA::CreateStencilBuffer()
{
	GL_CHECK(glCreateRenderbuffers(1, &_rbos[RBO_STENCIL]));
	GL_CHECK(glNamedRenderbufferStorage(_rbos[RBO_STENCIL], GL_STENCIL_INDEX8, _width, _height));
	GL_CHECK(glNamedFramebufferRenderbuffer(_id, GL_STENCIL_ATTACHMENT, GL_RENDERBUFFER, _rbos[RBO_STENCIL]));
}

void GLFramebuffer_NoDSA::CreateMultisampledStencilBuffer(int samples)
{
	GL_CHECK(glCreateRenderbuffers(1, &_rbos[RBO_STENCIL]));
	GL_CHECK(glNamedRenderbufferStorageMultisample(_rbos[RBO_STENCIL], samples, GL_STENCIL_INDEX8, _width, _height));
	GL_CHECK(glNamedFramebufferRenderbuffer(_id, GL_STENCIL_ATTACHMENT, GL_RENDERBUFFER, _rbos[RBO_STENCIL]));
}

void GLFramebuffer_NoDSA::CreateDepthStencilBuffer()
{
	GL_CHECK(glCreateRenderbuffers(1, &_rbos[RBO_DEPTH_STENCIL]));
	GL_CHECK(glNamedRenderbufferStorage(_rbos[RBO_DEPTH_STENCIL], GL_DEPTH24_STENCIL8, _width, _height));
	GL_CHECK(glNamedFramebufferRenderbuffer(_id, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, _rbos[RBO_DEPTH_STENCIL]));
}

void GLFramebuffer_NoDSA::CreateMultisampledDepthStencilBuffer(int samples)
{
	GL_CHECK(glCreateRenderbuffers(1, &_rbos[RBO_DEPTH_STENCIL]));
	GL_CHECK(glNamedRenderbufferStorageMultisample(_rbos[RBO_DEPTH_STENCIL], samples, GL_DEPTH24_STENCIL8, _width, _height));
	GL_CHECK(glNamedFramebufferRenderbuffer(_id, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, _rbos[RBO_DEPTH_STENCIL]));
}

FramebufferStatus GLFramebuffer_NoDSA::CheckStatus()
{
	GLenum status;
	GL_CHECK(status = glCheckNamedFramebufferStatus(_id, GL_FRAMEBUFFER));

	if (status == GL_FRAMEBUFFER_COMPLETE)
		return FramebufferStatus::Complete;

	if (status == GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT)
		return FramebufferStatus::IncompleteAttachment;

	if (status == GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT)
		return FramebufferStatus::MissingAttachment;

	if (status == GL_FRAMEBUFFER_INCOMPLETE_MULTISAMPLE)
		return FramebufferStatus::IncompleteMultisample;

	return FramebufferStatus::Unsupported;
}

void GLFramebuffer_NoDSA::Blit(RFramebuffer* dest, int srcX0, int srcY0, int srcX1, int srcY1, int dstX0, int dstY0, int dstX1, int dstY1)
{
	GLuint destFbo = 0;

	if (dest != R_DEFAULT_FRAMEBUFFER)
	{
		GLFramebuffer *d = (GLFramebuffer *)dest;
		destFbo = d->GetId();
	}

	GL_CHECK(glBlitNamedFramebuffer(_id, destFbo, srcX0, srcY0, srcX1, srcY1, dstX0, dstY0, dstX1, dstY1, GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT, GL_NEAREST));
}

void GLFramebuffer_NoDSA::CopyColor(RFramebuffer* dest, TextureFilter filter)
{
	GLuint destFbo = 0;

	if (dest != R_DEFAULT_FRAMEBUFFER)
	{
		GLFramebuffer *d = (GLFramebuffer *)dest;
		destFbo = d->GetId();
	}

	GL_CHECK(glBlitNamedFramebuffer(_id, destFbo, 0, 0, _width, _height, 0, 0, _width, _height, GL_COLOR_BUFFER_BIT, GL_TexFilter[(int)filter]));
}

void GLFramebuffer_NoDSA::CopyDepth(RFramebuffer* dest)
{
	GLuint destFbo = 0;

	if (dest != R_DEFAULT_FRAMEBUFFER)
	{
		GLFramebuffer *d = (GLFramebuffer *)dest;
		destFbo = d->GetId();
	}

	GL_CHECK(glBlitNamedFramebuffer(_id, destFbo, 0, 0, _width, _height, 0, 0, _width, _height, GL_DEPTH_BUFFER_BIT, GL_NEAREST));
}

void GLFramebuffer_NoDSA::CopyStencil(RFramebuffer* dest)
{
	GLuint destFbo = 0;

	if (dest != R_DEFAULT_FRAMEBUFFER)
	{
		GLFramebuffer *d = (GLFramebuffer *)dest;
		destFbo = d->GetId();
	}

	GL_CHECK(glBlitNamedFramebuffer(_id, destFbo, 0, 0, _width, _height, 0, 0, _width, _height, GL_STENCIL_BUFFER_BIT, GL_NEAREST));
}

void GLFramebuffer_NoDSA::SetDrawBuffer(DrawAttachment attachment)
{
	GL_CHECK(glNamedFramebufferDrawBuffer(_id, GL_Attachments[(int)attachment]));
}

void GLFramebuffer_NoDSA::SetDrawBuffers(int32_t n, DrawAttachment* buffers)
{
	GLenum drawBuffers[11];

	for (int i = 0; i < n; i++)
		drawBuffers[i] = GL_Attachments[(int)buffers[i]];

	GL_CHECK(glNamedFramebufferDrawBuffers(_id, n, drawBuffers));
}

GLFramebuffer_NoDSA::~GLFramebuffer_NoDSA()
{
	GL_CHECK(glDeleteFramebuffers(1, &_id));
}