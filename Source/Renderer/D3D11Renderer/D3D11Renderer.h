/* NekoEngine
 *
 * D3D11Renderer.h
 * Author: Alexandru Naiman
 *
 * DirectX 11 Renderer Implementation
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

#include <Renderer/Renderer.h>
#include "D3D11Context.h"

#include <Windows.h>
#include <windowsx.h>
#include <d3d11.h>
#include <d3d11_1.h>
#include <d3d11_2.h>

#define RENDERER_VERSION_MAJOR		0
#define RENDERER_VERSION_MINOR		3
#define RENDERER_VERSION_REVISION	0
#define RENDERER_VERSION_BUILD		41
#define RENDERER_VERSION_STRING		"0.3.0.41"

typedef struct SHADER_DEFINE
{
	std::string name;
	std::string value;
} ShaderDefine;

class D3D11Renderer :
	public Renderer
{
public:
	D3D11Renderer();

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

	virtual void DrawArrays(PolygonMode mode, int32_t first, int count) override;
	virtual void DrawElements(PolygonMode mode, int count, ElementType type, const void *indices) override;
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
	virtual void MakeCurrent(int context) override { (void)context; }

	virtual int GetMaxSamples() override;
	virtual int GetMaxAnisotropy() override;

	virtual void ResetDrawCalls() { _drawCalls = 0; }
	virtual uint64_t GetDrawCalls() { return _drawCalls; }

	virtual bool IsHBAOSupported() override;
	virtual bool InitializeHBAO() override;
	virtual bool RenderHBAO(RHBAOArgs *args, RFramebuffer *fbo) override;

	virtual const char *GetShadingLanguage() override { return "hlsl"; }

	virtual uint64_t GetVideoMemorySize() override;
	virtual uint64_t GetUsedVideoMemorySize() override;

	// Internal functions
	/*static std::vector<ShaderDefine>& GetShaderDefines() { return _shaderDefines; }*/
	static void SetBoundFramebuffer(class D3D11Framebuffer *fbo) { _boundFramebuffer = fbo; }
	static void SetActiveShader(class D3D11Shader *shader) { _activeShader = shader; }
	static void SetActiveArrayBuffer(class D3D11ArrayBuffer *abuff) { _activeArrayBuffer = abuff; }
	static class D3D11ArrayBuffer *GetActiveArrayBuffer() { return _activeArrayBuffer; }
	/*static bool HasExtension(const char* extension);
	static GLRendererConfig *GetConfiguration() { return &_configuration; }*/

	virtual ~D3D11Renderer();

private:
	D3D11Context _ctx;

	IDXGISwapChain *_swapChain;
	IDXGISwapChain1 *_swapChain1;
	ID3D11Device1 *_device1;
	ID3D11RasterizerState *_rasterizerState;

	D3D_DRIVER_TYPE _driverType;
	D3D_FEATURE_LEVEL _featureLevel;

	D3D11_RASTERIZER_DESC _rasterizerDesc;

	float _clearColor[4];
	static std::vector<ShaderDefine> _shaderDefines;
	static class D3D11Shader *_activeShader;
	static class D3D11ArrayBuffer *_activeArrayBuffer;
	static class D3D11Framebuffer *_boundFramebuffer;

	// move to pipeline
	D3D11_CULL_MODE _cullMode;
	D3D11_BLEND_DESC _blendState;
	D3D11_DEPTH_STENCIL_DESC _depthStencilStateDesc;

	ID3D11DepthStencilState *_depthStencilState;
	ID3D11BlendState *_enableAlpha;
	ID3D11BlendState *_disableAlpha;

	float _blendFactor[4];
	int _stencilRef;
	unsigned int _syncInterval;
	uint64_t _drawCalls;
};

