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

#include "MGLRenderer.h"
#include "MGLArrayBuffer.h"
#include "MGLBuffer.h"
#include "MGLFramebuffer.h"
#include "MGLShader.h"
#include "MGLTexture.h"

#include <OpenGL/gl3.h>

static GLenum GL_DrawModes[2] =
{
    GL_TRIANGLES,
    GL_TRIANGLE_STRIP
};

static GLenum GL_ElementType[2] =
{
    GL_UNSIGNED_INT,
    GL_UNSIGNED_INT
};

static GLenum GL_PolygonFace[3] =
{
    GL_FRONT,
    GL_BACK,
    GL_FRONT_AND_BACK
};

static GLenum GL_TestFunc[8] =
{
    GL_NEVER,
    GL_LESS,
    GL_LEQUAL,
    GL_GREATER,
    GL_GEQUAL,
    GL_EQUAL,
    GL_NOTEQUAL,
    GL_ALWAYS
};

static GLenum GL_TestOp[8] =
{
    GL_KEEP,
    GL_ZERO,
    GL_REPLACE,
    GL_INCR,
    GL_INCR_WRAP,
    GL_DECR,
    GL_DECR_WRAP,
    GL_INVERT
};

static GLenum GL_BlendFactor[19] =
{
    GL_ZERO, GL_ONE, GL_SRC_COLOR, GL_ONE_MINUS_SRC_COLOR,
    GL_DST_COLOR, GL_ONE_MINUS_DST_COLOR, GL_SRC_ALPHA,
    GL_ONE_MINUS_SRC_ALPHA, GL_DST_ALPHA, GL_ONE_MINUS_DST_ALPHA,
    GL_CONSTANT_COLOR, GL_ONE_MINUS_CONSTANT_COLOR,
    GL_CONSTANT_ALPHA, GL_ONE_MINUS_CONSTANT_ALPHA,
    GL_SRC_ALPHA_SATURATE, GL_SRC1_COLOR,
    GL_ONE_MINUS_SRC1_COLOR, GL_SRC1_ALPHA,
    GL_ONE_MINUS_SRC1_ALPHA
};

static GLenum GL_BlendEquation[3] =
{
    GL_FUNC_ADD,
    GL_FUNC_SUBTRACT,
    GL_FUNC_REVERSE_SUBTRACT
};

static GLenum GL_FrontFace[2] =
{
    GL_CW,
    GL_CCW
};

extern GLenum GL_TexFormat[];
extern GLenum GL_TexType[];

RFramebuffer* MGLRenderer::_boundFramebuffer = nullptr;
std::vector<ShaderDefine> MGLRenderer::_shaderDefines;
MGLShader* MGLRenderer::_activeShader;

static NSOpenGLContext *_ctx;

MGLRenderer::MGLRenderer() { }

void MGLRenderer::SetDebugLogFunction(RendererDebugLogProc debugLog)
{
    //_debugLogFunc = debugLog;
}

const char* MGLRenderer::GetName()
{
    return "OpenGL";
}

int MGLRenderer::GetMajorVersion()
{
    GLint ver;
    GL_CHECK(glGetIntegerv(GL_MAJOR_VERSION, &ver));
    return ver;
}

int MGLRenderer::GetMinorVersion()
{
    GLint ver;
    GL_CHECK(glGetIntegerv(GL_MINOR_VERSION, &ver));
    return ver;
}

void MGLRenderer::SetClearColor(float r, float g, float b, float a)
{
	[_ctx makeCurrentContext];
    GL_CHECK(glClearColor(r, g, b, a));
}

void MGLRenderer::SetViewport(int x, int y, int width, int height)
{
    GL_CHECK(glViewport(x, y, width, height));
}

void MGLRenderer::EnableDepthTest(bool enable)
{
    if (enable)
    { GL_CHECK(glEnable(GL_DEPTH_TEST)); }
    else
    { GL_CHECK(glDisable(GL_DEPTH_TEST)); }
}

void MGLRenderer::SetDepthFunc(TestFunc func)
{
    GL_CHECK(glDepthFunc(GL_TestFunc[(int)func]));
}

void MGLRenderer::SetDepthRange(double n, double f)
{
    GL_CHECK(glDepthRange(n, f));
}

void MGLRenderer::SetDepthRangef(float n, float f)
{
    GL_CHECK(glDepthRangef(n, f));
}

void MGLRenderer::SetDepthMask(bool mask)
{
    GL_CHECK(glDepthMask(mask ? GL_TRUE : GL_FALSE));
}

void MGLRenderer::EnableStencilTest(bool enable)
{
    if (enable)
    { GL_CHECK(glEnable(GL_STENCIL_TEST)); }
    else
    { GL_CHECK(glDisable(GL_STENCIL_TEST)); }
}

void MGLRenderer::SetStencilFunc(TestFunc func, int ref, unsigned int mask)
{
    GL_CHECK(glStencilFunc(GL_TestFunc[(int)func], ref, mask));
}

