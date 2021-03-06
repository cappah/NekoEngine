/* NekoEngine
 *
 * AudioSourceComponent.cpp
 * Author: Alexandru Naiman
 *
 * AudioSource component class implementation
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

#include <Scene/Components/AudioSourceComponent.h>
#include <Engine/ResourceManager.h>
#include <Audio/AudioSystem.h>
#include <Scene/Object.h>

using namespace glm;

ENGINE_REGISTER_COMPONENT_CLASS(AudioSourceComponent);

AudioSourceComponent::AudioSourceComponent(ComponentInitializer *initializer)
	: ObjectComponent(initializer)
{
	_src = nullptr;
	_defaultClip = nullptr;
	_defaultClipId = initializer->arguments.find("defaultclip")->second;

	if((_src = AudioSystem::GetInstance()->CreateSource()) == nullptr)
	{ DIE("Out of resources"); }

	_src->SetLooping(!initializer->arguments.find("loop")->second.compare("true") ? true : false);
	_src->SetReferenceDistance((float)atof(initializer->arguments.find("refdistance")->second.c_str()));
	_src->SetMaxDistance((float)atof(initializer->arguments.find("maxdistance")->second.c_str()));

	_src->SetGain(Engine::GetConfiguration().Audio.MasterVolume * Engine::GetConfiguration().Audio.EffectsVolume);

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
	_src->SetClip(_defaultClip);
	_src->Play();
}

void AudioSourceComponent::PlayClip(AudioClip *clip) noexcept
{
	_src->SetClip(clip);
	_src->Play();
}

void AudioSourceComponent::UpdatePosition() noexcept
{
	ObjectComponent::UpdatePosition();

	vec3 pos = _parent->GetPosition() + _position;
	_src->SetPosition(pos);
}

bool AudioSourceComponent::Unload()
{
	if (!ObjectComponent::Unload())
		return false;

	if(_defaultClipId.length())
		ResourceManager::UnloadResourceByName(_defaultClipId.c_str(), ResourceType::RES_AUDIOCLIP);
	_defaultClip = nullptr;

	delete _src;
	_src = nullptr;

	return true;
}
