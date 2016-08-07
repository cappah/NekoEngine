/* NekoEngine
 *
 * Skybox.cpp
 * Author: Alexandru Naiman
 *
 * Engine skybox implementation 
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

#include <glm/glm.hpp>

#include <Engine/Engine.h>
#include <Engine/SceneManager.h>
#include <Engine/CameraManager.h>
#include <Engine/ResourceManager.h>
#include <Scene/Skybox.h>

using namespace glm;

ENGINE_REGISTER_OBJECT_CLASS(Skybox)

int Skybox::Load()
{
	int ret = Object::Load();
	
	if (ret != ENGINE_OK)
		return ret;

	if (_shaderId == -1)
		_skyboxShader = (Shader*)ResourceManager::GetResourceByName("sh_fwd_skybox", ResourceType::RES_SHADER);
	else
		_skyboxShader = (Shader*)ResourceManager::GetResource(_shaderId, ResourceType::RES_SHADER);

	if (!_skyboxShader)
		return ENGINE_FAIL;

	_shaderId = _skyboxShader->GetResourceInfo()->id;
	
	_skyboxShader->GetRShader()->VSUniformBlockBinding(0, "MatrixBlock");
	_skyboxShader->GetRShader()->FSUniformBlockBinding(0, "ObjectBlock");
	_skyboxShader->GetRShader()->FSUniformBlockBinding(1, "MaterialBlock");

	return ENGINE_OK;
}

void Skybox::Draw(RShader* shader) noexcept
{
	_skyboxShader->GetRShader()->Enable();

	Object::Draw(_skyboxShader->GetRShader());

	_skyboxShader->GetRShader()->Disable();
}

void Skybox::Update(double deltaTime) noexcept
{
	Object::Update(deltaTime);

	vec3 pos = CameraManager::GetActiveCamera()->GetPosition();
	pos.y = _position.y;
	SetPosition(pos);
}

Skybox::~Skybox() noexcept
{
	ResourceManager::UnloadResource(_shaderId, ResourceType::RES_SHADER);
	_skyboxShader = nullptr;
}
