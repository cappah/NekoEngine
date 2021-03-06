/* NekoEngine
 *
 * AnimatorComponent.cpp
 * Author: Alexandru Naiman
 *
 * Animator component class implementation
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

#include <Scene/Components/AnimatorComponent.h>
#include <Scene/Components/SkeletalMeshComponent.h>
#include <Scene/SceneManager.h>
#include <Scene/Object.h>
#include <Animation/Skeleton.h>
#include <Renderer/SkeletalMesh.h>
#include <Engine/ResourceManager.h>

ENGINE_REGISTER_COMPONENT_CLASS(AnimatorComponent);

AnimatorComponent::AnimatorComponent(ComponentInitializer *initializer)
	: ObjectComponent(initializer)
{
	_skeleton = nullptr;
	_defaultAnim = nullptr;
	_defaultAnimId = initializer->arguments.find("defaultanim")->second;
	_targetMesh = initializer->arguments.find("targetmesh")->second;
	_currentTime = 0.0;
	_loop = false;
	_oneShot = false;
	_prevLoop = false;
	_prevClip = nullptr;
}

int AnimatorComponent::Load()
{
	int ret = ObjectComponent::Load();
	
	if (ret != ENGINE_OK)
		return ret;
	
	_defaultAnim = (AnimationClip*)ResourceManager::GetResourceByName(_defaultAnimId.c_str(), ResourceType::RES_ANIMCLIP);
	
	if(!_defaultAnim)
		return ENGINE_INVALID_RES;
	
	SkeletalMeshComponent *comp = dynamic_cast<SkeletalMeshComponent *>(_parent->GetComponent(_targetMesh.c_str()));
	if(!comp)
		return ENGINE_INVALID_ARGS;
	
	SkeletalMesh *mesh = comp->GetMesh();
	if(!mesh)
		return ENGINE_INVALID_ARGS;
	
	_skeleton = mesh->CreateSkeleton();
	if(!_skeleton)
		return ENGINE_FAIL;
	
	PlayDefaultAnimation();
	
	return ENGINE_OK;
}

void AnimatorComponent::PlayDefaultAnimation(bool loop) noexcept
{
	if (_skeleton->GetAnimationClip() == _defaultAnim)
		return;

	_loop = loop;
	_playing = true;
	_oneShot = false;
	_currentTime = 0.0;

	if(_skeleton)
		_skeleton->SetAnimationClip(_defaultAnim);
}

void AnimatorComponent::PlayAnimation(AnimationClip *clip, bool loop) noexcept
{
	if (_skeleton->GetAnimationClip() == clip)
		return;

	_loop = loop;
	_playing = true;
	_oneShot = false;
	_currentTime = 0.0;

	if (_skeleton)
		_skeleton->SetAnimationClip(clip);
}

void AnimatorComponent::Update(double deltaTime) noexcept
{
	ObjectComponent::Update(deltaTime);

	if (!SceneManager::IsSceneLoaded())
		return;

	if (_skeleton && _playing)
	{
		_currentTime += deltaTime;
		
		if (_currentTime > _skeleton->GetAnimationClip()->GetDuration()) {
			if (_loop) {
				_currentTime = 0.0;
			} else if (_oneShot) {
				_oneShot = false;
				PlayAnimation(_prevClip, _prevLoop);
			} else {
				_playing = false;
			}
		}

		if (_playing)
			_skeleton->TransformBones(_currentTime);
	}
}

void AnimatorComponent::UpdateData(VkCommandBuffer commandBuffer) noexcept
{
	_skeleton->UpdateData(commandBuffer);
}

bool AnimatorComponent::Unload()
{
	if (!ObjectComponent::Unload())
		return false;

	if (_defaultAnim)
		ResourceManager::UnloadResource(_defaultAnim->GetResourceInfo()->id, ResourceType::RES_ANIMCLIP);

	delete _skeleton;

	return true;
}
