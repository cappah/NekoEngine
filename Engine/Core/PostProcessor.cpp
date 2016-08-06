/* Neko Engine
 *
 * PostProcessor.cpp
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

#define ENGINE_INTERNAL

#include <Engine/Engine.h>
#include <Engine/DeferredBuffer.h>
#include <Engine/PostProcessor.h>
#include <Engine/ResourceManager.h>
#include <Engine/SceneManager.h>

#define PP_MODULE	"PostProcessor"

RFramebuffer* PostProcessor::_fbos[5]{0, 0, 0, 0, 0};
RTexture* PostProcessor::_ppTextures[5]{0, 0, 0, 0, 0};
Shader* PostProcessor::_shader = nullptr;
uint32_t PostProcessor::_fboWidth = 1280;
uint32_t PostProcessor::_fboHeight = 720;
TextureSizedFormat PostProcessor::_textureFormat = TextureSizedFormat::RGBA_16F;
std::vector<Effect *> PostProcessor::_effects;
bool PostProcessor::_secondFb = true;
RBuffer* PostProcessor::_ppUbo = nullptr;

bool PostProcessor::_GenerateTextures() noexcept
{	
	if((_ppTextures[PP_TEX_FBO0] = Engine::GetRenderer()->CreateTexture(TextureType::Tex2D)) == nullptr)
		return false;
	_ppTextures[PP_TEX_FBO0]->SetStorage2D(1, _textureFormat, _fboWidth, _fboHeight);
	_ppTextures[PP_TEX_FBO0]->SetMinFilter(TextureFilter::Linear);
	_ppTextures[PP_TEX_FBO0]->SetMagFilter(TextureFilter::Linear);
	_ppTextures[PP_TEX_FBO0]->SetWrapS(TextureWrap::ClampToEdge);
	_ppTextures[PP_TEX_FBO0]->SetWrapT(TextureWrap::ClampToEdge);

	if((_ppTextures[PP_TEX_FBO1] = Engine::GetRenderer()->CreateTexture(TextureType::Tex2D)) == nullptr)
		return false;
	_ppTextures[PP_TEX_FBO1]->SetStorage2D(1, _textureFormat, _fboWidth, _fboHeight);
	_ppTextures[PP_TEX_FBO1]->SetMinFilter(TextureFilter::Linear);
	_ppTextures[PP_TEX_FBO1]->SetMagFilter(TextureFilter::Linear);
	_ppTextures[PP_TEX_FBO1]->SetWrapS(TextureWrap::ClampToEdge);
	_ppTextures[PP_TEX_FBO1]->SetWrapT(TextureWrap::ClampToEdge);

	if((_ppTextures[PP_TEX_COLOR] = Engine::GetRenderer()->CreateTexture(TextureType::Tex2D)) == nullptr)
		return false;
	_ppTextures[PP_TEX_COLOR]->SetStorage2D(1, _textureFormat, _fboWidth, _fboHeight);
	_ppTextures[PP_TEX_COLOR]->SetMinFilter(TextureFilter::Linear);
	_ppTextures[PP_TEX_COLOR]->SetMagFilter(TextureFilter::Linear);
	_ppTextures[PP_TEX_COLOR]->SetWrapS(TextureWrap::ClampToEdge);
	_ppTextures[PP_TEX_COLOR]->SetWrapT(TextureWrap::ClampToEdge);

	if((_ppTextures[PP_TEX_BRIGHT] = Engine::GetRenderer()->CreateTexture(TextureType::Tex2D)) == nullptr)
		return false;
	_ppTextures[PP_TEX_BRIGHT]->SetStorage2D(1, TextureSizedFormat::RGB_8U, _fboWidth, _fboHeight);
	_ppTextures[PP_TEX_BRIGHT]->SetMinFilter(TextureFilter::Linear);
	_ppTextures[PP_TEX_BRIGHT]->SetMagFilter(TextureFilter::Linear);
	_ppTextures[PP_TEX_BRIGHT]->SetWrapS(TextureWrap::ClampToEdge);
	_ppTextures[PP_TEX_BRIGHT]->SetWrapT(TextureWrap::ClampToEdge);

	if((_ppTextures[PP_TEX_DEPTH_STENCIL] = Engine::GetRenderer()->CreateTexture(TextureType::Tex2D)) == nullptr)
		return false;
	_ppTextures[PP_TEX_DEPTH_STENCIL]->SetStorage2D(1, TextureSizedFormat::DEPTH24_STENCIL8, _fboWidth, _fboHeight);
	_ppTextures[PP_TEX_DEPTH_STENCIL]->SetMinFilter(TextureFilter::Nearest);
	_ppTextures[PP_TEX_DEPTH_STENCIL]->SetMagFilter(TextureFilter::Nearest);
	
	return true;
}

bool PostProcessor::_AttachTextures() noexcept
{
	assert(_ppTextures[PP_TEX_FBO0]);
	assert(_ppTextures[PP_TEX_FBO1]);
	assert(_ppTextures[PP_TEX_COLOR]);
	assert(_ppTextures[PP_TEX_BRIGHT]);
	assert(_ppTextures[PP_TEX_DEPTH_STENCIL]);

	_fbos[FBO_DRAW]->AttachTexture(DrawAttachment::Color0, _ppTextures[PP_TEX_FBO0]);
	_fbos[FBO_DRAW]->AttachDepthStencilTexture(_ppTextures[PP_TEX_DEPTH_STENCIL]);
	
	if (_fbos[FBO_DRAW]->CheckStatus() != FramebufferStatus::Complete)
	{
		Logger::Log(PP_MODULE, LOG_CRITICAL, "Draw framebuffer incomplete");
		return false;
	}

	_fbos[FBO_0]->AttachTexture(DrawAttachment::Color0, _ppTextures[PP_TEX_FBO0]);
	_fbos[FBO_0]->AttachTexture(DrawAttachment::Color1, _ppTextures[PP_TEX_COLOR]);

	if (_fbos[FBO_0]->CheckStatus() != FramebufferStatus::Complete)
	{
		Logger::Log(PP_MODULE, LOG_CRITICAL, "First framebuffer incomplete");
		return false;
	}

	_fbos[FBO_1]->AttachTexture(DrawAttachment::Color0, _ppTextures[PP_TEX_FBO1]);
	_fbos[FBO_1]->AttachTexture(DrawAttachment::Color1, _ppTextures[PP_TEX_COLOR]);

	if (_fbos[FBO_1]->CheckStatus() != FramebufferStatus::Complete)
	{
		Logger::Log(PP_MODULE, LOG_CRITICAL, "Second framebuffer incomplete");
		return false;
	}

	_fbos[FBO_BRIGHT]->AttachTexture(DrawAttachment::Color0, _ppTextures[PP_TEX_BRIGHT]);

	if (_fbos[FBO_BRIGHT]->CheckStatus() != FramebufferStatus::Complete)
	{
		Logger::Log(PP_MODULE, LOG_CRITICAL, "Brightness framebuffer incomplete");
		return false;
	}

	_fbos[FBO_COLOR]->AttachTexture(DrawAttachment::Color0, _ppTextures[PP_TEX_COLOR]);

	if (_fbos[FBO_COLOR]->CheckStatus() != FramebufferStatus::Complete)
	{
		Logger::Log(PP_MODULE, LOG_CRITICAL, "Color framebuffer incomplete");
		return false;
	}

	return true;
}

int PostProcessor::Initialize()
{
	Logger::Log(PP_MODULE, LOG_INFORMATION, "Loading...");

	_fboWidth = DeferredBuffer::GetWidth();
	_fboHeight = DeferredBuffer::GetHeight();

	_shader = (Shader*)ResourceManager::GetResourceByName("sh_pp_quad", ResourceType::RES_SHADER);
	if (!_shader)
		return ENGINE_FAIL;

	if((_fbos[FBO_0] = Engine::GetRenderer()->CreateFramebuffer(_fboWidth, _fboHeight)) == nullptr)
	{
		Release();
		return ENGINE_OUT_OF_RESOURCES;
	}
	
	if((_fbos[FBO_1] = Engine::GetRenderer()->CreateFramebuffer(_fboWidth, _fboHeight)) == nullptr)
	{
		Release();
		return ENGINE_OUT_OF_RESOURCES;
	}
	
	if((_fbos[FBO_DRAW] = Engine::GetRenderer()->CreateFramebuffer(_fboWidth, _fboHeight)) == nullptr)
	{
		Release();
		return ENGINE_OUT_OF_RESOURCES;
	}
	
	if((_fbos[FBO_BRIGHT] = Engine::GetRenderer()->CreateFramebuffer(_fboWidth, _fboHeight)) == nullptr)
	{
		Release();
		return ENGINE_OUT_OF_RESOURCES;
	}
	
	if((_fbos[FBO_COLOR] = Engine::GetRenderer()->CreateFramebuffer(_fboWidth, _fboHeight)) == nullptr)
	{
		Release();
		return ENGINE_OUT_OF_RESOURCES;
	}

	if (!_GenerateTextures())
	{
		Release();
		return ENGINE_OUT_OF_RESOURCES;
	}
	
	if (!_AttachTextures())
	{
		Release();
		return ENGINE_FAIL;
	}
	
	Logger::Log(PP_MODULE, LOG_INFORMATION, "Loading effects...");

	float uboData[4] { (float)_fboWidth, (float)_fboHeight, 0, 0 };

	if((_ppUbo = Engine::GetRenderer()->CreateBuffer(BufferType::Uniform, true, false)) == nullptr)
	{
		Release();
		return ENGINE_OUT_OF_RESOURCES;
	}
	_ppUbo->SetStorage(sizeof(uboData), uboData);

	_shader->GetRShader()->FSUniformBlockBinding(0, "PPSharedData");
	_shader->GetRShader()->FSSetUniformBuffer(0, 0, sizeof(uboData), _ppUbo);
	
	for (Effect *e : _effects)
	{
		if (e->Load(_ppUbo) != ENGINE_OK)
		{
			Logger::Log(PP_MODULE, LOG_CRITICAL, "Failed to load effect %s", e->GetName());
			return ENGINE_FAIL;
		}
	}

	Logger::Log(PP_MODULE, LOG_INFORMATION, "Initialized");

	return ENGINE_OK;
}

void PostProcessor::ApplyEffects() noexcept
{
	Renderer *r = Engine::GetRenderer();
	Engine::BindQuadVAO();

	for (size_t i = 0; i < _effects.size(); ++i)
		_effects[i]->Apply();
	
	RTexture* texture = _secondFb ? _ppTextures[PP_TEX_FBO0] : _ppTextures[PP_TEX_FBO1];

	_shader->Enable();
	_shader->GetRShader()->SetTexture(U_TEXTURE0, texture);
	_shader->GetRShader()->BindUniformBuffers();
	
	r->BindDefaultFramebuffer();
	r->Clear(R_CLEAR_COLOR);
	r->DrawArrays(PolygonMode::TriangleStrip, 0, 4);

	_shader->Disable();
}

void PostProcessor::DrawEffect(RShader *shader, bool writeColor) noexcept
{
	Renderer *r = Engine::GetRenderer();
	DrawAttachment attachments[2]{ DrawAttachment::Color0, DrawAttachment::Color1 };

	shader->Enable();
	shader->BindUniformBuffers();
	shader->SetTexture(U_TEXTURE0, _ppTextures[_secondFb ? PP_TEX_FBO0 : PP_TEX_FBO1]);
	shader->SetTexture(U_TEXTURE1, _ppTextures[PP_TEX_COLOR]);
	shader->SetTexture(U_TEXTURE2, _ppTextures[PP_TEX_BRIGHT]);
	shader->SetTexture(U_TEXTURE3, _ppTextures[PP_TEX_DEPTH_STENCIL]);

	_fbos[_secondFb ? FBO_1 : FBO_0]->Bind(FB_DRAW);
	_fbos[_secondFb ? FBO_1 : FBO_0]->SetDrawBuffers(writeColor ? 2 : 1, attachments);

	r->DrawArrays(PolygonMode::TriangleStrip, 0, 4);

	shader->Disable();

	_secondFb = !_secondFb;
}

void PostProcessor::ScreenResized() noexcept
{
	_fboWidth = DeferredBuffer::GetWidth();
	_fboHeight = DeferredBuffer::GetHeight();

	_fbos[FBO_0]->Resize(_fboWidth, _fboHeight);
	_fbos[FBO_1]->Resize(_fboWidth, _fboHeight);
	_fbos[FBO_DRAW]->Resize(_fboWidth, _fboHeight);
	_fbos[FBO_BRIGHT]->Resize(_fboWidth, _fboHeight);
	_fbos[FBO_COLOR]->Resize(_fboWidth, _fboHeight);
}

void PostProcessor::_DeleteTextures() noexcept
{
	delete _ppTextures[PP_TEX_FBO0];
	delete _ppTextures[PP_TEX_FBO1];
	delete _ppTextures[PP_TEX_COLOR];
	delete _ppTextures[PP_TEX_BRIGHT];
	delete _ppTextures[PP_TEX_DEPTH_STENCIL];
}

void PostProcessor::Release() noexcept
{
	_DeleteTextures();

	delete _fbos[FBO_0]; _fbos[FBO_0] = nullptr;
	delete _fbos[FBO_1]; _fbos[FBO_1] = nullptr;
	delete _fbos[FBO_DRAW]; _fbos[FBO_DRAW] = nullptr;
	delete _fbos[FBO_BRIGHT]; _fbos[FBO_BRIGHT] = nullptr;
	delete _fbos[FBO_COLOR]; _fbos[FBO_COLOR] = nullptr;

	for (Effect *e : _effects)
		delete e;

	ResourceManager::UnloadResourceByName("sh_pp_quad", ResourceType::RES_SHADER);
	_shader = nullptr;

	delete _ppUbo;
	_ppUbo = nullptr;
	
	Logger::Log(PP_MODULE, LOG_INFORMATION, "Released");
}
