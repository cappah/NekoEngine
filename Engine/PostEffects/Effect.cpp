/* Neko Engine
 *
 * Effect.cpp
 * Author: Alexandru Naiman
 *
 * Effect class implementation
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

#include <glm/gtc/type_ptr.hpp>

#include <System/VFS/VFS.h>

#include <Engine/Engine.h>
#include <PostEffects/Effect.h>
#include <Engine/ResourceManager.h>
#include <Engine/PostProcessor.h>

using namespace std;
using namespace glm;

Effect::Effect(const char* name) noexcept :
	_name(name),
	_effectUbo(nullptr)
{
}
	
int Effect::Load(RBuffer *sharedUbo)
{
	// create ubo
	_effectUbo = Engine::GetRenderer()->CreateBuffer(BufferType::Uniform, true, false);
	_effectUbo->SetStorage(sizeof(vec4), value_ptr(_effectData));

	// load shaders
	for (int shaderId : _shaderIds)
	{
		Shader *shader = (Shader *)ResourceManager::GetResource(shaderId, ResourceType::RES_SHADER);

		if (!shader)
			return ENGINE_FAIL;

		shader->GetRShader()->FSUniformBlockBinding(0, "PPSharedData");
		shader->GetRShader()->FSUniformBlockBinding(1, "PPEffectData");

		shader->GetRShader()->FSSetUniformBuffer(0, 0, sizeof(vec4), sharedUbo);
		shader->GetRShader()->FSSetUniformBuffer(1, 0, sizeof(vec4), _effectUbo);

		_shaders.push_back(shader);
	}
	
	return ENGINE_OK;
}

void Effect::SetOption(std::string option, float value)
{
	*_options[option] = value;

	if (_effectUbo)
		_effectUbo->UpdateData(0, sizeof(vec4), value_ptr(_effectData));
}

void Effect::Apply()
{
	for (size_t i = 0; i < _shaders.size(); ++i)
		PostProcessor::DrawEffect(_shaders[i]->GetRShader(), (i == _shaders.size() - 1) ? true : false);
}

Effect::~Effect()
{
	for (int id : _shaderIds)
		ResourceManager::UnloadResource(id, ResourceType::RES_SHADER);
	
	delete _effectUbo;
}
