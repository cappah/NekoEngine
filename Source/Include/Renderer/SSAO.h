/* NekoEngine
 *
 * SSAO.h
 * Author: Alexandru Naiman
 *
 * Screen-Space Ambient Occlusion
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
#include <Renderer/Buffer.h>
#include <Renderer/Texture.h>
#include <Renderer/ShaderModule.h>

#define SSAO_FBO_0					0
#define SSAO_FBO_1					1

#define SSAO_TEX_COLOR				0
#define SSAO_TEX_BLUR				1
#define SSAO_TEX_NOISE				2

#define SSAO_MAX_SAMPLES			128

#define SSAO_MAX_NOISE				16

#define SSAO_LOW_SAMPLES			32
#define SSAO_LOW_RADIUS				10.f

#define SSAO_MED_SAMPLES			64
#define SSAO_MED_RADIUS				15.f

#define SSAO_HIGH_SAMPLES			SSAO_MAX_SAMPLES
#define SSAO_HIGH_RADIUS			20.f

#define SSAO_BLUR_RADIUS_2			2
#define SSAO_BLUR_RADIUS_4			4
#define SSAO_BLUR_RADIUS_8			8

class SSAO
{
public:
	static int Initialize();

	static VkImageView GetAOImageView();

	static int BuildCommandBuffer();
	static void Resize(int width, int height) noexcept;
	static void UpdateData(VkCommandBuffer cmdBuffer) noexcept;

	static VkCommandBuffer GetCommandBuffer() noexcept;

	static void Release();

private:
	static void _CreateTextures();
	static int _CreatePipelines();
	static int _CreateDescriptorSets();
	static bool _CreateRenderPass();
	static bool _CreateFramebuffers();

	bool _GenerateTextures();
	bool _AttachTextures();
	void _DeleteTextures(bool noise);
};