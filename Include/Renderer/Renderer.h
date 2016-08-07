/* NekoEngine
 *
 * Renderer.h
 * Author: Alexandru Naiman
 *
 * Rendering API abstraction
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

#include <stdint.h>

#include <string>
#include <unordered_map>

#include <Platform/Platform.h>
#include <Renderer/RBuffer.h>
#include <Renderer/RShader.h>
#include <Renderer/RTexture.h>
#include <Renderer/RFramebuffer.h>
#include <Renderer/RArrayBuffer.h>

// Conflicts with X11
#ifdef Always
#undef Always
#endif

#define RENDERER_API_VERSION	0x0026

#define R_CLEAR_COLOR			1
#define R_CLEAR_DEPTH			2
#define R_CLEAR_STENCIL			4

typedef void(*RendererDebugLogProc)(const char* msg);
typedef unsigned int(*RendererAPIVersionProc)(void);

enum class BufferType : uint8_t;
enum class TextureType : uint8_t;

enum class RendererCapability : uint8_t
{
	MemoryInformation = 0,
	AnisotropicFiltering = 1,
	MultisampledFramebuffer,
	PerSampleShading,
	DrawBaseVertex
};

enum class PolygonMode : uint8_t
{
	Triangles = 0,
	TriangleStrip = 1
};

enum class ElementType : uint8_t
{
	UnsignedByte = 0,
	UnsignedInt = 1
};

enum class PolygonFace : uint8_t
{
	Front = 0,
	Back = 1,
	FrontAndBack
};

enum class TestFunc : uint8_t
{
	Never = 0,
	Less = 1,
	LessOrEqual = 2,
	Greater = 3,
	GreaterOrEqual = 4,
	Equal = 5,
	NotEqual = 6,
	Always = 7
};

enum class TestOp : uint8_t
{
	Keep = 0,
	Zero = 1,
	Replace = 2,
	Increment = 3,
	IncrementWrap = 4,
	Decrement = 5,
	DecrementWrap = 6,
	Invert = 7
};

enum class BlendFactor : uint8_t
{
	Zero = 0,
	One = 1,
	SrcColor,
	OneMinusSrcColor,
	DstColor,
	OneMinusDstColor,
	SrcAlpha,
	OneMinusSrcAlpha,
	DstAlpha,
	OneMinusDstAlpha,
	ConstantColor,
	OneMinusConstantColor,
	ConstantAlpha,
	OneMinusConstantAlpha,
	SrcAlphaSaturate,
	Src1Color,
	OneMinusSrc1Color,
	Src1Alpha,
	OneMinusSrc1Alpha
};

enum class BlendEquation : uint8_t
{
	Add = 0,
	Subtract = 1,
	ReverseSubtract,
};

enum class FrontFace : uint8_t
{
	Clockwise = 0,
	CounterClockwise = 1
};

enum class TextureFileFormat : uint8_t
{
	DDS,
	KTX,
	TGA,
	WIC,
	PNG,
	JPG
};

enum class PixelStoreParameter : uint8_t
{
	PackSwapBytes = 0,
	PackLSBFirst,
	PackRowLength,
	PackImageHeight,
	PackSkipPixels,
	PackSkipRows,
	PackSkipImages,
	PackAlignment,
	UnpackSwapBytes,
	UnpackLSBFirst,
	UnpackRowLength,
	UnpackImageHeight,
	UnpackSkipPixels,
	UnpackSkipRows,
	UnpackSkipImages,
	UnpackAlignment	
};

class Renderer
{
public:
	
	/**
	 * Create a Renderer instance.
	 * You must call Initialize on this instance before use.
	 */
	Renderer() { };
	
	/**
	 * Initialize the rendering context
	 */
	virtual bool Initialize(PlatformWindowType hWnd, std::unordered_map<std::string, std::string> *args = nullptr, bool debug = false) = 0;

	/**
	 * Set a custom function for logging debug messages
	 */
	virtual void SetDebugLogFunction(RendererDebugLogProc debugLogFunction) = 0;

	/**
	 * The name of the rendering API
	 */
	virtual const char* GetName() = 0;
	
	/**
	 * Rendering API major version
	 */
	virtual int GetMajorVersion() = 0;
	
	/**
	 * Rendering API minor version
	 */
	virtual int GetMinorVersion() = 0;

	/**
	 * Set the screen clear color
	 */
	virtual void SetClearColor(float r, float g, float b, float a) = 0;
	
	/**
	 * Set viewport dimensions
	 */
	virtual void SetViewport(int x, int y, int width, int height) = 0;

	/**
	 * Enable the depth test
	 */
	virtual void EnableDepthTest(bool enable) = 0;

	/**
	 * Specify the value used for depth buffer comparisons
	 */
	virtual void SetDepthFunc(TestFunc func) = 0;

	/**
	 * Specify mapping of depth values from normalized device coordinates to window coordinates
	 */
	virtual void SetDepthRange(double near, double far) = 0;

	/**
	 * Specify mapping of depth values from normalized device coordinates to window coordinates
	*/
	virtual void SetDepthRangef(float near, float far) = 0;

	/**
	 * Enable or disable writing into the depth buffer
	 */
	virtual void SetDepthMask(bool mask) = 0;

	/**
	 * Enable the stencil test
	 */
	virtual void EnableStencilTest(bool enable) = 0;

	/*
	 * Set function and reference for stencil testing
	 */
	virtual void SetStencilFunc(TestFunc func, int ref, unsigned int mask) = 0;

	/*
	 * Set front and/or back function and reference for stencil testing
	 */
	virtual void SetStencilFuncSeparate(PolygonFace face, TestFunc func, int ref, unsigned int mask) = 0;

	/**
	 * Set front and back stencil test actions
	 */
	virtual void SetStencilOp(TestOp sfail, TestOp dpfail, TestOp dppass) = 0;

	/**
	 * Set front and/or back stencil test actions
	 */
	virtual void SetStencilOpSeparate(PolygonFace face, TestOp sfail, TestOp dpfail, TestOp dppass) = 0;

	/**
	 * Control the front and back writing of individual bits in the stencil planes
	 */
	virtual void SetStencilMask(unsigned int mask) = 0;

	/**
	 * Control the front and back writing of individual bits in the stencil planes
	 */
	virtual void SetStencilMaskSeparate(PolygonFace face, unsigned int mask) = 0;

	/**
	 * Enable blending
	 */
	virtual void EnableBlend(bool enable) = 0;

	/**
	 * Specify pixel arithmetic
	 */
	virtual void SetBlendFunc(BlendFactor src, BlendFactor dst) = 0;

	/*
	 * Specify pixel arithmetic for RGB and alpha components separately
	 */
	virtual void SetBlendFuncSeparate(BlendFactor srcColor, BlendFactor dstColor, BlendFactor srcAlpa, BlendFactor dstAlpha) = 0;

	/*
	 * Set the blend color
	 */
	virtual void SetBlendColor(float r, float g, float b, float a) = 0;

	/**
	 * Specify the equation used for both the RGB blend equation and the Alpha blend equation
	 */
	virtual void SetBlendEquation(BlendEquation eq) = 0;

	/**
	 * Set the RGB blend equation and the alpha blend equation separately
	 */
	virtual void SetBlendEquationSeparate(BlendEquation color, BlendEquation alpha) = 0;

	virtual void EnableFaceCulling(bool enable) = 0;

	virtual void SetFaceCulling(PolygonFace face) = 0;

	virtual void SetFrontFace(FrontFace face) = 0;
	
	virtual void SetColorMask(bool r, bool g, bool b, bool a) = 0;

	/**
	 * Render primitives from array data. Must have vertex buffer bound.
	 */
	virtual void DrawArrays(PolygonMode mode, int32_t first, int32_t count) = 0;
	
	/**
	 * Render primitives from array data. Must have vertex & index buffers bound.
	 */
	virtual void DrawElements(PolygonMode mode, int32_t count, ElementType type, const void *indices) = 0;
	
	virtual void DrawElementsBaseVertex(PolygonMode mode, int32_t count, ElementType type, const void *indices, int32_t baseVertex) = 0;

	/**
	 * Clear the active framebuffer
	 * Mask: R_CLEAR_COLOR, R_CLEAR_DEPTH, R_CLEAR_STENCIL
	 */
	virtual void Clear(uint32_t mask) = 0;

	virtual void BindDefaultFramebuffer() = 0;
	virtual RFramebuffer* GetBoundFramebuffer() = 0;	

	/**
	 * Specify the minimum rate at which sample shading takes place
	 * Functional only if the renderer has the PerSampleShading capability
	 */
	virtual void SetMinSampleShading(int32_t samples) = 0;

	virtual void SetSwapInterval(int swapInterval) = 0;

	/**
	 * Read a block of pixels from the framebuffer
	 */
	virtual void ReadPixels(int x, int y, int width, int height, TextureFormat format, TextureInternalType type, void* data) = 0;

	/**
	 * Set pixel storage mode
	 */
	virtual void SetPixelStore(PixelStoreParameter param, int value) = 0;

	/**
	 * Call this function when the drawable size changes
	 */
	virtual void ScreenResized() = 0;
	
	/**
	 * Swap the front & back buffers
	 */
	virtual void SwapBuffers() = 0;

	/**
	 * Check available renderer abilities
	 */
	virtual bool HasCapability(RendererCapability cap) = 0;

	/**
	 * Create a buffer object
	 */
	virtual class RBuffer* CreateBuffer(BufferType type, bool dynamic, bool persistent) = 0;
	
	/**
	 * Create a shader object
	 */
	virtual class RShader* CreateShader() = 0;
	
	/**
	 * Create a texture object
	 */
	virtual class RTexture* CreateTexture(TextureType type) = 0;
	
	/**
	 * Create a framebuffer object
	 */
	virtual class RFramebuffer* CreateFramebuffer(int width, int height) = 0;

	/**
	 * Create an array buffer object
	 */
	virtual class RArrayBuffer* CreateArrayBuffer() = 0;
	
	/**
	 * Add a define to be inserted into shader source
	 */
	virtual void AddShaderDefine(std::string name, std::string value) = 0;

	/**
	 * Check if this renderer supports the specified texture file format
	 */
	virtual bool IsTextureFormatSupported(TextureFileFormat format) = 0;

	/**
	 * Release resources
	 */
	virtual ~Renderer() { };

	/**
	 * Retrieves the total amount of memory available to the renderer
	 * Functional only if the renderer has the MemoryInformation capability
	 */
	virtual uint64_t GetVideoMemorySize() = 0;

	/**
	 * Retrieves the used amount of video memory
	 * Functional only if the renderer has the MemoryInformation capability
	 */
	virtual uint64_t GetUsedVideoMemorySize() = 0;
};

typedef Renderer*(*CreateRendererProc)();
