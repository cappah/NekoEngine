/* NekoEngine
 *
 * GLESFramebuffer.cpp
 * Author: Alexandru Naiman
 *
 * OpenGL|ES 3 Renderer Implementation
 *
 * -----------------------------------------------------------------------------
 *
 * Copyright (c) 2015-2017, Alexandru Naiman
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

#include "GLESFramebuffer.h"
#include "GLESRenderer.h"
#include "GLESTexture.h"

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

GLESFramebuffer::GLESFramebuffer(int width, int height)
	: RFramebuffer(width, height)
{
    _lastTarget = GL_FRAMEBUFFER;
	GL_CHECK(glGenFramebuffers(1, &_id));
	memset(_rbos, 0x0, sizeof(GLuint) * 3);
}

void GLESFramebuffer::Bind(int location)
{
	_lastTarget = GL_FramebufferTarget[location];
	GL_CHECK(glBindFramebuffer(_lastTarget, _id));
//	GL_CHECK(glViewport(0, 0, _width, _height));
	GLESRenderer::SetBoundFramebuffer(this);
}

void GLESFramebuffer::Unbind()
{
	GLESRenderer::SetBoundFramebuffer(nullptr);
}

void GLESFramebuffer::Resize(int width, int height)
{
	_width = width;
	_height = height;

	GL_CHECK(glBindFramebuffer(GL_FRAMEBUFFER, _id));
	for (GLESFramebufferAttachmentInfo &info : _attachmentInfo)
	{
		if (info.tex->GetWidth() != width || info.tex->GetHeight() != height)
			info.tex->Resize2D(width, height);

		GL_CHECK(glFramebufferTexture2D(GL_FRAMEBUFFER, info.attachment, GL_TEXTURE_2D, info.tex->GetId(), 0));
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

void GLESFramebuffer::AttachTexture(DrawAttachment attachment, class RTexture* texture)
{
	GL_CHECK(glBindFramebuffer(GL_FRAMEBUFFER, _id));
	GL_CHECK(glFramebufferTexture2D(GL_FRAMEBUFFER, GL_Attachments[(int)attachment], GL_TEXTURE_2D, ((GLESTexture *)texture)->GetId(), 0));
	_attachmentInfo.push_back({ GL_Attachments[(int)attachment], (GLESTexture *)texture });
}

void GLESFramebuffer::AttachDepthTexture(class RTexture* texture)
{
	GL_CHECK(glBindFramebuffer(GL_FRAMEBUFFER, _id));
	GL_CHECK(glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, ((GLESTexture *)texture)->GetId(), 0));
	_attachmentInfo.push_back({ GL_DEPTH_ATTACHMENT, (GLESTexture *)texture });
}

void GLESFramebuffer::AttachDepthStencilTexture(class RTexture* texture)
{
	GL_CHECK(glBindFramebuffer(GL_FRAMEBUFFER, _id));
	GL_CHECK(glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_TEXTURE_2D, ((GLESTexture *)texture)->GetId(), 0));
	_attachmentInfo.push_back({ GL_DEPTH_STENCIL_ATTACHMENT, (GLESTexture *)texture });
}

void GLESFramebuffer::CreateDepthBuffer()
{
	GL_CHECK(glGenRenderbuffers(1, &_rbos[RBO_DEPTH]));
	GL_CHECK(glBindRenderbuffer(GL_RENDERBUFFER, _rbos[RBO_DEPTH]));
	GL_CHECK(glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, _width, _height));
	GL_CHECK(glBindFramebuffer(GL_FRAMEBUFFER, _id));
	GL_CHECK(glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, _rbos[RBO_DEPTH]));
}

void GLESFramebuffer::CreateMultisampledDepthBuffer(int samples)
{
	GL_CHECK(glGenRenderbuffers(1, &_rbos[RBO_DEPTH]));
	GL_CHECK(glBindRenderbuffer(GL_RENDERBUFFER, _rbos[RBO_DEPTH]));
	GL_CHECK(glRenderbufferStorageMultisample(GL_RENDERBUFFER, samples, GL_DEPTH_COMPONENT24, _width, _height));
	GL_CHECK(glBindFramebuffer(GL_FRAMEBUFFER, _id));
	GL_CHECK(glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, _rbos[RBO_DEPTH]));
}

void GLESFramebuffer::CreateStencilBuffer()
{
	GL_CHECK(glGenRenderbuffers(1, &_rbos[RBO_STENCIL]));
	GL_CHECK(glBindRenderbuffer(GL_RENDERBUFFER, _rbos[RBO_STENCIL]));
	GL_CHECK(glRenderbufferStorage(GL_RENDERBUFFER, GL_STENCIL_INDEX8, _width, _height));
	GL_CHECK(glBindFramebuffer(GL_FRAMEBUFFER, _id));
	GL_CHECK(glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_STENCIL_ATTACHMENT, GL_RENDERBUFFER, _rbos[RBO_STENCIL]));
}

void GLESFramebuffer::CreateMultisampledStencilBuffer(int samples)
{
	GL_CHECK(glGenRenderbuffers(1, &_rbos[RBO_STENCIL]));
	GL_CHECK(glBindRenderbuffer(GL_RENDERBUFFER, _rbos[RBO_STENCIL]));
	GL_CHECK(glRenderbufferStorageMultisample(GL_RENDERBUFFER, samples, GL_STENCIL_INDEX8, _width, _height));
	GL_CHECK(glBindFramebuffer(GL_FRAMEBUFFER, _id));
	GL_CHECK(glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_STENCIL_ATTACHMENT, GL_RENDERBUFFER, _rbos[RBO_STENCIL]));
}

void GLESFramebuffer::CreateDepthStencilBuffer()
{
	GL_CHECK(glGenRenderbuffers(1, &_rbos[RBO_DEPTH_STENCIL]));
	GL_CHECK(glBindRenderbuffer(GL_RENDERBUFFER, _rbos[RBO_DEPTH_STENCIL]));
	GL_CHECK(glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, _width, _height));
	GL_CHECK(glBindFramebuffer(GL_FRAMEBUFFER, _id));
	GL_CHECK(glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, _rbos[RBO_DEPTH_STENCIL]));
}

void GLESFramebuffer::CreateMultisampledDepthStencilBuffer(int samples)
{
	GL_CHECK(glGenRenderbuffers(1, &_rbos[RBO_DEPTH_STENCIL]));
	GL_CHECK(glBindRenderbuffer(GL_RENDERBUFFER, _rbos[RBO_DEPTH_STENCIL]));
	GL_CHECK(glRenderbufferStorageMultisample(GL_RENDERBUFFER, samples, GL_DEPTH24_STENCIL8, _width, _height));
	GL_CHECK(glBindFramebuffer(GL_FRAMEBUFFER, _id));
	GL_CHECK(glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_STENCIL_ATTACHMENT, GL_RENDERBUFFER, _rbos[RBO_DEPTH_STENCIL]));
}

FramebufferStatus GLESFramebuffer::CheckStatus()
{
	GLenum status;
	GL_CHECK(glBindFramebuffer(GL_FRAMEBUFFER, _id));
	GL_CHECK(status = glCheckFramebufferStatus(GL_FRAMEBUFFER));

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

void GLESFramebuffer::Blit(RFramebuffer* dest, int srcX0, int srcY0, int srcX1, int srcY1, int dstX0, int dstY0, int dstX1, int dstY1)
{
	GLuint destFbo = 0;

	if (dest != R_DEFAULT_FRAMEBUFFER)
	{
		GLESFramebuffer *d = (GLESFramebuffer *)dest;
		destFbo = d->GetId();
	}

	GL_CHECK(glBindFramebuffer(GL_DRAW_FRAMEBUFFER, destFbo));
	GL_CHECK(glBindFramebuffer(GL_READ_FRAMEBUFFER, _id));

	GL_CHECK(glBlitFramebuffer(srcX0, srcY0, srcX1, srcY1, dstX0, dstY0, dstX1, dstY1, GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT, GL_NEAREST));
}

void GLESFramebuffer::CopyColor(RFramebuffer* dest, TextureFilter filter)
{
	GLuint destFbo = 0;

	if (dest != R_DEFAULT_FRAMEBUFFER)
	{
		GLESFramebuffer *d = (GLESFramebuffer *)dest;
		destFbo = d->GetId();
	}

	GL_CHECK(glBindFramebuffer(GL_DRAW_FRAMEBUFFER, destFbo));
	GL_CHECK(glBindFramebuffer(GL_READ_FRAMEBUFFER, _id));

	GL_CHECK(glBlitFramebuffer(0, 0, _width, _height, 0, 0, _width, _height, GL_COLOR_BUFFER_BIT, GL_TexFilter[(int)filter]));
}

void GLESFramebuffer::CopyDepth(RFramebuffer* dest)
{
	GLuint destFbo = 0;

	if (dest != R_DEFAULT_FRAMEBUFFER)
	{
		GLESFramebuffer *d = (GLESFramebuffer *)dest;
		destFbo = d->GetId();
	}

	GL_CHECK(glBindFramebuffer(GL_DRAW_FRAMEBUFFER, destFbo));
	GL_CHECK(glBindFramebuffer(GL_READ_FRAMEBUFFER, _id));

	GL_CHECK(glBlitFramebuffer(0, 0, _width, _height, 0, 0, _width, _height, GL_DEPTH_BUFFER_BIT, GL_NEAREST));
}

void GLESFramebuffer::CopyStencil(RFramebuffer* dest)
{
	GLuint destFbo = 0;

	if (dest != R_DEFAULT_FRAMEBUFFER)
	{
		GLESFramebuffer *d = (GLESFramebuffer *)dest;
		destFbo = d->GetId();
	}

	GL_CHECK(glBindFramebuffer(GL_DRAW_FRAMEBUFFER, destFbo));
	GL_CHECK(glBindFramebuffer(GL_READ_FRAMEBUFFER, _id));

	GL_CHECK(glBlitFramebuffer(0, 0, _width, _height, 0, 0, _width, _height, GL_STENCIL_BUFFER_BIT, GL_NEAREST));
}

void GLESFramebuffer::SetDrawBuffer(DrawAttachment attachment)
{
	GL_CHECK(glBindFramebuffer(GL_FRAMEBUFFER, _id));
	GL_CHECK(glDrawBuffers(1, &GL_Attachments[(int)attachment]));
}

void GLESFramebuffer::SetReadBuffer(DrawAttachment attachment)
{
	GL_CHECK(glBindFramebuffer(GL_FRAMEBUFFER, _id));
	GL_CHECK(glReadBuffer(GL_Attachments[(int)attachment]));
}

void GLESFramebuffer::SetDrawBuffers(int32_t n, DrawAttachment* buffers)
{
	GLenum drawBuffers[11];

	for (int i = 0; i < n; i++)
		drawBuffers[i] = GL_Attachments[(int)buffers[i]];

	GL_CHECK(glBindFramebuffer(GL_FRAMEBUFFER, _id));
	GL_CHECK(glDrawBuffers(n, drawBuffers));
}

GLESFramebuffer::~GLESFramebuffer()
{
	GL_CHECK(glDeleteFramebuffers(1, &_id));
}
