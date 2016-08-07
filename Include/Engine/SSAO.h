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
#include <Engine/Shader.h>

#include <vector>

#define SSAO_FBO_0				0
#define SSAO_FBO_1				1

#define SSAO_TEX_COLOR				0
#define SSAO_TEX_BLUR				1
#define SSAO_TEX_NOISE				2

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

typedef struct SSAO_MATRIX_BLOCK
{
	glm::mat4 View;
	glm::mat4 InverseView;
	glm::mat4 Projection;
} SSAOMatrixBlock;

typedef struct SSAO_DATA
{
	glm::vec4 FrameAndNoise;
	float KernelSize;
	float Radius;
	float padding[2];
	glm::vec4 Kernel[SSAO_MAX_SAMPLES];
} SSAODataBlock;

class SSAO
{
public:
	SSAO(int width, int height);

	RTexture* GetTexture() noexcept { return _textures[SSAO_TEX_BLUR]; }

	void Render() noexcept;
	void Resize(int width, int height) noexcept;

	~SSAO();

private:
	RFramebuffer* _fbos[2];
	RTexture* _textures[3];
	int _fboWidth, _fboHeight;
	Shader *_ssaoShader, *_ssaoBlurShader;
	int32_t _noiseSize;
	glm::vec3 _noise[SSAO_MAX_NOISE];

	SSAOMatrixBlock _matrixBlock;
	SSAODataBlock _dataBlock;
	int _blurRadius;

	RBuffer *_matrixUbo, *_dataUbo, *_blurUbo;

	bool _GenerateTextures();
	bool _AttachTextures();
	void _DeleteTextures(bool noise);
};

