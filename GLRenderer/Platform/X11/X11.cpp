/* Neko Engine
 *
 * X11.cpp
 * Author: Alexandru Naiman
 *
 * OpenGL 4.5 Renderer Implementation - UNIX / X11 platform support
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

#include "../../GLRenderer.h"
#include <X11/Xlib.h>
#include <GL/glx.h>

#include <stdio.h>

using namespace std;

#ifndef GLX_CONTEXT_PROFILE_MASK_ARB
#define GLX_CONTEXT_PROFILE_MASK_ARB		0x9126
#endif

#ifndef GLX_CONTEXT_CORE_PROFILE_BIT_ARB
#define GLX_CONTEXT_CORE_PROFILE_BIT_ARB	0x1
#endif

static PFNGLXSWAPINTERVALEXTPROC glXSwapIntervalEXT = 0;

bool GLRenderer::Initialize(PlatformWindowType hWnd, unordered_map<string, string> *args, bool debug)
{
	_dc = XOpenDisplay(NULL);

	static int visual_attribs[] =
	{
		GLX_X_RENDERABLE    , True,
		GLX_DRAWABLE_TYPE   , GLX_WINDOW_BIT,
		GLX_RENDER_TYPE     , GLX_RGBA_BIT,
		GLX_X_VISUAL_TYPE   , GLX_TRUE_COLOR,
		GLX_RED_SIZE        , 8,
		GLX_GREEN_SIZE      , 8,
		GLX_BLUE_SIZE       , 8,
		GLX_ALPHA_SIZE      , 0,
		GLX_DEPTH_SIZE      , 0,
		GLX_STENCIL_SIZE    , 0,
		GLX_SAMPLE_BUFFERS  , 0,
		GLX_SAMPLES         , 0,
		0
	};

	int glx_major, glx_minor;

	if (!glXQueryVersion(_dc, &glx_major, &glx_minor) || ((glx_major == 1) && (glx_minor < 3)) || (glx_major < 1))
	{
		Platform::MessageBox("Fatal Error", "This program requires a newer version of GLX", MessageBoxButtons::OK, MessageBoxIcon::Error);
		return false;
	}

	PFNGLXCHOOSEFBCONFIGPROC glXChooseFBConfig = 0;
	PFNGLXGETVISUALFROMFBCONFIGPROC glXGetVisualFromFBConfig = 0;
	PFNGLXGETFBCONFIGATTRIBPROC glXGetFBConfigAttrib = 0;

	glXChooseFBConfig = (PFNGLXCHOOSEFBCONFIGPROC)glXGetProcAddress((const GLubyte*)"glXChooseFBConfig");
	glXGetVisualFromFBConfig = (PFNGLXGETVISUALFROMFBCONFIGPROC)glXGetProcAddress((const GLubyte*)"glXGetVisualFromFBConfig");
	glXGetFBConfigAttrib = (PFNGLXGETFBCONFIGATTRIBPROC)glXGetProcAddress((const GLubyte*)"glXGetFBConfigAttrib");

	if (glXChooseFBConfig == nullptr ||
		glXGetVisualFromFBConfig == nullptr ||
		glXGetFBConfigAttrib == nullptr)
	{
		//Platform::MessageBox("Fatal Error", "Unable to load required GLX functions", MessageBoxButtons::OK, MessageBoxIcon::Error);
		return false;
	}

	int fbcount;
	GLXFBConfig* fbc = glXChooseFBConfig(_dc, DefaultScreen(_dc), visual_attribs, &fbcount);

	if (!fbc)
		return false;

	int best_fbc = -1, worst_fbc = -1, best_num_samp = -1, worst_num_samp = 999;

	int i;
	for (i = 0; i<fbcount; ++i)
	{
		XVisualInfo *vi = glXGetVisualFromFBConfig(_dc, fbc[i]);
		if (vi)
		{
			int samp_buf, samples;
			glXGetFBConfigAttrib(_dc, fbc[i], GLX_SAMPLE_BUFFERS, &samp_buf);
			glXGetFBConfigAttrib(_dc, fbc[i], GLX_SAMPLES, &samples);

			if (best_fbc < 0 || (samp_buf && (samples > best_num_samp)))
				best_fbc = i, best_num_samp = samples;
			if (worst_fbc < 0 || !samp_buf || (samples < worst_num_samp))
				worst_fbc = i, worst_num_samp = samples;
		}
		XFree(vi);
	}

	GLXFBConfig bestFbc = fbc[best_fbc];

	XFree(fbc);

	PFNGLXCREATECONTEXTATTRIBSARBPROC glXCreateContextAttribsARB = 0;
	glXCreateContextAttribsARB = (PFNGLXCREATECONTEXTATTRIBSARBPROC)glXGetProcAddressARB((const GLubyte*)"glXCreateContextAttribsARB");

	if (!glXCreateContextAttribsARB)
	{
		//Platform::MessageBox("Fatal Error", "Required extension GLX_ARB_create_context not supported.", MessageBoxButtons::OK, MessageBoxIcon::Error);
		return false;
	}

	std::vector<int> context_attribs;
	context_attribs.push_back(GLX_CONTEXT_PROFILE_MASK_ARB);
	context_attribs.push_back(GLX_CONTEXT_CORE_PROFILE_BIT_ARB);
	
	if (debug)
	{
		context_attribs.push_back(GLX_CONTEXT_FLAGS_ARB);
		context_attribs.push_back(GLX_CONTEXT_DEBUG_BIT_ARB);
	}

	context_attribs.push_back(0);

	_ctx = glXCreateContextAttribsARB(_dc, bestFbc, 0, True, context_attribs.data());

	XSync(_dc, False);

	glXMakeCurrent(_dc, hWnd, _ctx);

	if (!gladLoadGL())
		return false;
	
	_hWnd = hWnd;

	if (debug)
	{
		PFNGLDEBUGMESSAGECALLBACKPROC _glDebugMessageCallback = (PFNGLDEBUGMESSAGECALLBACKPROC)glXGetProcAddress((const GLubyte*)"glDebugMessageCallback");
		PFNGLDEBUGMESSAGECALLBACKARBPROC _glDebugMessageCallbackARB = (PFNGLDEBUGMESSAGECALLBACKARBPROC)glXGetProcAddress((const GLubyte*)"glDebugMessageCallbackARB");
		PFNGLDEBUGMESSAGECALLBACKAMDPROC _glDebugMessageCallbackAMD = (PFNGLDEBUGMESSAGECALLBACKAMDPROC)glXGetProcAddress((const GLubyte*)"glDebugMessageCallbackAMD");

		if (_glDebugMessageCallback != NULL)
			_glDebugMessageCallback((GLDEBUGPROC)DebugCallback, NULL);
		else if (_glDebugMessageCallbackARB != NULL)
			_glDebugMessageCallbackARB((GLDEBUGPROCARB)DebugCallback, NULL);
		else if (_glDebugMessageCallbackAMD != NULL)
			_glDebugMessageCallbackAMD((GLDEBUGPROCAMD)DebugCallbackAMD, NULL);
		else
			printf("OpenGL debug output not available.\n");

		if ((_glDebugMessageCallback != NULL) || (_glDebugMessageCallbackARB != NULL))
			glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
	}

	glXSwapIntervalEXT = (PFNGLXSWAPINTERVALEXTPROC)glXGetProcAddressARB((const GLubyte*)"glXSwapIntervalEXT");

	_CheckExtensions();
	_ParseArguments(args);

	return true;
}

void GLRenderer::SetSwapInterval(int swapInterval)
{
	glXSwapIntervalEXT(_dc, _hWnd, swapInterval);
}

void GLRenderer::SwapBuffers()
{
	glXSwapBuffers(_dc, _hWnd);
}

void GLRenderer::_DestroyContext()
{
	glXMakeCurrent(NULL, 0, 0);
	glXDestroyContext(_dc, _ctx);
}
