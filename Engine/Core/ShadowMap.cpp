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
#include <Engine/DeferredBuffer.h>
#include <Engine/ResourceManager.h>
#include <Scene/Light.h>

using namespace glm;

ShadowMap::ShadowMap(int size) :
	_fboWidth(size),
	_fboHeight(size),
	_size(size),
	_fbo(nullptr),
	_colorFbo(nullptr),
	_uniformBuffer(nullptr),
	_texture(nullptr),
	_shader(nullptr),
	_shadowCamera(nullptr)
{
	_texture = Engine::GetRenderer()->CreateTexture(TextureType::Tex2D);
	_texture->SetStorage2D(1, TextureSizedFormat::DEPTH_32F, size, size);
	_texture->SetMinFilter(TextureFilter::Nearest);
	_texture->SetMagFilter(TextureFilter::Nearest);
	_texture->SetWrapS(TextureWrap::Repeat);
	_texture->SetWrapT(TextureWrap::Repeat);

	_fbo = Engine::GetRenderer()->CreateFramebuffer(size, size);
	_fbo->AttachDepthTexture(_texture);
	_fbo->SetDrawBuffer(DrawAttachment::None);
	_fbo->SetReadBuffer(DrawAttachment::None);
	if(_fbo->CheckStatus() != FramebufferStatus::Complete)
	{ DIE("Failed to create shadow framebuffer"); }

	_uniformBuffer = Engine::GetRenderer()->CreateBuffer(BufferType::Uniform, true, false);
	_uniformBuffer->SetNumBuffers(0);
	_uniformBuffer->SetStorage(sizeof(ShadowMapMatrixBlock), &_matrixBlock);

	if((_shader = (Shader*)ResourceManager::GetResourceByName("sh_shadow", ResourceType::RES_SHADER)) == nullptr)
	{ DIE("Failed to load shadow map shader"); }

	/*_shader->GetRShader()->VSUniformBlockBinding(0, "MatrixBlock");
	_shader->GetRShader()->VSSetUniformBuffer(0, 0, sizeof(ShadowMapMatrixBlock), _uniformBuffer);*/

	/*lightShader->FSUniformBlockBinding(0, "SceneLightData");
	lightShader->FSSetUniformBuffer(0, 0, sizeof(LightSceneData), _sceneLightUbo);*/

	_shadowCamera = new Camera();

	_shadowCamera->SetNear(.1f);
	_shadowCamera->SetFar(10000.f);
	_shadowCamera->SetProjection(ProjectionType::Ortographic);
	_shadowCamera->UpdateProjection();

	_size = size;
}
#include <Engine/CameraManager.h>
void ShadowMap::Render()
{
	Scene *s = SceneManager::GetActiveScene();

	_fbo->Bind(FB_DRAW);

	Engine::GetRenderer()->SetDepthMask(true);
	Engine::GetRenderer()->Clear(R_CLEAR_DEPTH);

	Engine::BindQuadVAO();

	for (size_t i = 0; i < s->GetNumLights(); i++)
	{
		Light *l = s->GetLight(i);
		
		if (!l->CastShadows())
			continue;

		if (l->GetType() == LightType::Directional || l->GetType() == LightType::Spot)
		{
			_shadowCamera->LookAt(l->GetDirection(), vec3(0.f), vec3(0.f, 1.f, 0.f));
			SceneManager::DrawScene(DeferredBuffer::GetGeometryShader(), _shadowCamera);
		}
		else if (l->GetType() == LightType::Point)
		{
			//
		}

		//

		//Engine::GetRenderer()->DrawArrays(PolygonMode::TriangleStrip, 0, 4);
	}

	_fbo->Unbind();
	Engine::GetRenderer()->SetDepthMask(false);

//	_colorFbo->Bind(FB_DRAW);

	// bind world to camera

	/*Engine::GetRenderer()->Clear(R_CLEAR_DEPTH);
	Engine::GetRenderer()->DrawArrays(PolygonMode::TriangleStrip, 0, 4);*/
}

ShadowMap::~ShadowMap()
{
	delete _texture;
	delete _fbo;
	delete _colorFbo;
	delete _uniformBuffer;

	ResourceManager::UnloadResourceByName("sh_shadow", ResourceType::RES_SHADER);
}
