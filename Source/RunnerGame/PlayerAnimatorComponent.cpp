/* NekoEngine
*
* DemoAnimatorComponent.cpp
* Author: Alexandru Naiman
*
* DemoAnimatorComponent class implementation
*
* -----------------------------------------------------------------------------
*
* Copyright (c) 2015-2017, Alexandru Naiman
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

#include "PlayerAnimatorComponent.h"
#include <Engine/ResourceManager.h>
#include <Engine/Input.h>

REGISTER_COMPONENT_CLASS(PlayerAnimatorComponent);

PlayerAnimatorComponent::PlayerAnimatorComponent(ComponentInitializer *initializer)
	: AnimatorComponent(initializer)
{
	_clipIds[0] = initializer->arguments.find("running")->second;
	_clipIds[1] = initializer->arguments.find("jumping")->second;
	_clipIds[2] = initializer->arguments.find("crouching")->second;
	_clipIds[3] = initializer->arguments.find("death")->second;
	_clips[0] = _clips[1] = _clips[2] = _clips[3] = nullptr;
}

int PlayerAnimatorComponent::Load()
{
	int ret{ AnimatorComponent::Load() };

	if (ret != ENGINE_OK)
		return ret;

	_clips[0] = (AnimationClip *)ResourceManager::GetResourceByName(_clipIds[0].c_str(), ResourceType::RES_ANIMCLIP);
	_clips[1] = (AnimationClip *)ResourceManager::GetResourceByName(_clipIds[1].c_str(), ResourceType::RES_ANIMCLIP);
	_clips[2] = (AnimationClip *)ResourceManager::GetResourceByName(_clipIds[2].c_str(), ResourceType::RES_ANIMCLIP);
	_clips[3] = (AnimationClip *)ResourceManager::GetResourceByName(_clipIds[3].c_str(), ResourceType::RES_ANIMCLIP);

	if (!_clips[0] || !_clips[1] || !_clips[2] || !_clips[3])
		return ENGINE_INVALID_RES;

	_defaultAnim = _clips[0];

	return ENGINE_OK;
}

void PlayerAnimatorComponent::Update(double deltaTime) noexcept
{
	PlayerState *playerState = ((Player *)_parent)->GetState();
	PlayerStateType playerStateType = playerState->GetStateType ();

	if (_currentPlayerState != playerStateType)
	{
		if (playerStateType == PlayerStateType::STATE_JUMPING)
		{
			_defaultAnim = _clips[1];
			_currentTime = 0.0;
		}
		else if (playerStateType == PlayerStateType::STATE_CROUCHING)
		{
			_defaultAnim = _clips[2];
			_currentTime = 0.0;
		}
		else if (playerStateType == PlayerStateType::STATE_DYING)
		{
			_defaultAnim = _clips[3];
			_currentTime = 0.0;
		} 
		else // any other state use a running animation
		{
			_defaultAnim = _clips[0];
			_currentTime = 0.0;
		}
	}

	_currentPlayerState = playerStateType;
	_skeleton->SetAnimationClip(_defaultAnim);

	AnimatorComponent::Update(deltaTime);
}

bool PlayerAnimatorComponent::Unload()
{
	if (!AnimatorComponent::Unload())
		return false;

	ResourceManager::UnloadResourceByName(_clipIds[0].c_str(), ResourceType::RES_ANIMCLIP);
	ResourceManager::UnloadResourceByName(_clipIds[1].c_str(), ResourceType::RES_ANIMCLIP);
	ResourceManager::UnloadResourceByName(_clipIds[2].c_str(), ResourceType::RES_ANIMCLIP);
	ResourceManager::UnloadResourceByName(_clipIds[3].c_str(), ResourceType::RES_ANIMCLIP);

	return true;
}
