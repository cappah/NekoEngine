/* Neko Engine
 *
 * GLRenderer.h
 * Author: Alexandru Naiman
 *
 * OpenGL Renderer Implementation
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

#ifdef __APPLE__
	#include <OpenGL/gl3.h>
	#include <OpenGL/gl3ext.h>
#else
	#include "glad.h"
#endif

#include <Renderer/Renderer.h>
#include <Platform/Platform.h>

#include <stdlib.h>

#include <vector>

#define RENDERER_VERSION_MAJOR		0
#define RENDERER_VERSION_MINOR		3
#define RENDERER_VERSION_REVISION	0
#define RENDERER_VERSION_BUILD		80
#define RENDERER_VERSION_STRING		"0.3.0.80"

#if defined(_WIN32) || defined(_WIN64)
#include <Windows.h>
typedef HGLRC 		RHI_GL_CTX;
typedef HDC		RHI_DC;
#elif defined(PLATFORM_X11)
#include <X11/Xlib.h>
#include <GL/glx.h>
typedef GLXContext 	RHI_GL_CTX;
typedef Display* 	RHI_DC;
#elif defined(__APPLE__)
typedef NSOpenGLContext* RHI_GL_CTX;
typedef NSView*		RHI_DC;
#endif

#include <stdio.h>

/**
* Check glGetError() log a critical message and close the program if an error has occured
*/

