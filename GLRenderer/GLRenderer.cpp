/* Neko Engine
 *
 * GLRenderer.cpp
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

#include "GLRenderer.h"
#include "GLBuffer.h"
#include "GLShader.h"
#include "GLTexture.h"
#include "GLFramebuffer.h"
#include "GLArrayBuffer.h"

#include <Renderer/Renderer.h>
#include <Platform/Platform.h>

#include <stdio.h>
#include <string.h>

#ifndef GL_GPU_MEMORY_INFO_TOTAL_AVAILABLE_MEMORY_NVX
#define GL_GPU_MEMORY_INFO_TOTAL_AVAILABLE_MEMORY_NVX 0
#endif

#ifndef GL_GPU_MEMORY_INFO_CURRENT_AVAILABLE_VIDMEM_NVX
#define GL_GPU_MEMORY_INFO_CURRENT_AVAILABLE_VIDMEM_NVX 0
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

RendererDebugLogProc _debugLogFunc = nullptr;
RFramebuffer* GLRenderer::_boundFramebuffer = nullptr;
std::vector<ShaderDefine> GLRenderer::_shaderDefines;
GLShader* GLRenderer::_activeShader;
GLRendererConfig GLRenderer::_configuration = { true, true, true };

GLRenderer::GLRenderer()
{
	_ctx = nullptr;
	_dc = nullptr;
	_hWnd = (PlatformWindowType)0;
	_haveDSA = false;
}

void GLRenderer::SetDebugLogFunction(RendererDebugLogProc debugLog)
{
	_debugLogFunc = debugLog;
}

const char* GLRenderer::GetName()
{
	return "OpenGL";
}

int GLRenderer::GetMajorVersion()
{
	GLint ver;
	GL_CHECK(glGetIntegerv(GL_MAJOR_VERSION, &ver));
	return ver;
}

int GLRenderer::GetMinorVersion()
{
	GLint ver;
	GL_CHECK(glGetIntegerv(GL_MINOR_VERSION, &ver));
	return ver;
}
 
void GLRenderer::SetClearColor(float r, float g, float b, float a)
{
	GL_CHECK(glClearColor(r, g, b, a));
}

void GLRenderer::SetViewport(int x, int y, int width, int height)
{
	GL_CHECK(glViewport(x, y, width, height));
}

void GLRenderer::EnableDepthTest(bool enable)
{
	if (enable)
	{ GL_CHECK(glEnable(GL_DEPTH_TEST)); }
	else
	{ GL_CHECK(glDisable(GL_DEPTH_TEST)); }
}

void GLRenderer::SetDepthFunc(TestFunc func)
{
	GL_CHECK(glDepthFunc(GL_TestFunc[(int)func]));
}

void GLRenderer::SetDepthRange(double n, double f)
{
	GL_CHECK(glDepthRange(n, f));
}

void GLRenderer::SetDepthRangef(float n, float f)
{
	GL_CHECK(glDepthRangef(n, f));
}

void GLRenderer::SetDepthMask(bool mask)
{
	GL_CHECK(glDepthMask(mask ? GL_TRUE : GL_FALSE));
}

void GLRenderer::EnableStencilTest(bool enable)
{
	if (enable)
	{ GL_CHECK(glEnable(GL_STENCIL_TEST)); }
	else
	{ GL_CHECK(glDisable(GL_STENCIL_TEST)); }
}

void GLRenderer::SetStencilFunc(TestFunc func, int ref, unsigned int mask)
{
	GL_CHECK(glStencilFunc(GL_TestFunc[(int)func], ref, mask));
}

void GLRenderer::SetStencilFuncSeparate(PolygonFace face, TestFunc func, int ref, unsigned int mask)
{
	GL_CHECK(glStencilFuncSeparate(GL_PolygonFace[(int)face], GL_TestFunc[(int)func], ref, mask));
}

void GLRenderer::SetStencilOp(TestOp sfail, TestOp dpfail, TestOp dppass)
{
	GL_CHECK(glStencilOp(GL_TestOp[(int)sfail], GL_TestOp[(int)dpfail], GL_TestOp[(int)dppass]));
}

void GLRenderer::SetStencilOpSeparate(PolygonFace face, TestOp sfail, TestOp dpfail, TestOp dppass)
{
	GL_CHECK(glStencilOpSeparate(GL_PolygonFace[(int)face], GL_TestOp[(int)sfail], GL_TestOp[(int)dpfail], GL_TestOp[(int)dppass]));
}

void GLRenderer::EnableBlend(bool enable)
{
	if (enable)
	{ GL_CHECK(glEnable(GL_BLEND)); }
	else
	{ GL_CHECK(glDisable(GL_BLEND)); }
}

void GLRenderer::SetBlendFunc(BlendFactor src, BlendFactor dst)
{
	GL_CHECK(glBlendFunc(GL_BlendFactor[(int)src], GL_BlendFactor[(int)dst]));
}

void GLRenderer::SetBlendFuncSeparate(BlendFactor srcColor, BlendFactor dstColor, BlendFactor srcAlpha, BlendFactor dstAlpha)
{
	GL_CHECK(glBlendFuncSeparate(GL_BlendFactor[(int)srcColor], GL_BlendFactor[(int)dstColor], GL_BlendFactor[(int)srcAlpha], GL_BlendFactor[(int)dstAlpha]));
}

void GLRenderer::SetBlendColor(float r, float g, float b, float a)
{
	GL_CHECK(glBlendColor(r, g, b, a));
}

void GLRenderer::SetBlendEquation(BlendEquation eq)
{
	GL_CHECK(glBlendEquation(GL_BlendEquation[(int)eq]));
}

void GLRenderer::SetBlendEquationSeparate(BlendEquation color, BlendEquation alpha)
{
	GL_CHECK(glBlendEquationSeparate(GL_BlendEquation[(int)color], GL_BlendEquation[(int)alpha]));
}

void GLRenderer::SetStencilMask(unsigned int mask)
{
	GL_CHECK(glStencilMask(mask));
}

void GLRenderer::SetStencilMaskSeparate(PolygonFace face, unsigned int mask)
{
	GL_CHECK(glStencilMaskSeparate(GL_PolygonFace[(int)face], mask));
}

void GLRenderer::EnableFaceCulling(bool enable)
{
	if (enable)
	{ GL_CHECK(glEnable(GL_CULL_FACE)); }
	else
	{ GL_CHECK(glDisable(GL_CULL_FACE)); }
}

void GLRenderer::SetFaceCulling(PolygonFace face)
{
	GL_CHECK(glCullFace(GL_PolygonFace[(int)face]));
}

void GLRenderer::SetFrontFace(FrontFace face)
{
	GL_CHECK(glFrontFace(GL_FrontFace[(int)face]));
}

void GLRenderer::SetColorMask(bool r, bool g, bool b, bool a)
{
	GL_CHECK(glColorMask(r ? GL_TRUE : GL_FALSE,
		g ? GL_TRUE : GL_FALSE,
		b ? GL_TRUE : GL_FALSE,
		a ? GL_TRUE : GL_FALSE));
}

void GLRenderer::DrawArrays(PolygonMode mode, int32_t first, int32_t count)
{
	_activeShader->EnableTextures();
	
	if(!_haveDSA)
	{
		if (_boundFramebuffer)
			_boundFramebuffer->Bind(FB_DRAW);
		else
			BindDefaultFramebuffer();
	}
	
	GL_CHECK(glDrawArrays(GL_DrawModes[(int)mode], first, (GLsizei)count));
}

void GLRenderer::DrawElements(PolygonMode mode, int32_t count, ElementType type, const void *indices)
{
	_activeShader->EnableTextures();
	
	if(!_haveDSA)
	{
		if (_boundFramebuffer)
			_boundFramebuffer->Bind(FB_DRAW);
		else
			BindDefaultFramebuffer();
	}
	
	GL_CHECK(glDrawElements(GL_DrawModes[(int)mode], (GLsizei)count, GL_ElementType[(int)type], indices));
}

void GLRenderer::DrawElementsBaseVertex(PolygonMode mode, int32_t count, ElementType type, const void *indices, int32_t baseVertex)
{
	_activeShader->EnableTextures();
	
	if(!_haveDSA)
	{
		if (_boundFramebuffer)
			_boundFramebuffer->Bind(FB_DRAW);
		else
			BindDefaultFramebuffer();
	}
	
	GL_CHECK(glDrawElementsBaseVertex(GL_DrawModes[(int)mode], (GLsizei)count, GL_ElementType[(int)type], indices, baseVertex));
}

void GLRenderer::Clear(uint32_t mask)
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

void GLRenderer::BindDefaultFramebuffer()
{
	GL_CHECK(glBindFramebuffer(GL_FRAMEBUFFER, 0));
	_boundFramebuffer = nullptr;
}

RFramebuffer* GLRenderer::GetBoundFramebuffer()
{
	return _boundFramebuffer;
}

void GLRenderer::SetMinSampleShading(int32_t samples)
{
	GL_CHECK(glMinSampleShading((GLfloat)samples));
}

void GLRenderer::ReadPixels(int x, int y, int width, int height, TextureFormat format, TextureInternalType type, void* data)
{
	GL_CHECK(glReadPixels(x, y, width, height, GL_TexFormat[(int)format], GL_TexType[(int)type], data));
}

bool GLRenderer::HasCapability(RendererCapability cap)
{
	const char *ext = nullptr;

	switch (cap)
	{
	case RendererCapability::MemoryInformation:
		ext = "GL_NVX_gpu_memory_info";
		break;
	case RendererCapability::AnisotropicFiltering:
		ext = "GL_EXT_texture_filter_anisotropic";
		break;
	case RendererCapability::MultisampledFramebuffer:
		ext = "GL_EXT_framebuffer_multisample";
		break;
	case RendererCapability::PerSampleShading:
		ext = "GL_ARB_sample_shading";
		break;
	case RendererCapability::DrawBaseVertex: // Supported in OpenGL 3.2+
		return true;
	default:
		return false;
	}

	return HasExtension(ext);
}

RBuffer* GLRenderer::CreateBuffer(BufferType type, bool dynamic, bool persistent)
{
	if (_haveDSA)
		return (RBuffer *)new GLBuffer(type, dynamic, persistent);
	else
		return (RBuffer *)new GLBuffer_NoDSA(type, dynamic, persistent);
}

RShader* GLRenderer::CreateShader()
{
	return (RShader *)new GLShader();
}

RTexture* GLRenderer::CreateTexture(TextureType type)
{
	if(_haveDSA)
		return (RTexture *)new GLTexture(type);
	else
		return (RTexture *)new GLTexture_NoDSA(type);
}

RFramebuffer* GLRenderer::CreateFramebuffer(int width, int height)
{
	if(_haveDSA)
		return (RFramebuffer *)new GLFramebuffer(width, height);
	else
		return (RFramebuffer *)new GLFramebuffer_NoDSA(width, height);
}

RArrayBuffer* GLRenderer::CreateArrayBuffer()
{
	return (RArrayBuffer *)new GLArrayBuffer();
}

void GLRenderer::AddShaderDefine(std::string name, std::string value)
{
	ShaderDefine d{name, value};
	_shaderDefines.push_back(d);
}

bool GLRenderer::IsTextureFormatSupported(TextureFileFormat format)
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

uint64_t GLRenderer::GetVideoMemorySize()
{
	GLint mem;
	GL_CHECK(glGetIntegerv(GL_GPU_MEMORY_INFO_TOTAL_AVAILABLE_MEMORY_NVX, &mem));
	return mem;
}

uint64_t GLRenderer::GetUsedVideoMemorySize()
{
	GLint mem;
	GL_CHECK(glGetIntegerv(GL_GPU_MEMORY_INFO_CURRENT_AVAILABLE_VIDMEM_NVX, &mem));
	return mem;
}

bool GLRenderer::HasExtension(const char* extension)
{
	int numExtensions;

	glGetIntegerv(GL_NUM_EXTENSIONS, &numExtensions);
	size_t len = strlen(extension);

	for (int i = 0; i < numExtensions; i++)
		if (!strncmp((char *)glGetStringi(GL_EXTENSIONS, i), extension, len))
			return true;

	return false;
}

void GLRenderer::_CheckExtensions()
{
	_haveDSA = HasExtension("GL_ARB_direct_state_access");
}

void GLRenderer::_ParseArguments(unordered_map<string, string> *args)
{
	for(pair<string, string> kvp : *args)
	{
		if(!kvp.first.compare("bEnableSubroutines"))
			_configuration.EnableSubroutines = kvp.second.compare("0");
		else if(!kvp.first.compare("bEnableBindlessTexture"))
			_configuration.EnableBindlessTexture = kvp.second.compare("0");
		else if(!kvp.first.compare("bEnableExplicitUniforms"))
			_configuration.EnableExplicitUniforms = kvp.second.compare("0");
	}
}

GLRenderer::~GLRenderer()
{
	_DestroyContext();
}
