/* NekoEngine
 *
 * PostProcessor.h
 * Author: Alexandru Naiman
 *
 * PostProcessing effects
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

#include <Engine/Engine.h>
#include <Engine/Shader.h>
#include <PostEffects/Effect.h>
#include <PostEffects/SMAA.h>

#define PP_TEX_FBO0					0
#define PP_TEX_FBO1					1
#define PP_TEX_COLOR				2
#define PP_TEX_BRIGHT				3
#define PP_TEX_DEPTH_STENCIL		4

#define FBO_0						0
#define FBO_1						1
#define FBO_DRAW					2
#define FBO_BRIGHT					3
#define FBO_COLOR					4

class PostProcessor
{
public:
	ENGINE_API static int Initialize();

	ENGINE_API static void AddEffect(Effect* e) noexcept { _effects.push_back(e); };

	ENGINE_API static RFramebuffer* GetBuffer() noexcept { return _fbos[FBO_DRAW]; }
	ENGINE_API static RFramebuffer* GetColorBuffer() noexcept { return _fbos[FBO_COLOR]; }
	ENGINE_API static RFramebuffer* GetBrightnessBuffer() noexcept { return _fbos[FBO_BRIGHT]; }
	
	ENGINE_API static void ApplyEffects() noexcept;

	ENGINE_API static void DrawEffect(RShader *shader, bool writeColor) noexcept;

	ENGINE_API static void ScreenResized() noexcept;

	ENGINE_API static void Release() noexcept;
	
private:
	static RFramebuffer* _fbos[5];
	static RTexture* _ppTextures[5];
	static Shader *_shader;
	static uint32_t _fboWidth, _fboHeight;
	static TextureSizedFormat _textureFormat;
	static std::vector<Effect *> _effects;
	static bool _secondFb;
	static RBuffer *_ppUbo;

	static bool _GenerateTextures() noexcept;
	static bool _AttachTextures() noexcept;
	static void _DeleteTextures() noexcept;
	
	PostProcessor() { }
};

