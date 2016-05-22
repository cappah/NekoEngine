/* Neko Engine
 *
 * AnimatorComponent.cpp
 * Author: Alexandru Naiman
 *
 * Animator component class implementation
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

#include <Scene/Components/AnimatorComponent.h>
#include <Scene/Components/SkeletalMeshComponent.h>
#include <Scene/Object.h>
#include <Engine/Skeleton.h>
#include <Engine/ResourceManager.h>

ENGINE_REGISTER_COMPONENT_CLASS(AnimatorComponent);

AnimatorComponent::AnimatorComponent(ComponentInitializer *initializer)
	: ObjectComponent(initializer)
{
	_mesh = nullptr;
	_defaultAnim = nullptr;
	_defaultAnimId = initializer->arguments.find("defaultanim")->second;
	_targetMesh = initializer->arguments.find("targetmesh")->second;
}

int AnimatorComponent::Load()
{
	int ret = ObjectComponent::Load();

	if (ret != ENGINE_OK)
		return ret;

	_defaultAnim = (AnimationClip*)ResourceManager::GetResourceByName(_defaultAnimId.c_str(), ResourceType::RES_ANIMCLIP);
	
	if(!_defaultAnim)
		return ENGINE_INVALID_RES;
	
	SkeletalMeshComponent *comp = dynamic_cast<SkeletalMeshComponent*>(_parent->GetComponent(_targetMesh.c_str()));
	
	if(!comp)
		return ENGINE_INVALID_ARGS;
	
	_mesh = comp->GetMesh();
	
	PlayDefaultAnimation();
	
	return ENGINE_OK;
}

void AnimatorComponent::PlayDefaultAnimation() noexcept
{
	if(_mesh)
		_mesh->GetSkeleton()->SetAnimationClip(_defaultAnim);
}

void AnimatorComponent::PlayAnimation(AnimationClip *clip) noexcept
{
	if(_mesh)
		_mesh->GetSkeleton()->SetAnimationClip(clip);
}

void AnimatorComponent::Update(float deltaTime) noexcept
{
	ObjectComponent::Update(deltaTime);

	if(_mesh)
		_mesh->GetSkeleton()->Update(deltaTime);
}

void AnimatorComponent::Unload()
{
	ObjectComponent::Unload();

	if (_defaultAnim)
		ResourceManager::UnloadResource(_defaultAnim->GetResourceInfo()->id, ResourceType::RES_ANIMCLIP);
}