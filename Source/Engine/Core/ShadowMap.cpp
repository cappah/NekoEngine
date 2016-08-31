/* NekoEngine
 *
 * ShadowMap.h
 * Author: Alexandru Naiman
 *
 * ShadowMap class implementation 
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

#include <glm/glm.hpp>

#include <Engine/ShadowMap.h>
#include <Engine/SceneManager.h>
#include <Engine/CameraManager.h>
#include <Engine/DeferredBuffer.h>
#include <Engine/ResourceManager.h>
#include <Scene/Light.h>

using namespace glm;

ShadowMap::ShadowMap(int size) :
	_fbo(nullptr),
	_occlusionFbo(nullptr),
	_texture(nullptr),
	_occlussionTexture(nullptr),
	_lightCamera(nullptr),
	_shader(nullptr),
	_size(size)
{
	_texture = Engine::GetRenderer()->CreateTexture(TextureType::Tex2D);
	_texture->SetStorage2D(1, TextureSizedFormat::DEPTH_16, size, size);
	_texture->SetMinFilter(TextureFilter::Nearest);
	_texture->SetMagFilter(TextureFilter::Nearest);
	_texture->SetWrapS(TextureWrap::ClampToEdge);
	_texture->SetWrapT(TextureWrap::ClampToEdge);

	_fbo = Engine::GetRenderer()->CreateFramebuffer(size, size);
	_fbo->AttachDepthTexture(_texture);
	_fbo->SetDrawBuffer(DrawAttachment::None);
	_fbo->SetReadBuffer(DrawAttachment::None);
	if(_fbo->CheckStatus() != FramebufferStatus::Complete)
	{ DIE("Failed to create shadow framebuffer"); }

	_lightCamera = new Camera();
	_lightCamera->Ortho(-400, 400, -400, 400, -40, 4000);
	_lightCamera->UpdateProjection();
}

void ShadowMap::Render(Light *l)
{
	Engine::GetRenderer()->SetDepthMask(true);
	_fbo->Bind(FB_DRAW);
	Engine::GetRenderer()->Clear(R_CLEAR_DEPTH);

	if (l->GetType() == LightType::Directional || l->GetType() == LightType::Spot)
	{
		_lightCamera->SetPosition(l->GetPosition());
		_lightCamera->SetRotation(l->GetRotation());
		_lightCamera->UpdateView();
		SceneManager::DrawScene(DeferredBuffer::GetGeometryShader(), _lightCamera);
	}
	else if (l->GetType() == LightType::Point)
	{
		//
	}

	_fbo->Unbind();
	Engine::GetRenderer()->SetDepthMask(false);
}

ShadowMap::~ShadowMap()
{
	delete _texture;
	delete _fbo;
}
