/* Neko Engine
 *
 * AudioSourceComponent.cpp
 * Author: Alexandru Naiman
 *
 * AudioSource component class implementation
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

#include <Scene/Components/AudioSourceComponent.h>
#include <Engine/ResourceManager.h>
#include <Scene/Object.h>

ENGINE_REGISTER_COMPONENT_CLASS(AudioSourceComponent);

AudioSourceComponent::AudioSourceComponent(ComponentInitializer *initializer)
	: ObjectComponent(initializer)
{
	_src = nullptr;
	_defaultClip = nullptr;
	_defaultClipId = initializer->arguments.find("defaultclip")->second;

	if((_src = new AudioSource()) == nullptr)
	{ DIE("Out of resources"); }

	_src->SetLooping(!initializer->arguments.find("loop")->second.compare("true") ? true : false);
	_src->SetReferenceDistance((float)atof(initializer->arguments.find("refdistance")->second.c_str()));
	_src->SetMaxDistance((float)atof(initializer->arguments.find("maxdistance")->second.c_str()));

	_playOnLoad = !initializer->arguments.find("playonload")->second.compare("true") ? true : false;
}

int AudioSourceComponent::Load()
{
	int ret = ObjectComponent::Load();

	if (ret != ENGINE_OK)
		return ret;

	_defaultClip = (AudioClip*)ResourceManager::GetResourceByName(_defaultClipId.c_str(), ResourceType::RES_AUDIOCLIP);
	
	if(!_defaultClip)
		return ENGINE_INVALID_RES;

	_src->SetClip(_defaultClip);

	if(_playOnLoad)
		_src->Play();
	
	return ENGINE_OK;
}

void AudioSourceComponent::PlayDefaultClip() noexcept
{
	//
}

void AudioSourceComponent::PlaySound(AudioClip *clip) noexcept
{
	//
}

void AudioSourceComponent::Update(double deltaTime) noexcept
{
	if (!_src)
		return;

	ObjectComponent::Update(deltaTime);

	glm::vec3 pos = _parent->GetPosition() + _localPosition;
	
	_src->SetPosition(pos.x, pos.y, pos.z);
}

void AudioSourceComponent::Unload()
{
	ObjectComponent::Unload();

	if(_defaultClip)
		ResourceManager::UnloadResourceByName(_defaultClipId.c_str(), ResourceType::RES_AUDIOCLIP);
	_defaultClip = nullptr;

	if (_src)
		delete _src;
	_src = nullptr;
}