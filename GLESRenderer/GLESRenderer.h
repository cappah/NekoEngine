/* NekoEngine
 *
 * GLESRenderer.h
 * Author: Alexandru Naiman
 *
 * OpenGL|ES 3 Renderer Implementation
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

#ifndef GLESRenderer_h
#define GLESRenderer_h

#ifdef __APPLE__
	#include <OpenGLES/ES3/glext.h>
#else
	#include <GLES3/gl3.h>
    #include <GLES2/gl2ext.h>
#endif

#ifdef _DEBUG
#define GL_CHECK(x)\
x;																																			\
if(GLenum err = glGetError())																												\
{						\
fprintf(stderr, "%s call from %s, line %d returned 0x%x. Shutting down.\n", #x, __FILE__, __LINE__, err);				\
exit(-1);																																\
}
#else
#define GL_CHECK(x) x;
#endif

#define DIE(x) \
fprintf(stderr, "FATAL ERROR: %s\n", #x); \
exit(-1);

#include <Renderer/Renderer.h>
#include <Platform/Platform.h>
#include <string>

#define RENDERER_VERSION_MAJOR		0
#define RENDERER_VERSION_MINOR		3
#define RENDERER_VERSION_REVISION	0
#define RENDERER_VERSION_BUILD		90
#define RENDERER_VERSION_STRING		"0.3.0.90"

typedef struct SHADER_DEFINE
{
    std::string name;
    std::string value;
} ShaderDefine;

typedef struct COLOR
{
	int r;
	int g;
	int b;
	int a;
} Color;

typedef struct FUNC
{
	TestFunc F;
	int Ref;
	unsigned int Mask;
} Func;

typedef struct BLEND_FUNC
{
	BlendFactor src;
	BlendFactor dst;
} BlendFunc;

typedef struct RENDERER_STATE
{
	bool Blend;
	bool DepthTest;
	bool StencilTest;
	bool DepthMask;
	unsigned int StencilMask;

	Color ClearColor;

	Func StencilFunc;
	TestFunc DepthFunc;
	BlendFunc BlendFunction;

	struct VIEWPORT
	{
		float x;
		float y;
		float width;
		float height;
	} Viewport;

} RendererState;

#ifndef NE_PLATFORM_IOS
typedef struct GL_FUNCS
{
    PFNGLBUFFERSTORAGEEXTPROC BufferStorage;
    PFNGLPROGRAMUNIFORM1IEXTPROC ProgramUniform1i;
    PFNGLDRAWELEMENTSBASEVERTEXOESPROC DrawElementsBaseVertex;
} GLFuncs;
#endif

class GLESRenderer : public Renderer
{
public:
    GLESRenderer();

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
	virtual void SetPixelStore(PixelStoreParameter param, int value) override;

	virtual void ScreenResized() override;
    virtual void SwapBuffers() override;

    virtual bool HasCapability(RendererCapability cap) override;

    virtual class RBuffer *CreateBuffer(BufferType type, bool dynamic, bool persistent) override;
    virtual class RShader *CreateShader() override;
    virtual class RTexture *CreateTexture(TextureType type) override;
    virtual class RFramebuffer *CreateFramebuffer(int width, int height) override;
    virtual class RArrayBuffer *CreateArrayBuffer() override;
	virtual class RFence *CreateFence() override;

    virtual void AddShaderDefine(std::string name, std::string value) override;
    virtual bool IsTextureFormatSupported(TextureFileFormat format) override;

	virtual int GetMaxSamples() override;
	virtual int GetMaxAnisotropy() override;

    virtual void MakeCurrent(int context) override;

	virtual void ResetDrawCalls() override;
	virtual uint64_t GetDrawCalls() override;

	virtual bool IsHBAOSupported() override { return false; }
	virtual bool InitializeHBAO() override { return false; }
	virtual bool RenderHBAO(RHBAOArgs *args, RFramebuffer *fbo) override { return false; }

    virtual uint64_t GetVideoMemorySize() override;
    virtual uint64_t GetUsedVideoMemorySize() override;

    virtual ~GLESRenderer();

    // Internal functions
    static void SetBoundFramebuffer(RFramebuffer* fbo) { _boundFramebuffer = fbo; }
    static std::vector<ShaderDefine>& GetShaderDefines() { return _shaderDefines; }
	static void SetActiveShader(class GLESShader *shader) { _activeShader = shader; }
	static void MakeCurrent();
	static bool HasExtension(const char* extension);
	
#ifndef NE_PLATFORM_IOS
	static GLFuncs Funcs;
#endif
	
private:
    PlatformWindowType _window;
	static class GLESShader* _activeShader;
    static RFramebuffer* _boundFramebuffer;
    static std::vector<ShaderDefine> _shaderDefines;

	uint64_t _drawCalls;
	RendererState _state;

    void _DestroyContext();
};

extern RendererDebugLogProc _debugLogFunc;

void LogDebugOutput(GLenum source, GLenum type, GLuint id, GLenum severity, const char* msg);
void DebugCallbackAMD(GLuint id, GLenum category, GLenum severity, GLsizei length, const GLchar* message, GLvoid* userParam);
void DebugCallback(GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, const GLchar* message, const GLvoid* userParam);

/*
 * These functions and constants do not exist in Apple's OpenGL implementation
 */
#ifdef __APPLE__

#define glBufferStorageEXT(x, y, z, w)

#define GL_MAP_PERSISTENT_BIT_EXT	0
#define GL_MAP_COHERENT_BIT_EXT		0
#define GL_DYNAMIC_STORAGE_BIT_EXT	0

#else

#define glBufferStorageEXT(x, y, z, w) GLESRenderer::Funcs.BufferStorage(x, y, z, w)
#define glProgramUniform1iEXT(x, y, z) GLESRenderer::Funcs.ProgramUniform1i(x, y, z)
#define glDrawElementsBaseVertex(x, y, z, w, a) GLESRenderer::Funcs.DrawElementsBaseVertex(x, y, z, w, a)

#endif

#endif /* GLESRenderer_h */
