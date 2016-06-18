/* Neko Engine
 *
 * Windows.cpp
 * Author: Alexandru Naiman
 *
 * OpenGL 4.5 Renderer Implementation - Windows platform support
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

#include "../../GLRenderer.h"
#include <GL/wglext.h>

using namespace std;

PIXELFORMATDESCRIPTOR pfd =
{
	sizeof(PIXELFORMATDESCRIPTOR),
	1,
	PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER,	// Flags
	PFD_TYPE_RGBA,												// The kind of framebuffer. RGBA or palette.
	32,															// Colordepth of the framebuffer.
	0, 0, 0, 0, 0, 0,
	0,
	0,
	0,
	0, 0, 0, 0,
	0,															// Number of bits for the depthbuffer
	0,															// Number of bits for the stencilbuffer
	0,															// Number of Aux buffers in the framebuffer.
	PFD_MAIN_PLANE,
	0,
	0, 0, 0
};

static PFNWGLSWAPINTERVALEXTPROC wglSwapIntervalEXT = nullptr;

bool GLRenderer::Initialize(PlatformWindowType hWnd, unordered_map<string, string> *args, bool debug)
{
	_dc = GetDC(hWnd);
	HGLRC dummy;

	const int pixelFormatAttribs[] =
	{
		WGL_DRAW_TO_WINDOW_ARB, GL_TRUE,
		WGL_SUPPORT_OPENGL_ARB, GL_TRUE,
		WGL_DOUBLE_BUFFER_ARB, GL_TRUE,
		WGL_PIXEL_TYPE_ARB, WGL_TYPE_RGBA_ARB,
		WGL_COLOR_BITS_ARB, 32,
		WGL_DEPTH_BITS_ARB, 0,
		WGL_SAMPLE_BUFFERS_ARB, 0,
		WGL_SAMPLES_ARB, 1,
		0
	};

	int format = ChoosePixelFormat(_dc, &pfd);

	if (!format)
		return false;

	if (!SetPixelFormat(_dc, format, &pfd))
		return false;

	dummy = wglCreateContext(_dc);
	
	if (!dummy)
		return false;

	if (!wglMakeCurrent(_dc, dummy))
	{
		wglDeleteContext(dummy);
		return false;
	}

	PFNWGLCREATECONTEXTATTRIBSARBPROC wglCreateContextAttribsARB = (PFNWGLCREATECONTEXTATTRIBSARBPROC)wglGetProcAddress("wglCreateContextAttribsARB");
	if (!wglCreateContextAttribsARB)
	{
		wglMakeCurrent(NULL, NULL);
		wglDeleteContext(dummy);

		return false;
	}

	wglMakeCurrent(NULL, NULL);
	wglDeleteContext(dummy);

	PFNWGLCHOOSEPIXELFORMATARBPROC wglChoosePixelFormatARB = (PFNWGLCHOOSEPIXELFORMATARBPROC)wglGetProcAddress("wglChoosePixelFormatARB");
	if (wglChoosePixelFormatARB)
	{
		int pixelFormat, numFormats;
		wglChoosePixelFormatARB(_dc, pixelFormatAttribs, NULL, 1, &pixelFormat, (UINT *)&numFormats);

		SetPixelFormat(_dc, pixelFormat, &pfd);
	}

	std::vector<int> attribList;
	attribList.push_back(WGL_CONTEXT_PROFILE_MASK_ARB);
	attribList.push_back(WGL_CONTEXT_CORE_PROFILE_BIT_ARB);
	
	if (debug)
	{
		attribList.push_back(WGL_CONTEXT_FLAGS_ARB);
		attribList.push_back(WGL_CONTEXT_DEBUG_BIT_ARB);
	}

	attribList.push_back(0);

	_ctx = wglCreateContextAttribsARB(_dc, nullptr, attribList.data());

	if (!_ctx)

		return false;

	wglMakeCurrent(_dc, _ctx);

	if (!gladLoadGL())
		return false;

	if (debug)
	{
		PFNGLDEBUGMESSAGECALLBACKPROC _glDebugMessageCallback = (PFNGLDEBUGMESSAGECALLBACKPROC)wglGetProcAddress("glDebugMessageCallback");
		PFNGLDEBUGMESSAGECALLBACKARBPROC _glDebugMessageCallbackARB = (PFNGLDEBUGMESSAGECALLBACKARBPROC)wglGetProcAddress("glDebugMessageCallbackARB");
		PFNGLDEBUGMESSAGECALLBACKAMDPROC _glDebugMessageCallbackAMD = (PFNGLDEBUGMESSAGECALLBACKAMDPROC)wglGetProcAddress("glDebugMessageCallbackAMD");

		if (_glDebugMessageCallback != NULL)
			_glDebugMessageCallback((GLDEBUGPROC)DebugCallback, NULL);
		else if (_glDebugMessageCallbackARB != NULL)
			_glDebugMessageCallbackARB((GLDEBUGPROCARB)DebugCallback, NULL);
		else if (_glDebugMessageCallbackAMD != NULL)
			_glDebugMessageCallbackAMD((GLDEBUGPROCAMD)DebugCallbackAMD, NULL);
		else
			OutputDebugStringA("OpenGL debug output not available.\n");

		if ((_glDebugMessageCallback != NULL) || (_glDebugMessageCallbackARB != NULL))
			glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
	}

	wglSwapIntervalEXT = (PFNWGLSWAPINTERVALEXTPROC)wglGetProcAddress("wglSwapIntervalEXT");

	_CheckExtensions();
	_ParseArguments(args);

	return true;
}

void GLRenderer::SetSwapInterval(int swapInterval)
{
	wglSwapIntervalEXT(swapInterval);
}

void GLRenderer::SwapBuffers()
{
	::SwapBuffers(_dc);
}

void GLRenderer::_DestroyContext()
{
	wglMakeCurrent(NULL, NULL);
	wglDeleteContext(_ctx);
	DeleteDC(_dc);
}	
