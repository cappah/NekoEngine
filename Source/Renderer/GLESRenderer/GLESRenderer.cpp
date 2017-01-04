/* NekoEngine
 *
 * GLESRenderer.cpp
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

#include "GLESRenderer.h"
#include "GLESArrayBuffer.h"
#include "GLESFence.h"
#include "GLESBuffer.h"
#include "GLESFramebuffer.h"
#include "GLESShader.h"
#include "GLESTexture.h"

#include <string.h>

#ifdef __APPLE__
    #include <OpenGLES/ES3/glext.h>
#else
    #include <GLES3/gl3ext.h>
#endif

using namespace std;

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
    GL_SRC_ALPHA_SATURATE, 0,
    0, 0,
    0
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

static GLenum GL_PixelStoreParameter[16] =
{
	0,
	0,
	GL_PACK_ROW_LENGTH,
	0,
	GL_PACK_SKIP_PIXELS,
	GL_PACK_SKIP_ROWS,
	0,
	GL_PACK_ALIGNMENT,
	0,
	0,
	GL_UNPACK_ROW_LENGTH,
	GL_UNPACK_IMAGE_HEIGHT,
	GL_UNPACK_SKIP_PIXELS,
	GL_UNPACK_SKIP_ROWS,
	GL_UNPACK_SKIP_IMAGES,
	GL_UNPACK_ALIGNMENT
};

extern GLenum GL_TexFormat[];
extern GLenum GL_TexType[];
static int _verMajor = -1, _verMinor = -1;

RFramebuffer* GLESRenderer::_boundFramebuffer = nullptr;
std::vector<ShaderDefine> GLESRenderer::_shaderDefines;
GLESShader* GLESRenderer::_activeShader;

#ifndef NE_PLATFORM_IOS
	GLFuncs GLESRenderer::Funcs;
#endif

GLESRenderer::GLESRenderer()
{
    _window = (PlatformWindowType)0;
	_drawCalls = 0;
	memset(&_state, 0x0, sizeof(RendererState));
}

void GLESRenderer::SetDebugLogFunction(RendererDebugLogProc debugLog)
{
    //_debugLogFunc = debugLog;
}

const char* GLESRenderer::GetName()
{
    return "OpenGL|ES";
}

int GLESRenderer::GetMajorVersion()
{
    if(_verMajor < 0)
	{ GL_CHECK(glGetIntegerv(GL_MAJOR_VERSION, &_verMajor)); }
    return _verMajor;
}

int GLESRenderer::GetMinorVersion()
{
    if(_verMinor < 0)
	{ GL_CHECK(glGetIntegerv(GL_MINOR_VERSION, &_verMinor)); }
    return _verMinor;
}

void GLESRenderer::SetClearColor(float r, float g, float b, float a)
{
	if((_state.ClearColor.r == r) &&
	   (_state.ClearColor.g == g) &&
	   (_state.ClearColor.b == b) &&
	   (_state.ClearColor.a == a))
		return;

	_state.ClearColor.r = r;
	_state.ClearColor.g = g;
	_state.ClearColor.b = b;
	_state.ClearColor.a = a;

    GL_CHECK(glClearColor(r, g, b, a));
}

void GLESRenderer::SetViewport(int x, int y, int width, int height)
{
	if((_state.Viewport.x == x) &&
	   (_state.Viewport.y == y) &&
	   (_state.Viewport.width == width) &&
	   (_state.Viewport.height == height))
		return;

	_state.Viewport.x = x;
	_state.Viewport.y = y;
	_state.Viewport.width = width;
	_state.Viewport.height = height;

    GL_CHECK(glViewport(x, y, width, height));
}

void GLESRenderer::EnableDepthTest(bool enable)
{
	if (enable == _state.DepthTest)
		return;

	_state.DepthTest = enable;

    if (enable)
    { GL_CHECK(glEnable(GL_DEPTH_TEST)); }
    else
    { GL_CHECK(glDisable(GL_DEPTH_TEST)); }
}

void GLESRenderer::SetDepthFunc(TestFunc func)
{
	if(_state.DepthFunc == func)
		return;

	_state.DepthFunc = func;

    GL_CHECK(glDepthFunc(GL_TestFunc[(int)func]));
}

void GLESRenderer::SetDepthRange(double n, double f)
{
    GL_CHECK(glDepthRangef(n, f));
}

void GLESRenderer::SetDepthRangef(float n, float f)
{
    GL_CHECK(glDepthRangef(n, f));
}

void GLESRenderer::SetDepthMask(bool mask)
{
	if(_state.DepthMask == mask)
		return;

	_state.DepthMask = mask;

    GL_CHECK(glDepthMask(mask ? GL_TRUE : GL_FALSE));
}

void GLESRenderer::EnableStencilTest(bool enable)
{
	if (enable == _state.StencilTest)
		return;

	_state.StencilTest = enable;

    if (enable)
    { GL_CHECK(glEnable(GL_STENCIL_TEST)); }
    else
    { GL_CHECK(glDisable(GL_STENCIL_TEST)); }
}

void GLESRenderer::SetStencilFunc(TestFunc func, int ref, unsigned int mask)
{
	if((_state.StencilFunc.F == func) &&
	   (_state.StencilFunc.Ref == ref) &&
	   (_state.StencilFunc.Mask == mask))
		return;

	_state.StencilFunc.F = func;
	_state.StencilFunc.Ref = ref;
	_state.StencilFunc.Mask = mask;

    GL_CHECK(glStencilFunc(GL_TestFunc[(int)func], ref, mask));
}

void GLESRenderer::SetStencilFuncSeparate(PolygonFace face, TestFunc func, int ref, unsigned int mask)
{
    GL_CHECK(glStencilFuncSeparate(GL_PolygonFace[(int)face], GL_TestFunc[(int)func], ref, mask));
}

void GLESRenderer::SetStencilOp(TestOp sfail, TestOp dpfail, TestOp dppass)
{
    GL_CHECK(glStencilOp(GL_TestOp[(int)sfail], GL_TestOp[(int)dpfail], GL_TestOp[(int)dppass]));
}

void GLESRenderer::SetStencilOpSeparate(PolygonFace face, TestOp sfail, TestOp dpfail, TestOp dppass)
{
    GL_CHECK(glStencilOpSeparate(GL_PolygonFace[(int)face], GL_TestOp[(int)sfail], GL_TestOp[(int)dpfail], GL_TestOp[(int)dppass]));
}

void GLESRenderer::EnableBlend(bool enable)
{
	if (enable == _state.Blend)
		return;

	_state.Blend = enable;

    if (enable)
    { GL_CHECK(glEnable(GL_BLEND)); }
    else
    { GL_CHECK(glDisable(GL_BLEND)); }
}

void GLESRenderer::SetBlendFunc(BlendFactor src, BlendFactor dst)
{
	if((_state.BlendFunction.src == src) &&
	   (_state.BlendFunction.dst) == dst)
		return;

	_state.BlendFunction.src = src;
	_state.BlendFunction.dst = dst;

    GL_CHECK(glBlendFunc(GL_BlendFactor[(int)src], GL_BlendFactor[(int)dst]));
}

void GLESRenderer::SetBlendFuncSeparate(BlendFactor srcColor, BlendFactor dstColor, BlendFactor srcAlpha, BlendFactor dstAlpha)
{
    GL_CHECK(glBlendFuncSeparate(GL_BlendFactor[(int)srcColor], GL_BlendFactor[(int)dstColor], GL_BlendFactor[(int)srcAlpha], GL_BlendFactor[(int)dstAlpha]));
}

void GLESRenderer::SetBlendColor(float r, float g, float b, float a)
{
    GL_CHECK(glBlendColor(r, g, b, a));
}

void GLESRenderer::SetBlendEquation(BlendEquation eq)
{
    GL_CHECK(glBlendEquation(GL_BlendEquation[(int)eq]));
}

void GLESRenderer::SetBlendEquationSeparate(BlendEquation color, BlendEquation alpha)
{
    GL_CHECK(glBlendEquationSeparate(GL_BlendEquation[(int)color], GL_BlendEquation[(int)alpha]));
}

void GLESRenderer::SetStencilMask(unsigned int mask)
{
	if(_state.StencilMask == mask)
		return;

	_state.StencilMask = mask;

    GL_CHECK(glStencilMask(mask));
}

void GLESRenderer::SetStencilMaskSeparate(PolygonFace face, unsigned int mask)
{
    GL_CHECK(glStencilMaskSeparate(GL_PolygonFace[(int)face], mask));
}

void GLESRenderer::EnableFaceCulling(bool enable)
{
    if (enable)
    { GL_CHECK(glEnable(GL_CULL_FACE)); }
    else
    { GL_CHECK(glDisable(GL_CULL_FACE)); }
}

void GLESRenderer::SetFaceCulling(PolygonFace face)
{
    GL_CHECK(glCullFace(GL_PolygonFace[(int)face]));
}

void GLESRenderer::SetFrontFace(FrontFace face)
{
    GL_CHECK(glFrontFace(GL_FrontFace[(int)face]));
}

void GLESRenderer::SetColorMask(bool r, bool g, bool b, bool a)
{
    GL_CHECK(glColorMask(r ? GL_TRUE : GL_FALSE,
                         g ? GL_TRUE : GL_FALSE,
                         b ? GL_TRUE : GL_FALSE,
                         a ? GL_TRUE : GL_FALSE));
}

void GLESRenderer::DrawArrays(PolygonMode mode, int32_t first, int32_t count)
{
#ifdef _DEBUG
	if(!_activeShader->Validate())
	{ DIE("Shader program validation failed"); }
#endif

	_activeShader->EnableTextures();

	if(_boundFramebuffer)
		_boundFramebuffer->Bind(FB_DRAW);
	else
		BindDefaultFramebuffer();

    GL_CHECK(glDrawArrays(GL_DrawModes[(int)mode], first, (GLsizei)count));
}

void GLESRenderer::DrawElements(PolygonMode mode, int32_t count, ElementType type, const void *indices)
{
#ifdef _DEBUG
	if(!_activeShader->Validate())
	{ DIE("Shader program validation failed"); }
#endif

	_activeShader->EnableTextures();

	if(_boundFramebuffer)
		_boundFramebuffer->Bind(FB_DRAW);
	else
		BindDefaultFramebuffer();

    GL_CHECK(glDrawElements(GL_DrawModes[(int)mode], (GLsizei)count, GL_ElementType[(int)type], indices));
}

void GLESRenderer::DrawElementsBaseVertex(PolygonMode mode, int32_t count, ElementType type, const void *indices, int32_t baseVertex)
{
#ifndef NE_PLATFORM_IOS
#ifdef _DEBUG
	if(!_activeShader->Validate())
	{ DIE("Shader program validation failed"); }
#endif

	_activeShader->EnableTextures();

	if(_boundFramebuffer)
		_boundFramebuffer->Bind(FB_DRAW);
	else
		BindDefaultFramebuffer();

    GL_CHECK(glDrawElementsBaseVertex(GL_DrawModes[(int)mode], (GLsizei)count, GL_ElementType[(int)type], indices, baseVertex));
#endif
}

void GLESRenderer::Clear(uint32_t mask)
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

RFramebuffer* GLESRenderer::GetBoundFramebuffer()
{
    return _boundFramebuffer;
}

void GLESRenderer::SetMinSampleShading(int32_t samples)
{
    //GL_CHECK(glMinSampleShading((GLfloat)samples));
}

void GLESRenderer::ReadPixels(int x, int y, int width, int height, TextureFormat format, TextureInternalType type, void* data)
{
    GL_CHECK(glReadPixels(x, y, width, height, GL_TexFormat[(int)format], GL_TexType[(int)type], data));
}

void GLESRenderer::SetPixelStore(PixelStoreParameter param, int value)
{
	GL_CHECK(glPixelStorei(GL_PixelStoreParameter[(int)param], value));
}

bool GLESRenderer::HasCapability(RendererCapability cap)
{
    switch (cap)
    {
        case RendererCapability::AnisotropicFiltering:
			return true;
        case RendererCapability::MultisampledFramebuffer:
			return false;
        case RendererCapability::PerSampleShading:
			return true;
		case RendererCapability::DrawBaseVertex:
#ifndef NE_PLATFORM_IOS
			return Funcs.DrawElementsBaseVertex != nullptr;
#else
			return false;
#endif
        case RendererCapability::MemoryInformation:
            return false;
        default:
            return false;
    }
}

RBuffer *GLESRenderer::CreateBuffer(BufferType type, bool dynamic, bool persistent)
{
    return (RBuffer *)new GLESBuffer(type, dynamic, persistent);
}

RShader *GLESRenderer::CreateShader()
{
    return (RShader *)new GLESShader();
}

RTexture *GLESRenderer::CreateTexture(TextureType type)
{
    return (RTexture *)new GLESTexture(type);
}

RFramebuffer *GLESRenderer::CreateFramebuffer(int width, int height)
{
    return (RFramebuffer *)new GLESFramebuffer(width, height);
}

RArrayBuffer *GLESRenderer::CreateArrayBuffer()
{
    return (RArrayBuffer *)new GLESArrayBuffer();
}

RFence *GLESRenderer::CreateFence()
{
	return (RFence *)new GLESFence();
}

void GLESRenderer::AddShaderDefine(std::string name, std::string value)
{
    ShaderDefine d{name, value};
    _shaderDefines.push_back(d);
}

bool GLESRenderer::IsTextureFormatSupported(TextureFileFormat format)
{
    switch (format)
    {
        case TextureFileFormat::TGA:
            return true;
        default:
            return false;
    }
}

int GLESRenderer::GetMaxSamples()
{
	GLint maxSamples;
	GL_CHECK(glGetIntegerv(GL_MAX_SAMPLES, &maxSamples));
	return maxSamples;
}

int GLESRenderer::GetMaxAnisotropy()
{
	GLint maxAniso;
	GL_CHECK(glGetIntegerv(GL_MAX_TEXTURE_MAX_ANISOTROPY_EXT, &maxAniso));
	return maxAniso;
}

void GLESRenderer::ResetDrawCalls()
{
	_drawCalls = 0;
}

uint64_t GLESRenderer::GetDrawCalls()
{
	return _drawCalls;
}

uint64_t GLESRenderer::GetVideoMemorySize()
{
    return 0;
}

uint64_t GLESRenderer::GetUsedVideoMemorySize()
{
    return 0;
}

bool GLESRenderer::HasExtension(const char* extension)
{
    int numExtensions;

    GL_CHECK(glGetIntegerv(GL_NUM_EXTENSIONS, &numExtensions));
    size_t len = strlen(extension);

    for (int i = 0; i < numExtensions; i++)
	{
		const char *ext = (const char *)glGetStringi(GL_EXTENSIONS, i);
		if(!ext)
			continue;

		len = strlen(ext);

        if (!strncmp(ext, extension, len))
            return true;
	}

    return false;
}

#ifndef NE_PLATFORM_IOS
void GLESRenderer::BindDefaultFramebuffer()
{
    GL_CHECK(glBindFramebuffer(GL_FRAMEBUFFER, 0));
    _boundFramebuffer = nullptr;
}
#endif

GLESRenderer::~GLESRenderer()
{
    _DestroyContext();
}
