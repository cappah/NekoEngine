/* Neko Engine
 *
 * MGLRenderer.mm
 * Author: Alexandru Naiman
 *
 * MacOS X OpenGL Renderer Implementation
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

#include "GLRenderer.h"
#include "GLArrayBuffer.h"
#include "GLBuffer.h"
#include "GLFramebuffer.h"
#include "GLShader.h"
#include "GLTexture.h"

#include <OpenGL/gl3.h>

bool GLRenderer::Initialize(PlatformWindowType hWnd, bool debug)
{
	_hWnd = hWnd;
	
	NSOpenGLPixelFormatAttribute attribs[] =
	{
		NSOpenGLPFAOpenGLProfile,
		NSOpenGLProfileVersion4_1Core,
		NSOpenGLPFADoubleBuffer,
		NSOpenGLPFAAccelerated,
        NSOpenGLPFANoRecovery,
        NSOpenGLPFAAllowOfflineRenderers,
		0
	};
	
	NSOpenGLPixelFormat *pf = [[NSOpenGLPixelFormat alloc] initWithAttributes:attribs];
	
	_ctx = [[NSOpenGLContext alloc] initWithFormat:pf shareContext:nil];
	[_ctx setView:_hWnd.contentView];
	[_ctx makeCurrentContext];
	
	[_ctx flushBuffer];
	
	return true;
}

void GLRenderer::SetSwapInterval(int swapInterval)
{
	//
}

void GLRenderer::SwapBuffers()
{
	[_ctx makeCurrentContext];
	[_ctx flushBuffer];
}

void GLRenderer::_DestroyContext()
{
	//
}