#ifdef _DEBUG
	#ifdef _WIN32 
	#define GL_CHECK(x)																														\
		x;																																			\
		if(GLenum err = glGetError())																												\
		{						\
			if(IsDebuggerPresent()) \
			{ \
				char buff[1024];	\
				if(snprintf(buff, 1024, "%s call from %s, line %d returned 0x%x. Shutting down.\n", #x, __FILE__, __LINE__, err) >= 1024) \
					OutputDebugStringA("OpenGL call failed. Shutting down.\n"); \
				else \
					OutputDebugStringA(buff); \
				exit(-1); \
			} \
			else \
			{ \
				fprintf(stderr, "%s call from %s, line %d returned 0x%x. Shutting down.", #x, __FILE__, __LINE__, err);				\
				getchar(); \
				exit(-1); \
			} \
		}
	#else
		#define GL_CHECK(x)																														\
		x;																																			\
		if(GLenum err = glGetError())																												\
		{						\
			fprintf(stderr, "%s call from %s, line %d returned 0x%x. Shutting down.", #x, __FILE__, __LINE__, err);				\
			exit(-1);																																\
		}
	#endif

	

#else
	#define GL_CHECK(x) x;
#endif

#ifdef _WIN32
#define DIE(x) \
		if(IsDebuggerPresent()) \
		{ \
			OutputDebugStringA("FATAL ERROR: "); \
			OutputDebugStringA(x); \
			OutputDebugStringA("\n"); \
			exit(-1); \
		} \
		else \
		{ \
			fprintf(stderr, "FATAL ERROR: %s\n", #x); \
			getchar(); \
			exit(-1); \
		} 
#else
#define DIE(x) \
		fprintf(stderr, "FATAL ERROR: %s\n", #x); \
		exit(-1);
#endif

typedef struct SHADER_DEFINE
{
	std::string name;
	std::string value;
} ShaderDefine;

typedef struct GL_RENDERER_CONFIG
{
	bool EnableExplicitUniforms;	
	bool EnableBindlessTexture;
	bool EnableSubroutines;
} GLRendererConfig;

class GLRenderer :
	public Renderer
{
public:
	GLRenderer();

	virtual bool Initialize(PlatformWindowType hWnd, std::unordered_map<std::string, std::string> *args = nullptr, bool debug = false) override;

	virtual void SetDebugLogFunction(RendererDebugLogProc debugLogFunction) override;

	virtual const char* GetName() override;
	virtual int GetMajorVersion() override;
	virtual int GetMinorVersion() override;

	virtual void SetClearColor(float r, float g, float b, float a) override;
	virtual void SetViewport(int x, int y, int width, int height) override;

	virtual void EnableDepthTest(bool enable) override;

	virtual void SetDepthFunc(TestFunc func) override;
	virtual void SetDepthRange(double near, double far) override;
	virtual void SetDepthRangef(float near, float far) override;
	virtual void SetDepthMask(bool mask) override;

	virtual void EnableStencilTest(bool enable) override;
	virtual void SetStencilFunc(TestFunc func, int ref, unsigned int mask) override;
	virtual void SetStencilFuncSeparate(PolygonFace face, TestFunc func, int ref, unsigned int mask) override;
	virtual void SetStencilOp(TestOp sfail, TestOp dpfail, TestOp dppass) override;
	virtual void SetStencilOpSeparate(PolygonFace face, TestOp sfail, TestOp dpfail, TestOp dppass) override;

	virtual void EnableBlend(bool enable) override;
	virtual void SetBlendFunc(BlendFactor src, BlendFactor dst) override;
	virtual void SetBlendFuncSeparate(BlendFactor srcColor, BlendFactor dstColor, BlendFactor srcAlpha, BlendFactor dstAlpha) override;
	virtual void SetBlendColor(float r, float g, float b, float a) override;
	virtual void SetBlendEquation(BlendEquation eq) override;
	virtual void SetBlendEquationSeparate(BlendEquation color, BlendEquation alpha) override;
	virtual void SetStencilMask(unsigned int mask) override;
	virtual void SetStencilMaskSeparate(PolygonFace face, unsigned int mask) override;

	virtual void EnableFaceCulling(bool enable) override;
	virtual void SetFaceCulling(PolygonFace face) override;
	virtual void SetFrontFace(FrontFace face) override;
	virtual void SetColorMask(bool r, bool g, bool b, bool a) override;

	virtual void DrawArrays(PolygonMode mode, int32_t first, int32_t count) override;
	virtual void DrawElements(PolygonMode mode, int32_t count, ElementType type, const void *indices) override;
	virtual void DrawElementsBaseVertex(PolygonMode mode, int32_t count, ElementType type, const void *indices, int32_t baseVertex) override;
	virtual void Clear(uint32_t mask) override;

	virtual void BindDefaultFramebuffer() override;
	virtual RFramebuffer* GetBoundFramebuffer() override;

	virtual void SetMinSampleShading(int32_t samples) override;
	virtual void SetSwapInterval(int swapInterval) override;

	virtual void ReadPixels(int x, int y, int width, int height, TextureFormat format, TextureInternalType type, void* data) override;

	virtual void SwapBuffers() override;

	virtual bool HasCapability(RendererCapability cap) override;

	virtual class RBuffer* CreateBuffer(BufferType type, bool dynamic, bool persistent) override;
	virtual class RShader* CreateShader() override;
	virtual class RTexture* CreateTexture(TextureType type) override;
	virtual class RFramebuffer* CreateFramebuffer(int width, int height) override;
	virtual class RArrayBuffer* CreateArrayBuffer() override;
	
	virtual void AddShaderDefine(std::string name, std::string value) override;
	virtual bool IsTextureFormatSupported(TextureFileFormat format) override;

	virtual uint64_t GetVideoMemorySize() override;
	virtual uint64_t GetUsedVideoMemorySize() override;

	virtual ~GLRenderer();

	// Internal functions
	static void SetBoundFramebuffer(RFramebuffer* fbo) { _boundFramebuffer = fbo; }
	static std::vector<ShaderDefine>& GetShaderDefines() { return _shaderDefines; }
	static void SetActiveShader(class GLShader *shader) { _activeShader = shader; }
	static bool HasExtension(const char* extension);
	static GLRendererConfig *GetConfiguration() { return &_configuration; }

private:
	RHI_GL_CTX _ctx;
	RHI_DC _dc;
	PlatformWindowType _hWnd;
	bool _haveDSA;
	static RFramebuffer* _boundFramebuffer;
	static std::vector<ShaderDefine> _shaderDefines;
	static class GLShader* _activeShader;
	static GLRendererConfig _configuration;
	
	void _DestroyContext();
	void _CheckExtensions();
	void _ParseArguments(std::unordered_map<std::string, std::string> *args);
};

extern RendererDebugLogProc _debugLogFunc;

void LogDebugOutput(GLenum source, GLenum type, GLuint id, GLenum severity, const char* msg);
void DebugCallbackAMD(GLuint id, GLenum category, GLenum severity, GLsizei length, const GLchar* message, GLvoid* userParam);
void DebugCallback(GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, const GLchar* message, const GLvoid* userParam);

/*
 * These functions and constants do not exist in Apple's OpenGL implementation
 */
#ifdef __APPLE__

#define glCreateBuffers(x, y)
#define glNamedBufferStorage(x, y, z, w)
#define glMapNamedBufferRange(x, y, z, w) NULL
#define glUnmapNamedBuffer(x)
#define glNamedBufferSubData(x, y, z, w)
#define glBufferStorage(x, y, z, w)

#define glCreateTextures(x, y, z)
#define glGetTextureHandleARB(x) 0
#define glMakeTextureHandleResidentARB(x)
#define glTextureStorage1D(a, b, c, d)
#define glTextureStorage2D(a, b, c, d, e)
#define glTextureStorage3D(a, b, c, d, e, f)
#define glTextureStorage2DMultisample(a, b, c, d, e, f)
#define glTextureSubImage1D(a, b, c, d, e, f, g)
#define glTextureSubImage2D(a, b, c, d, e, f, g, h, i)
#define glTextureSubImage3D(a, b, c, d, e, f, g, h, i, j, k)
#define glCompressedTextureSubImage1D(a, b, c, d, e, f, g)
#define glCompressedTextureSubImage2D(a, b, c, d, e, f, g, h, i)
#define glCompressedTextureSubImage3D(a, b, c, d, e, f, g, h, i, j, k)
#define glTextureParameteri(x, y, z)
#define glGenerateTextureMipmap(x)
#define glMakeTextureHandleNonResidentARB(x)

#define glCreateFramebuffers(x, y)
#define glNamedFramebufferTexture(x, y, z, w)
#define glCreateRenderbuffers(x, y)
#define glNamedRenderbufferStorage(x, y, z, w)
#define glNamedFramebufferRenderbuffer(x, y, w, z)
#define glNamedRenderbufferStorageMultisample(a, b, c, d, e)
#define glCheckNamedFramebufferStatus(x, y) 0
#define glBlitNamedFramebuffer(a, b, c, d, e, f, g, h, i, j, k, l)
#define glNamedFramebufferDrawBuffer(x, y)
#define glNamedFramebufferDrawBuffers(x, y, z)

#define GL_MAP_PERSISTENT_BIT	0
#define GL_MAP_COHERENT_BIT		0
#define GL_DYNAMIC_STORAGE_BIT	0

#define GL_DEBUG_CATEGORY_API_ERROR_AMD 0
#define GL_DEBUG_SOURCE_API 1
#define GL_DEBUG_CATEGORY_APPLICATION_AMD 2
#define GL_DEBUG_SOURCE_APPLICATION 3
#define GL_DEBUG_CATEGORY_WINDOW_SYSTEM_AMD 4
#define GL_DEBUG_SOURCE_WINDOW_SYSTEM 5
#define GL_DEBUG_CATEGORY_SHADER_COMPILER_AMD 6
#define GL_DEBUG_SOURCE_SHADER_COMPILER 7
#define GL_DEBUG_SOURCE_THIRD_PARTY 8
#define GL_DEBUG_CATEGORY_OTHER_AMD 9
#define GL_DEBUG_SOURCE_OTHER 10

#define GL_DEBUG_TYPE_ERROR 0
#define GL_DEBUG_CATEGORY_DEPRECATION_AMD 1
#define GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR 2
#define GL_DEBUG_CATEGORY_UNDEFINED_BEHAVIOR_AMD 3
#define GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR 4
#define GL_DEBUG_TYPE_PORTABILITY_ARB 5
#define GL_DEBUG_CATEGORY_PERFORMANCE_AMD 6
#define GL_DEBUG_TYPE_PERFORMANCE 7
#define GL_DEBUG_TYPE_OTHER 8

#define GL_DEBUG_SEVERITY_HIGH 0
#define GL_DEBUG_SEVERITY_MEDIUM 1
#define GL_DEBUG_SEVERITY_LOW 2

#endif
