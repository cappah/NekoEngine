/* NekoEngine
 *
 * NullRenderer.h
 * Author: Alexandru Naiman
 *
 * Null Renderer Implementation
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
#include <Platform/Platform.h>

#include "NullArrayBuffer.h"
#include "NullBuffer.h"
#include "NullShader.h"
#include "NullTexture.h"

#include <stdlib.h>

#define RENDERER_VERSION_MAJOR		0
#define RENDERER_VERSION_MINOR		3
#define RENDERER_VERSION_REVISION	0
#define RENDERER_VERSION_BUILD		1
#define RENDERER_VERSION_STRING		"0.3.0.1"

class NullRenderer :
	public Renderer
{
public:
	NullRenderer() { _boundFramebuffer = nullptr; };

	virtual bool Initialize(PlatformWindowType hWnd, std::unordered_map<std::string, std::string> *args = nullptr, bool debug = false) override { return true; }

	virtual void SetDebugLogFunction(RendererDebugLogProc debugLogFunction) override { }

	virtual const char* GetName() override { return "NullRenderer"; }
	virtual int GetMajorVersion() override { return RENDERER_VERSION_MAJOR; }
	virtual int GetMinorVersion() override { return RENDERER_VERSION_MINOR; }

	virtual void SetClearColor(float r, float g, float b, float a) override { }
	virtual void SetViewport(int x, int y, int width, int height) override { }

	virtual void EnableDepthTest(bool enable) override { }

	virtual void SetDepthFunc(TestFunc func) override { }
	virtual void SetDepthRange(double near, double far) override { }
	virtual void SetDepthRangef(float near, float far) override { }
	virtual void SetDepthMask(bool mask) override { }

	virtual void EnableStencilTest(bool enable) override { }
	virtual void SetStencilFunc(TestFunc func, int ref, unsigned int mask) override { }
	virtual void SetStencilFuncSeparate(PolygonFace face, TestFunc func, int ref, unsigned int mask) override { }
	virtual void SetStencilOp(TestOp sfail, TestOp dpfail, TestOp dppass) override { }
	virtual void SetStencilOpSeparate(PolygonFace face, TestOp sfail, TestOp dpfail, TestOp dppass) override { }

	virtual void EnableBlend(bool enable) override { }
	virtual void SetBlendFunc(BlendFactor src, BlendFactor dst) override { }
	virtual void SetBlendFuncSeparate(BlendFactor srcColor, BlendFactor dstColor, BlendFactor srcAlpha, BlendFactor dstAlpha) override { }
	virtual void SetBlendColor(float r, float g, float b, float a) override { }
	virtual void SetBlendEquation(BlendEquation eq) override { }
	virtual void SetBlendEquationSeparate(BlendEquation color, BlendEquation alpha) override { }
	virtual void SetStencilMask(unsigned int mask) override { }
	virtual void SetStencilMaskSeparate(PolygonFace face, unsigned int mask) override { }

	virtual void EnableFaceCulling(bool enable) override { }
	virtual void SetFaceCulling(PolygonFace face) override { }
	virtual void SetFrontFace(FrontFace face) override { }
	virtual void SetColorMask(bool r, bool g, bool b, bool a) override { }

	virtual void DrawArrays(PolygonMode mode, int32_t first, int32_t count) override { }
	virtual void DrawElements(PolygonMode mode, int32_t count, ElementType type, const void *indices) override { }
	virtual void DrawElementsBaseVertex(PolygonMode mode, int32_t count, ElementType type, const void *indices, int32_t baseVertex) override { }
	virtual void Clear(uint32_t mask) override { }

	virtual void BindDefaultFramebuffer() override { _boundFramebuffer = nullptr; }
	virtual RFramebuffer* GetBoundFramebuffer() override { return _boundFramebuffer; }

	virtual void SetMinSampleShading(int32_t samples) override { }
	virtual void SetSwapInterval(int swapInterval) override { }

	virtual void ReadPixels(int x, int y, int width, int height, TextureFormat format, TextureInternalType type, void* data) override { }
	virtual void SetPixelStore(PixelStoreParameter param, int value) override { }

	virtual void ScreenResized() override { }
	virtual void SwapBuffers() override { }

	virtual bool HasCapability(RendererCapability cap) override { return false; }

	virtual class RBuffer* CreateBuffer(BufferType type, bool dynamic, bool persistent) override { return new NullBuffer(type, dynamic, persistent); }
	virtual class RShader* CreateShader() override { return new NullShader(); }
	virtual class RTexture* CreateTexture(TextureType type) override { return new NullTexture(type); }
	virtual class RFramebuffer* CreateFramebuffer(int width, int height) override;
	virtual class RArrayBuffer* CreateArrayBuffer() override { return new NullArrayBuffer(); }

	virtual void AddShaderDefine(std::string name, std::string value) override { }
	virtual bool IsTextureFormatSupported(TextureFileFormat format) override { return true; }

	virtual int GetMaxSamples() override { return 31; }
	virtual int GetMaxAnisotropy() override { return 16; }

	virtual bool IsHBAOSupported() { return false; }
	virtual bool InitializeHBAO() { return false; }
	virtual bool RenderHBAO(RHBAOArgs *args, RFramebuffer *fbo) { return false; }

	virtual uint64_t GetVideoMemorySize() override { return 0; }
	virtual uint64_t GetUsedVideoMemorySize() override { return 0; }

	// Internal functions
	static void SetBoundFramebuffer(RFramebuffer* fbo) { _boundFramebuffer = fbo; }

	virtual ~NullRenderer() { }

private:
	static RFramebuffer* _boundFramebuffer;
};