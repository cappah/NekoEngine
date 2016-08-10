/* NekoEngine
 *
 * EGL.cpp
 * Author: Alexandru Naiman
 *
 * OpenGL|ES 3 Renderer Implementation - Platform Support for EGL-compatible
 * Systems
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

#include <EGL/egl.h>
#include <EGL/eglext.h>
#include <Platform/Platform.h>

#include "GLESRenderer.h"
#include "glad.h"

static EGLDisplay _display;
static EGLContext _context;
static EGLSurface _surface;

using namespace std;

bool GLESRenderer::Initialize(PlatformWindowType hWnd, unordered_map<string, string> *args, bool debug)
{
    EGLint numConfigs = 0;
    EGLConfig config = nullptr;

    _window = hWnd;

    EGLint configAttribs[] =
    {
            EGL_RED_SIZE,           8,
            EGL_GREEN_SIZE,         8,
            EGL_BLUE_SIZE,          8,
            EGL_ALPHA_SIZE,         0,
            EGL_DEPTH_SIZE,         EGL_DONT_CARE,
            EGL_STENCIL_SIZE,       0,
            EGL_SAMPLE_BUFFERS,     0,
            EGL_SAMPLES,            0,
            EGL_SURFACE_TYPE,       EGL_WINDOW_BIT,
            EGL_RENDERABLE_TYPE,    EGL_OPENGL_ES2_BIT,
            EGL_NONE
    };

    eglGetError();

    _display = eglGetDisplay(EGL_DEFAULT_DISPLAY);
    if(_display == EGL_NO_DISPLAY)
    {
        fprintf(stderr, "eglGetDisplay failed with 0x%x\n", eglGetError());
        return false;
    }

    if(!eglInitialize(_display, nullptr, nullptr))
    {
        fprintf(stderr, "eglInitialize failed with 0x%x\n", eglGetError());
        return false;
    }

    if(!eglBindAPI(EGL_OPENGL_ES_API))
    {
        fprintf(stderr, "eglBindAPI failed with 0x%s\n", eglGetError());
        return false;
    }

    if(!eglChooseConfig(_display, configAttribs, &config, 1, &numConfigs))
    {
        fprintf(stderr, "eglChooseConfig failed with 0x%x\n", eglGetError());
        return false;
    }

    if(!numConfigs || config == nullptr)
    {
        fprintf(stderr, "no suitable display configuration found\n");
        return false;
    }

    EGLint attributes[] = { EGL_CONTEXT_CLIENT_VERSION, 3, EGL_NONE };
    if((_context = eglCreateContext(_display, config, EGL_NO_CONTEXT, attributes)) == EGL_NO_CONTEXT)
    {
        fprintf(stderr, "eglCreateContext failed with 0x%x\n", eglGetError());
        return false;
    }

    if((_surface = eglCreateWindowSurface(_display, config, (EGLNativeWindowType)_window, NULL)) == EGL_NO_SURFACE)
    {
        fprintf(stderr, "eglCreateWindowSurface failed with 0x%x\n", eglGetError());
        return false;
    }

    eglMakeCurrent(_display, _surface, _surface, _context);

    if (!gladLoadGLES2Loader((GLADloadproc)eglGetProcAddress))
        return false;

    return true;
}

void GLESRenderer::SetSwapInterval(int swapInterval)
{
    eglSwapInterval(_display, swapInterval);
}

void GLESRenderer::ScreenResized()
{
    //
}

void GLESRenderer::SwapBuffers()
{
    eglSwapBuffers(_display, _surface);
}

void GLESRenderer::_DestroyContext()
{
    eglMakeCurrent(_display, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT);
    eglDestroySurface(_display, _surface);
    eglDestroyContext(_display, _surface);
    eglTerminate(_display);
}

void GLESRenderer::MakeCurrent()
{
    eglMakeCurrent(_display, _surface, _surface, _context);
}
