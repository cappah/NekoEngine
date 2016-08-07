/* NekoEngine
 *
 * DeferredRenderer.h
 * Author: Alexandru Naiman
 *
 * Deferred rendering buffer
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

#define GB_TEX_POSITION					0
#define GB_TEX_NORMAL					1
#define GB_TEX_COLOR_SPECULAR				2
#define GB_TEX_MATERIAL_INFO				3
#define GB_TEX_DEPTH_STENCIL				4
#define GB_TEX_LIGHT					5
#define GB_TEX_BRIGHT					6
#define GB_TEX_LIGHT_ACCUM				7

#define GB_FBO_GEOMETRY					0
#define GB_FBO_LIGHT					1
#define GB_FBO_BRIGHT					2
#define GB_FBO_LIGHT_ACCUM				3

#include <Engine/Engine.h>
#include <Engine/Shader.h>
#include <Scene/Object.h>
#include <Engine/SSAO.h>
#include <Renderer/Renderer.h>
#include <Engine/ShadowMap.h>

typedef struct LIGHT_SCENE_DATA
{
	glm::vec4 CameraPositionAndAmbient;
	glm::vec4 AmbientColorAndRClear;
	glm::vec4 FogColorAndRFog;
	glm::vec4 FrameSizeAndSSAO;
} LightSceneData;

typedef struct LIGHT_DATA
{
	glm::vec3 LightPosition;
	float padding0;
	glm::vec3 LightColor;
	float padding1;
	glm::vec3 LightDirection;
	float padding2;
	glm::vec4 LightAttenuationAndData;
} LightData;

class DeferredBuffer
{
public:
	ENGINE_API static int Initialize() noexcept;

	ENGINE_API static int GetWidth() noexcept { return _fboWidth; }
	ENGINE_API static int GetHeight() noexcept { return _fboHeight; }
	
	ENGINE_API static RTexture* GetPositionTexture() noexcept { return _gbTextures[GB_TEX_POSITION]; }
	ENGINE_API static RTexture* GetNormalTexture() noexcept { return _gbTextures[GB_TEX_NORMAL]; }
	ENGINE_API static RTexture* GetDepthTexture() noexcept { return _gbTextures[GB_TEX_DEPTH_STENCIL]; }

	ENGINE_API static RShader* GetGeometryShader() noexcept { return _geometryShader->GetRShader(); }

	ENGINE_API static void SetAmbientColor(glm::vec3 &color, float intensity) noexcept;
	ENGINE_API static void SetFogColor(glm::vec3 &color) noexcept;
	ENGINE_API static void SetFogProperties(float clear, float start) noexcept;

	ENGINE_API static void BindGeometry() noexcept;
	ENGINE_API static void BindLighting() noexcept;
	ENGINE_API static void Unbind() noexcept;

	ENGINE_API static void RenderLighting() noexcept;

	ENGINE_API static void ScreenResized(int width, int height) noexcept;

	ENGINE_API static void CopyLight(RFramebuffer* destFbo) noexcept;
	ENGINE_API static void CopyColor(RFramebuffer* destFbo) noexcept;
	ENGINE_API static void CopyBrightness(RFramebuffer* destFbo) noexcept;
	ENGINE_API static void CopyDepth(RFramebuffer* destFbo) noexcept;
	ENGINE_API static void CopyStencil(RFramebuffer* destFbo) noexcept;

	ENGINE_API static void Release() noexcept;
	
private:
	static RFramebuffer* _fbos[4];
	static RTexture* _gbTextures[8];
	static uint64_t _gbTexHandles[7];
	static uint32_t _fboWidth, _fboHeight;
	static Shader *_geometryShader, *_lightingShader;
	static SSAO *_ssao;
	static int _samples;
	static Object *_lightSphere;
	static ShadowMap *_shadow;
	static RBuffer *_sceneLightUbo, *_lightUbo, *_lightMatrixUbo;

	static void _Bind() noexcept;

	static bool _GenerateTextures() noexcept;
	static bool _AttachTextures() noexcept;
	static void _DeleteTextures() noexcept;
	
	DeferredBuffer() { }
};