void MGLRenderer::SetStencilFuncSeparate(PolygonFace face, TestFunc func, int ref, unsigned int mask)
{
    GL_CHECK(glStencilFuncSeparate(GL_PolygonFace[(int)face], GL_TestFunc[(int)func], ref, mask));
}

void MGLRenderer::SetStencilOp(TestOp sfail, TestOp dpfail, TestOp dppass)
{
    GL_CHECK(glStencilOp(GL_TestOp[(int)sfail], GL_TestOp[(int)dpfail], GL_TestOp[(int)dppass]));
}

void MGLRenderer::SetStencilOpSeparate(PolygonFace face, TestOp sfail, TestOp dpfail, TestOp dppass)
{
    GL_CHECK(glStencilOpSeparate(GL_PolygonFace[(int)face], GL_TestOp[(int)sfail], GL_TestOp[(int)dpfail], GL_TestOp[(int)dppass]));
}

void MGLRenderer::EnableBlend(bool enable)
{
    if (enable)
    { GL_CHECK(glEnable(GL_BLEND)); }
    else
    { GL_CHECK(glDisable(GL_BLEND)); }
}

void MGLRenderer::SetBlendFunc(BlendFactor src, BlendFactor dst)
{
    GL_CHECK(glBlendFunc(GL_BlendFactor[(int)src], GL_BlendFactor[(int)dst]));
}

void MGLRenderer::SetBlendFuncSeparate(BlendFactor srcColor, BlendFactor dstColor, BlendFactor srcAlpha, BlendFactor dstAlpha)
{
    GL_CHECK(glBlendFuncSeparate(GL_BlendFactor[(int)srcColor], GL_BlendFactor[(int)dstColor], GL_BlendFactor[(int)srcAlpha], GL_BlendFactor[(int)dstAlpha]));
}

void MGLRenderer::SetBlendColor(float r, float g, float b, float a)
{
    GL_CHECK(glBlendColor(r, g, b, a));
}

void MGLRenderer::SetBlendEquation(BlendEquation eq)
{
    GL_CHECK(glBlendEquation(GL_BlendEquation[(int)eq]));
}

void MGLRenderer::SetBlendEquationSeparate(BlendEquation color, BlendEquation alpha)
{
    GL_CHECK(glBlendEquationSeparate(GL_BlendEquation[(int)color], GL_BlendEquation[(int)alpha]));
}

void MGLRenderer::SetStencilMask(unsigned int mask)
{
    GL_CHECK(glStencilMask(mask));
}

void MGLRenderer::SetStencilMaskSeparate(PolygonFace face, unsigned int mask)
{
    GL_CHECK(glStencilMaskSeparate(GL_PolygonFace[(int)face], mask));
}

void MGLRenderer::EnableFaceCulling(bool enable)
{
    if (enable)
    { GL_CHECK(glEnable(GL_CULL_FACE)); }
    else
    { GL_CHECK(glDisable(GL_CULL_FACE)); }
}

void MGLRenderer::SetFaceCulling(PolygonFace face)
{
    GL_CHECK(glCullFace(GL_PolygonFace[(int)face]));
}

void MGLRenderer::SetFrontFace(FrontFace face)
{
    GL_CHECK(glFrontFace(GL_FrontFace[(int)face]));
}

void MGLRenderer::SetColorMask(bool r, bool g, bool b, bool a)
{
    GL_CHECK(glColorMask(r ? GL_TRUE : GL_FALSE,
                         g ? GL_TRUE : GL_FALSE,
                         b ? GL_TRUE : GL_FALSE,
                         a ? GL_TRUE : GL_FALSE));
}

void MGLRenderer::DrawArrays(PolygonMode mode, int32_t first, int32_t count)
{
	_activeShader->EnableTextures();
	
	if(_boundFramebuffer)
		_boundFramebuffer->Bind(FB_DRAW);
	else
	{ GL_CHECK(glBindFramebuffer(GL_FRAMEBUFFER, 0)); }
	
    GL_CHECK(glDrawArrays(GL_DrawModes[(int)mode], first, (GLsizei)count));
}

void MGLRenderer::DrawElements(PolygonMode mode, int32_t count, ElementType type, const void *indices)
{
	_activeShader->EnableTextures();
	
	if(_boundFramebuffer)
		_boundFramebuffer->Bind(FB_DRAW);
	else
	{ GL_CHECK(glBindFramebuffer(GL_FRAMEBUFFER, 0)); }

    GL_CHECK(glDrawElements(GL_DrawModes[(int)mode], (GLsizei)count, GL_ElementType[(int)type], indices));
}

void MGLRenderer::DrawElementsBaseVertex(PolygonMode mode, int32_t count, ElementType type, const void *indices, int32_t baseVertex)
{
	_activeShader->EnableTextures();
	
	if(_boundFramebuffer)
		_boundFramebuffer->Bind(FB_DRAW);
	else
	{ GL_CHECK(glBindFramebuffer(GL_FRAMEBUFFER, 0)); }
	
    GL_CHECK(glDrawElementsBaseVertex(GL_DrawModes[(int)mode], (GLsizei)count, GL_ElementType[(int)type], indices, baseVertex));
}

