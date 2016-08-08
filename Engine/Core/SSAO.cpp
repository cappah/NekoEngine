/* NekoEngine
 *
 * SSAO.cpp
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

#include <stdlib.h>

#include <glm/gtc/type_ptr.hpp>

#include <Engine/SSAO.h>
#include <Engine/DeferredBuffer.h>
#include <Engine/SceneManager.h>
#include <Engine/ResourceManager.h>
#include <Engine/CameraManager.h>
#include <Platform/Compat.h>

using namespace std;
using namespace glm;

#define SSAO_MODULE	"SSAO"

SSAO::SSAO(int width, int height) :
	_fbos{ 0, 0 },
	_textures{ 0, 0 ,0 },
	_fboWidth(width), _fboHeight(height),
	_ssaoShader(0), _ssaoBlurShader(0),
	_noiseSize(SSAO_MAX_NOISE),
	_blurRadius(SSAO_BLUR_RADIUS_4),
	_matrixUbo(nullptr), _dataUbo(nullptr), _blurUbo(nullptr)
{
	_dataBlock.KernelSize = SSAO_HIGH_SAMPLES;
	_dataBlock.Radius = SSAO_HIGH_RADIUS;

	if (Engine::GetConfiguration().Renderer.SSAOQuality == RENDER_SSAO_LOW)
	{
		_dataBlock.KernelSize = SSAO_LOW_SAMPLES;
		_dataBlock.Radius = SSAO_LOW_RADIUS;
	}
	else if (Engine::GetConfiguration().Renderer.SSAOQuality == RENDER_SSAO_MEDIUM)
	{
		_dataBlock.KernelSize = SSAO_MED_SAMPLES;
		_dataBlock.Radius = SSAO_MED_RADIUS;
	}

	NE_SRANDOM((unsigned int)time(NULL));

	// Generate kernel
	for (int i = 0; i < (int)_dataBlock.KernelSize; ++i)
	{
		float scale = (float)i / _dataBlock.KernelSize;

		_dataBlock.Kernel[i].x = 2.f * (float)NE_RANDOM() / RAND_MAX - 1.f;
		_dataBlock.Kernel[i].y = 2.f * (float)NE_RANDOM() / RAND_MAX - 1.f;
		_dataBlock.Kernel[i].z = (float)NE_RANDOM() / RAND_MAX;

		_dataBlock.Kernel[i] = normalize(_dataBlock.Kernel[i]);
		_dataBlock.Kernel[i] *= (float)NE_RANDOM() / RAND_MAX;

		_dataBlock.Kernel[i] *= .1f + 1.f * ((scale * scale) - .1f);
	}

	// Generate noise
	for (int i = 0; i < _noiseSize; i++)
	{
		_noise[i] = glm::vec3(2.f * (float)NE_RANDOM() / RAND_MAX - 1.f,
			2.f * (float)NE_RANDOM() / RAND_MAX - 1.f,
			0.f);
	}

	_ssaoShader = (Shader*)ResourceManager::GetResourceByName("sh_ssao", ResourceType::RES_SHADER);
	_ssaoBlurShader = (Shader*)ResourceManager::GetResourceByName("sh_ssao_blur", ResourceType::RES_SHADER);

	if (_ssaoShader == nullptr || _ssaoBlurShader == nullptr)
	{ DIE("Failed to load SSAO shaders"); }

	if((_fbos[SSAO_FBO_0] = Engine::GetRenderer()->CreateFramebuffer(_fboWidth, _fboHeight)) == nullptr)
	{ DIE("Out of resources"); }
	if((_fbos[SSAO_FBO_1] = Engine::GetRenderer()->CreateFramebuffer(_fboWidth, _fboHeight)) == nullptr)
	{ DIE("Out of resources"); }
	
	_textures[SSAO_TEX_NOISE] = Engine::GetRenderer()->CreateTexture(TextureType::Tex2D);
	_textures[SSAO_TEX_NOISE]->SetStorage2D(1, TextureSizedFormat::RGB_16F, _noiseSize / 4, _noiseSize / 4);
	_textures[SSAO_TEX_NOISE]->SetImage2D(0, _noiseSize / 4, _noiseSize / 4, TextureFormat::RGB, TextureInternalType::Float, _noise);
	_textures[SSAO_TEX_NOISE]->SetMinFilter(TextureFilter::Nearest);
	_textures[SSAO_TEX_NOISE]->SetMagFilter(TextureFilter::Nearest);
	_textures[SSAO_TEX_NOISE]->SetWrapS(TextureWrap::Repeat);
	_textures[SSAO_TEX_NOISE]->SetWrapT(TextureWrap::Repeat);

	if(!_GenerateTextures())
	{ DIE("Out of resources"); }
	
	if(!_AttachTextures())
	{ DIE("Incomplete SSAO FBO"); }

	_dataBlock.FrameAndNoise.x = (float)_fboWidth;
	_dataBlock.FrameAndNoise.y = (float)_fboHeight;
	_dataBlock.FrameAndNoise.z = (float)_fboWidth / (_noiseSize / 4);
	_dataBlock.FrameAndNoise.w = (float)_fboHeight / (_noiseSize / 4);

	if((_matrixUbo = Engine::GetRenderer()->CreateBuffer(BufferType::Uniform, true, false)) == nullptr)
	{ DIE("Out of resources"); }
	_matrixUbo->SetStorage(sizeof(SSAOMatrixBlock), nullptr);

	if((_dataUbo = Engine::GetRenderer()->CreateBuffer(BufferType::Uniform, true, false)) == nullptr)
	{ DIE("Out of resources"); }
	_dataUbo->SetStorage(sizeof(SSAODataBlock), &_dataBlock);

	if((_blurUbo = Engine::GetRenderer()->CreateBuffer(BufferType::Uniform, true, false)) == nullptr)
	{ DIE("Out of resources"); }
	_blurUbo->SetStorage(sizeof(int), &_blurRadius);

	RShader *shader = _ssaoShader->GetRShader();
	RShader *blurShader = _ssaoBlurShader->GetRShader();

	shader->FSUniformBlockBinding(0, "SSAOMatrixBlock");
	shader->FSUniformBlockBinding(1, "SSAODataBlock");
	shader->FSSetUniformBuffer(0, 0, sizeof(SSAOMatrixBlock), _matrixUbo);
	shader->FSSetUniformBuffer(1, 0, sizeof(SSAODataBlock), _dataUbo);

	blurShader->FSUniformBlockBinding(0, "SSAOBlurDataBlock");
	blurShader->FSSetUniformBuffer(0, 0, sizeof(int), _blurUbo);
}

void SSAO::Render() noexcept
{
	Renderer* r = Engine::GetRenderer();
	RShader* shader = _ssaoShader->GetRShader();
	RShader* blurShader = _ssaoBlurShader->GetRShader();
	RFramebuffer* fbo = r->GetBoundFramebuffer();

	_fbos[SSAO_FBO_0]->Bind(FB_DRAW);

	Engine::BindQuadVAO();

	// Pass 1: SSAO
	shader->Enable();
	shader->BindUniformBuffers();

	shader->SetTexture(U_TEXTURE0, DeferredBuffer::GetPositionTexture());
	shader->SetTexture(U_TEXTURE1, DeferredBuffer::GetNormalTexture());
	shader->SetTexture(U_TEXTURE2, _textures[SSAO_TEX_NOISE]);

	_matrixBlock.Projection = CameraManager::GetActiveCamera()->GetProjectionMatrix();
	_matrixBlock.View = CameraManager::GetActiveCamera()->GetView();
	_matrixBlock.InverseView = inverse(_matrixBlock.View);
	_matrixUbo->UpdateData(0, sizeof(SSAOMatrixBlock), &_matrixBlock);

	r->Clear(R_CLEAR_COLOR);
	r->DrawArrays(PolygonMode::TriangleStrip, 0, 4);

	_fbos[SSAO_FBO_1]->Bind(FB_DRAW);

	// Pass 2: Blur
	blurShader->Enable();
	blurShader->BindUniformBuffers();

	blurShader->SetTexture(U_TEXTURE0, _textures[SSAO_TEX_COLOR]);

	r->Clear(R_CLEAR_COLOR);
	r->DrawArrays(PolygonMode::TriangleStrip, 0, 4);

	fbo->Bind(FB_DRAW);
}

void SSAO::Resize(int width, int height) noexcept
{
	_fboWidth = width;
	_fboHeight = height;

	_fbos[SSAO_FBO_0]->Resize(_fboWidth, _fboHeight);
	_fbos[SSAO_FBO_1]->Resize(_fboWidth, _fboHeight);	

	_dataBlock.FrameAndNoise.x = (float)_fboWidth;
	_dataBlock.FrameAndNoise.y = (float)_fboHeight;
	_dataBlock.FrameAndNoise.z = (float)_fboWidth / (_noiseSize / 4);
	_dataBlock.FrameAndNoise.w = (float)_fboHeight / (_noiseSize / 4);
	_dataUbo->UpdateData(0, sizeof(vec4), &_dataBlock.FrameAndNoise);
}

bool SSAO::_GenerateTextures()
{
	if((_textures[SSAO_TEX_COLOR] = Engine::GetRenderer()->CreateTexture(TextureType::Tex2D)) == nullptr)
		return false;
	_textures[SSAO_TEX_COLOR]->SetStorage2D(1, TextureSizedFormat::R_8U, _fboWidth, _fboHeight);
	_textures[SSAO_TEX_COLOR]->SetMinFilter(TextureFilter::Linear);
	_textures[SSAO_TEX_COLOR]->SetMagFilter(TextureFilter::Linear);

	if((_textures[SSAO_TEX_BLUR] = Engine::GetRenderer()->CreateTexture(TextureType::Tex2D)) == nullptr)
		return false;
	_textures[SSAO_TEX_BLUR]->SetStorage2D(1, TextureSizedFormat::R_8U, _fboWidth, _fboHeight);
	_textures[SSAO_TEX_BLUR]->SetMinFilter(TextureFilter::Linear);
	_textures[SSAO_TEX_BLUR]->SetMagFilter(TextureFilter::Linear);
	
	return true;
}

bool SSAO::_AttachTextures()
{
	_fbos[SSAO_FBO_0]->AttachTexture(DrawAttachment::Color0, _textures[SSAO_TEX_COLOR]);
	if (_fbos[SSAO_FBO_0]->CheckStatus() != FramebufferStatus::Complete)
	{
		Logger::Log(SSAO_MODULE, LOG_CRITICAL, "First framebuffer incomplete");
		return false;
	}

	_fbos[SSAO_FBO_1]->AttachTexture(DrawAttachment::Color0, _textures[SSAO_TEX_BLUR]);
	if (_fbos[SSAO_FBO_1]->CheckStatus() != FramebufferStatus::Complete)
	{
		Logger::Log(SSAO_MODULE, LOG_CRITICAL, "Second framebuffer incomplete");
		return false;
	}

	return true;
}

void SSAO::_DeleteTextures(bool noise)
{
	delete _textures[SSAO_TEX_COLOR];
	delete _textures[SSAO_TEX_BLUR];

	if (noise)
		delete _textures[SSAO_TEX_NOISE];
}

SSAO::~SSAO()
{
	_DeleteTextures(true);
	
	ResourceManager::UnloadResourceByName("sh_ssao", ResourceType::RES_SHADER);
	ResourceManager::UnloadResourceByName("sh_ssao_blur", ResourceType::RES_SHADER);
	
	delete _fbos[SSAO_FBO_0];
	delete _fbos[SSAO_FBO_1];
	delete _matrixUbo;
	delete _dataUbo;
	delete _blurUbo;
}
