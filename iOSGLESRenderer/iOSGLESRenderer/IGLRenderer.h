/* Neko Engine
 *
 * IGLRenderer.h
 * Author: Alexandru Naiman
 *
 * iOS OpenGL|ES Renderer Implementation
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

#ifndef IGLRenderer_h
#define IGLRenderer_h

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

#import <Foundation/Foundation.h>
#import <UIKit/UIKit.h>
#include <Renderer/Renderer.h>
#include <Platform/Platform.h>
#include <string>

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
	BlendFunc BlendFunc;
	
	struct VIEWPORT
	{
		float x;
		float y;
		float width;
		float height;
	} Viewport;
	
} RendererState;

class IGLRenderer : public Renderer
{
public:
    IGLRenderer();
    
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
    
    virtual ~IGLRenderer();
    
    // Internal functions
    static void SetBoundFramebuffer(RFramebuffer* fbo) { _boundFramebuffer = fbo; }
    static std::vector<ShaderDefine>& GetShaderDefines() { return _shaderDefines; }
	static void SetActiveShader(class IGLShader *shader) { _activeShader = shader; }
	static void MakeCurrent();
    
private:
    UIWindow *_window;
	static class IGLShader* _activeShader;
    static RFramebuffer* _boundFramebuffer;
    static std::vector<ShaderDefine> _shaderDefines;
	RendererState _state;
    
    bool _HasExtension(const char* extension);
    void _DestroyContext();
};

extern RendererDebugLogProc _debugLogFunc;

void LogDebugOutput(GLenum source, GLenum type, GLuint id, GLenum severity, const char* msg);
void DebugCallbackAMD(GLuint id, GLenum category, GLenum severity, GLsizei length, const GLchar* message, GLvoid* userParam);
void DebugCallback(GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, const GLchar* message, const GLvoid* userParam);

#endif /* IGLRenderer_h */