void MGLRenderer::Clear(uint32_t mask)
{
    GLbitfield glMask = 0;
    
    if (mask & R_CLEAR_COLOR)
        glMask |= GL_COLOR_BUFFER_BIT;
    
    if (mask & R_CLEAR_DEPTH)
        glMask |= GL_DEPTH_BUFFER_BIT;
    
    if (mask & R_CLEAR_STENCIL)
        glMask |= GL_STENCIL_BUFFER_BIT;
    
    GL_CHECK(glClear(glMask));
}

void MGLRenderer::BindDefaultFramebuffer()
{
    GL_CHECK(glBindFramebuffer(GL_FRAMEBUFFER, 0));
	_boundFramebuffer = nullptr;
}

RFramebuffer* MGLRenderer::GetBoundFramebuffer()
{
    return _boundFramebuffer;
}

void MGLRenderer::SetMinSampleShading(int32_t samples)
{
    GL_CHECK(glMinSampleShading((GLfloat)samples));
}

void MGLRenderer::ReadPixels(int x, int y, int width, int height, TextureFormat format, TextureInternalType type, void* data)
{
    GL_CHECK(glReadPixels(x, y, width, height, GL_TexFormat[(int)format], GL_TexType[(int)type], data));
}

bool MGLRenderer::HasCapability(RendererCapability cap)
{
    const char *ext = nullptr;
    
    switch (cap)
    {
        case RendererCapability::MemoryInformation:
            return false;
        case RendererCapability::AnisotropicFiltering:
            return true;
        case RendererCapability::MultisampledFramebuffer:
            return true;
        case RendererCapability::PerSampleShading:
			return true;
        default:
            return false;
    }
    
    return _HasExtension(ext);
}

RBuffer* MGLRenderer::CreateBuffer(BufferType type, bool dynamic, bool persistent)
{
    return (RBuffer *)new MGLBuffer(type, dynamic, persistent);
}

RShader* MGLRenderer::CreateShader()
{
    return (RShader *)new MGLShader();
}

RTexture* MGLRenderer::CreateTexture(TextureType type)
{
    return (RTexture *)new MGLTexture(type);
}

RFramebuffer* MGLRenderer::CreateFramebuffer(int width, int height)
{
    return (RFramebuffer *)new MGLFramebuffer(width, height);
}

RArrayBuffer* MGLRenderer::CreateArrayBuffer()
{
    return (RArrayBuffer *)new MGLArrayBuffer();
}

void MGLRenderer::AddShaderDefine(std::string name, std::string value)
{
    ShaderDefine d{name, value};
    _shaderDefines.push_back(d);
}

bool MGLRenderer::IsTextureFormatSupported(TextureFileFormat format)
{
    switch (format)
    {
        case TextureFileFormat::DDS:
        case TextureFileFormat::TGA:
            return true;
        default:
            return false;
    }
}

uint64_t MGLRenderer::GetVideoMemorySize()
{
    return 0;
}

uint64_t MGLRenderer::GetUsedVideoMemorySize()
{
    return 0;
}

bool MGLRenderer::_HasExtension(const char* extension)
{
    int numExtensions;
    
    GL_CHECK(glGetIntegerv(GL_NUM_EXTENSIONS, &numExtensions));
    size_t len = strlen(extension);
    
    for (int i = 0; i < numExtensions; i++)
        if (!strncmp((char *)glGetStringi(GL_EXTENSIONS, i), extension, len))
            return true;
    
    return false;
}

MGLRenderer::~MGLRenderer()
{
    _DestroyContext();
}

// Platform

bool MGLRenderer::Initialize(PlatformWindowType hWnd, bool debug)
{
	_window = hWnd;
	
	NSOpenGLPixelFormatAttribute attribs[] =
	{
		NSOpenGLPFAOpenGLProfile,
		NSOpenGLProfileVersion4_1Core,
		NSOpenGLPFADoubleBuffer,
		NSOpenGLPFAAccelerated,
		0
	};
	
	NSOpenGLPixelFormat *pf = [[NSOpenGLPixelFormat alloc] initWithAttributes:attribs];
	
	_ctx = [[NSOpenGLContext alloc] initWithFormat:pf shareContext:nil];
	[_ctx setView:_window.contentView];
	[_ctx makeCurrentContext];
	
	[_ctx flushBuffer];
	
	return true;
}

void MGLRenderer::SetSwapInterval(int swapInterval)
{
	//
}

void MGLRenderer::SwapBuffers()
{
	[_ctx makeCurrentContext];
	[_ctx flushBuffer];
}

void MGLRenderer::_DestroyContext()
{
	//
}

void MGLRenderer::MakeCurrent()
{
	[_ctx makeCurrentContext];
}
