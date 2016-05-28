/* Neko Engine
 *
 * DemoAnimatorComponent.cpp
 * Author: Alexandru Naiman
 *
 * DemoAnimatorComponent class implementation 
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

#define TESTGAME_INTERNAL

#include "DemoAnimatorComponent.h"
#include <Engine/ResourceManager.h>

REGISTER_COMPONENT_CLASS(DemoAnimatorComponent);

DemoAnimatorComponent::DemoAnimatorComponent(ComponentInitializer *initializer)
	: AnimatorComponent(initializer)
{
	_clipIds[0] = initializer->arguments.find("clip0")->second;
	_clipIds[1] = initializer->arguments.find("clip1")->second;
	_clipIds[2] = initializer->arguments.find("clip2")->second;
}

int DemoAnimatorComponent::Load()
{
	int ret = AnimatorComponent::Load();

	if (ret != ENGINE_OK)
		return ret;

	_clips[0] = (AnimationClip*)ResourceManager::GetResourceByName(_clipIds[0].c_str(), ResourceType::RES_ANIMCLIP);
	_clips[1] = (AnimationClip*)ResourceManager::GetResourceByName(_clipIds[1].c_str(), ResourceType::RES_ANIMCLIP);
	_clips[2] = (AnimationClip*)ResourceManager::GetResourceByName(_clipIds[2].c_str(), ResourceType::RES_ANIMCLIP);
	
	if(!_clips[0] || !_clips[1] || !_clips[2])
		return ENGINE_INVALID_RES;

	_initialAnim = _defaultAnim;

	return ENGINE_OK;
}

void DemoAnimatorComponent::Update(double deltaTime) noexcept
{
	if (Engine::GetKeyDown('j'))
	{
		_defaultAnim = _clips[0];
		_currentTime = 0.0;
	}
	else if (Engine::GetKeyDown('k'))
	{
		_defaultAnim = _clips[1];
		_currentTime = 0.0;
	}
	else if (Engine::GetKeyDown('l'))
	{
		_defaultAnim = _clips[2];
		_currentTime = 0.0;
	}
	else if (Engine::GetKeyDown('h'))
	{
		_defaultAnim = _initialAnim;
		_currentTime = 0.0;
	}

	_skeleton->SetAnimationClip(_defaultAnim);
	
	AnimatorComponent::Update(deltaTime);
}

void DemoAnimatorComponent::Unload()
{
	_defaultAnim = _initialAnim;
	_initialAnim = nullptr;

	AnimatorComponent::Unload();

	if (_clips[0])
		ResourceManager::UnloadResource(_clips[0]->GetResourceInfo()->id, ResourceType::RES_ANIMCLIP);

	if (_clips[1])
		ResourceManager::UnloadResource(_clips[1]->GetResourceInfo()->id, ResourceType::RES_ANIMCLIP);

	if (_clips[2])
		ResourceManager::UnloadResource(_clips[2]->GetResourceInfo()->id, ResourceType::RES_ANIMCLIP);
}