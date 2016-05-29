/* Neko Engine
 *
 * LoadingScreen.cpp
 * Author: Alexandru Naiman
 *
 * LoadingScreen implementation
 *
 * ----------------------------------------------------------------------------------
 *
 * Copyright (c) 2015-2016, Alexandru Naiman <alexandru dot naiman at icloud dot com>
 *
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without modification,
 * are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice, this list
 * of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above copyright notice, this
 * list of conditions and the following disclaimer in the documentation and/or other
 * materials provided with the distribution.
 *
 * 3. Neither the name of the copyright holder nor the names of its contributors may be
 * used to endorse or promote products derived from this software without specific prior
 * written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY ALEXANDRU NAIMAN "AS IS" AND ANY EXPRESS OR IMPLIED
 * WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY
 * AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL ALEXANDRU NAIMAN
 * BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE
 * OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */

#define ENGINE_INTERNAL

#include <string>

#include <Engine/Engine.h>
#include <Engine/LoadingScreen.h>
#include <Engine/ResourceManager.h>

#define LS_MODULE	"LoadingScreen"

using namespace std;

LoadingScreen::LoadingScreen(const char* texture)
{
	TextureParams texParams { TextureFilter::Linear, TextureFilter::Linear, TextureWrap::ClampToEdge, TextureWrap::ClampToEdge };

	if((_shader = (Shader*)ResourceManager::GetResourceByName("sh_loadscreen", ResourceType::RES_SHADER)) == nullptr)
	{ DIE("Cannot to load loading screen shader"); }
	
	float frame[2] = { (float)Engine::GetConfiguration().Engine.ScreenWidth, (float)Engine::GetConfiguration().Engine.ScreenHeight };

	if((_uniformBuffer = Engine::GetRenderer()->CreateBuffer(BufferType::Uniform, false, true)) == nullptr)
	{ DIE("Out of resources"); }
	_uniformBuffer->SetNumBuffers(1);
	_uniformBuffer->SetStorage(sizeof(float) * 2, frame);

	_shader->GetRShader()->FSUniformBlockBinding(0, "shader_data");
	_shader->GetRShader()->FSSetUniformBuffer(0, 0, sizeof(float) * 2, _uniformBuffer);

	// Must force high quality texture for loading screen
	int texQuality = Engine::GetConfiguration().Renderer.TextureQuality;
	Engine::GetConfiguration().Renderer.TextureQuality = RENDER_TEX_Q_HIGH;

	if(!texture)
		_texture = (Texture*)ResourceManager::GetResourceByName("tex_loadscreen", ResourceType::RES_TEXTURE);
	else
		_texture = (Texture*)ResourceManager::GetResourceByName(texture, ResourceType::RES_TEXTURE);

	Engine::GetConfiguration().Renderer.TextureQuality = texQuality;

	if (!_texture)
	{
		Logger::Log(LS_MODULE, LOG_CRITICAL, "Cannot load texture");
		DIE("Cannot load loading screen texture")
	}

	_texture->SetParameters(texParams);

	_shader->GetRShader()->SetTexture(U_TEXTURE0, _texture->GetRTexture());

	Draw();
}

void LoadingScreen::Draw() noexcept
{
	_shader->Enable();
	_shader->GetRShader()->BindUniformBuffers();

	Engine::BindQuadVAO();
	Engine::GetRenderer()->DrawArrays(PolygonMode::TriangleStrip, 0, 4);
	Engine::SwapBuffers();
}

LoadingScreen::~LoadingScreen() noexcept
{
	delete _uniformBuffer;

	if(_shader)
		ResourceManager::UnloadResource(_shader->GetResourceInfo()->id, ResourceType::RES_SHADER);

	if (_texture)
		ResourceManager::UnloadResource(_texture->GetResourceInfo()->id, ResourceType::RES_TEXTURE);
}
